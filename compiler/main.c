#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "codegen.h"

char *link_objects[32];
int link_object_count = 0;

void add_link_object(const char *name) {
    for (int i = 0; i < link_object_count; i++) {
        if (strcmp(link_objects[i], name) == 0) return;
    }
    link_objects[link_object_count++] = strdup(name);
}

char *find_module_path(const char *filename) {
    if (access(filename, F_OK) == 0) return strdup(filename);
    char lib_path[512];
    sprintf(lib_path, "library/%s", filename);
    if (access(lib_path, F_OK) == 0) return strdup(lib_path);
    return NULL;
}

char *read_file_content(const char *filename) {
    char *path = find_module_path(filename);
    if (!path) return NULL;
    FILE *file = fopen(path, "r");
    free(path);
    if (!file) return NULL;
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *buffer = malloc(length + 1);
    if (!buffer) { fclose(file); return NULL; }
    size_t read_len = fread(buffer, 1, length, file);
    buffer[read_len] = '\0';
    fclose(file);
    return buffer;
}

ASTNode *compile_file(const char *filename) {
    char *src = read_file_content(filename);
    if (!src) return NULL;
    Lexer *lexer = lexer_new(src);
    Parser *parser = parser_new(lexer);
    ASTNode *root = parser_parse(parser);
    parser_free(parser);
    lexer_free(lexer);
    free(src);
    return root;
}

void process_imports(ASTNode *root, int is_root) {
    int initial_count = root->data.program.count;
    for (int i = 0; i < initial_count; i++) {
        ASTNode *n = root->data.program.nodes[i];
        if (n->type == AST_IMPORT) {
            char *mod_name = n->data.import.module_name;
            ASTNode *imp = compile_file(mod_name);
            if (!imp) {
                fprintf(stderr, "Error: Could not find module '%s'\n", mod_name);
                exit(1);
            }

            char base_mod[256];
            strncpy(base_mod, mod_name, 255);
            char *dot = strrchr(base_mod, '.'); if (dot) *dot = '\0';
            char *slash = strrchr(base_mod, '/');
            char *final_mod = slash ? slash + 1 : base_mod;
            semantic_add_module(final_mod);

            char obj_path[512];
            char *found_path = find_module_path(mod_name);
            strncpy(obj_path, found_path, 511);
            char *dot_obj = strrchr(obj_path, '.');
            if (dot_obj) strcpy(dot_obj, ".o");
            else strcat(obj_path, ".o");
            
            if (access(obj_path, F_OK) != 0) {
                char compile_cmd[1024];
                sprintf(compile_cmd, "./nbc -c %s", found_path);
                if (system(compile_cmd) != 0) {
                    fprintf(stderr, "Error: Failed to auto-compile module '%s'\n", found_path);
                    exit(1);
                }
            }
            free(found_path);
            add_link_object(obj_path);

            for (int j = 0; j < imp->data.program.count; j++) {
                ASTNode *in = imp->data.program.nodes[j];
                if (in->type == AST_FUNC_DECL && in->data.func_decl.is_pub) {
                    char mangled[512];
                    sprintf(mangled, "%s_%s", final_mod, in->data.func_decl.name);
                    ASTNode *ext = ast_new_extern_decl(mangled, in->data.func_decl.return_type);
                    for (int k = 0; k < in->data.func_decl.param_count; k++) {
                        ast_func_add_param(ext, in->data.func_decl.params[k].type, in->data.func_decl.params[k].name, in->data.func_decl.params[k].struct_name);
                    }
                    ast_program_add(root, ext);
                }
            }
            ast_free(imp);
        }
    }
}

int main(int argc, char **argv) {
    int compile_only = 0;
    const char *filename = NULL;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0) compile_only = 1;
        else filename = argv[i];
    }
    if (!filename) return 1;

    ASTNode *root = compile_file(filename);
    if (!root) { fprintf(stderr, "Error: Could not read file '%s'\n", filename); return 1; }

    char base_name[256];
    const char *last_slash = strrchr(filename, '/');
    const char *actual_name = last_slash ? last_slash + 1 : filename;
    strncpy(base_name, actual_name, 255);
    char *dot_ptr = strrchr(base_name, '.');
    if (dot_ptr) *dot_ptr = '\0';

    process_imports(root, 1);

    SymbolTable *sem_tab = symtab_new(NULL);
    semantic_analyze(root, sem_tab);

    char base_path[512];
    strncpy(base_path, filename, 511);
    dot_ptr = strrchr(base_path, '.');
    if (dot_ptr) *dot_ptr = '\0';

    char asm_filename[512], obj_filename[512];
    sprintf(asm_filename, "%s.asm", base_path);
    sprintf(obj_filename, "%s.o", base_path);

    FILE *out = fopen(asm_filename, "w");
    Codegen *cg = codegen_new(out, base_name);
    if (compile_only) cg->emit_entry = 0;
    codegen_generate(cg, root);
    fclose(out);
    
    char cmd[2048];
    sprintf(cmd, "nasm -f elf64 %s -o %s", asm_filename, obj_filename);
    int res = system(cmd);
    remove(asm_filename);
    if (res != 0) {
        codegen_free(cg);
        symtab_free(sem_tab);
        semantic_cleanup();
        ast_free(root);
        for (int i = 0; i < link_object_count; i++) free(link_objects[i]);
        return 1;
    }

    if (!compile_only) {
        char link_cmd[4096];
        sprintf(link_cmd, "gcc %s ", obj_filename);
        for (int i = 0; i < link_object_count; i++) {
            strcat(link_cmd, link_objects[i]);
            strcat(link_cmd, " ");
        }
        strcat(link_cmd, "-o ");
        strcat(link_cmd, base_name);
        strcat(link_cmd, " -no-pie -nostartfiles -lc");
        
        if (system(link_cmd) != 0) {
            fprintf(stderr, "Linking failed.\n");
            codegen_free(cg);
            symtab_free(sem_tab);
            semantic_cleanup();
            ast_free(root);
            for (int i = 0; i < link_object_count; i++) free(link_objects[i]);
            return 1;
        }
        printf("Compilation successful! Executable generated: ./%s\n", base_name);
        remove(obj_filename);
    } else {
        printf("Object file generated: %s\n", obj_filename);
    }

    codegen_free(cg);
    symtab_free(sem_tab);
    semantic_cleanup();
    ast_free(root);
    for (int i = 0; i < link_object_count; i++) free(link_objects[i]);

    return 0;
}
