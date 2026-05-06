#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "codegen.h"

char *read_file_content(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        char lib_path[512];
        sprintf(lib_path, "library/%s", filename);
        file = fopen(lib_path, "r");
        if (!file) return NULL;
    }
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
    return parser_parse(parser);
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
    if (!root) return 1;

    char base_name[256];
    const char *last_slash = strrchr(filename, '/');
    const char *actual_name = last_slash ? last_slash + 1 : filename;
    strncpy(base_name, actual_name, 255);
    char *dot = strrchr(base_name, '.');
    if (dot) *dot = '\0';

    char asm_filename[260], obj_filename[260];
    sprintf(asm_filename, "%s.asm", base_name);
    sprintf(obj_filename, "%s.o", base_name);

    int initial_count = root->data.program.count;
    for (int i = 0; i < initial_count; i++) {
        ASTNode *n = root->data.program.nodes[i];
        if (n->type == AST_IMPORT) {
            ASTNode *imp = compile_file(n->data.import.module_name);
            if (imp) {
                char mod_name[256];
                strncpy(mod_name, n->data.import.module_name, 255);
                char *dot2 = strrchr(mod_name, '.'); if (dot2) *dot2 = '\0';
                char *slash = strrchr(mod_name, '/');
                char *final_mod = slash ? slash + 1 : mod_name;

                for (int j = 0; j < imp->data.program.count; j++) {
                    ASTNode *in = imp->data.program.nodes[j];
                    if (in->type == AST_FUNC_DECL && in->data.func_decl.is_pub) {
                        char mangled[512];
                        sprintf(mangled, "%s_%s", final_mod, in->data.func_decl.name);
                        free(in->data.func_decl.name);
                        in->data.func_decl.name = strdup(mangled);
                        in->data.func_decl.is_pub = 0;
                        ast_program_add(root, in);
                    } else if (in->type == AST_VAR_DECL && in->data.var_decl.is_pub) {
                        char mangled[512];
                        sprintf(mangled, "%s_%s", final_mod, in->data.var_decl.name);
                        free(in->data.var_decl.name);
                        in->data.var_decl.name = strdup(mangled);
                        in->data.var_decl.is_pub = 0;
                        ast_program_add(root, in);
                    }
                }
            }
        }
    }

    SymbolTable *sem_tab = symtab_new(NULL);
    semantic_analyze(root, sem_tab);

    FILE *out = fopen(asm_filename, "w");
    Codegen *cg = codegen_new(out, base_name);
    if (compile_only) cg->emit_entry = 0;
    codegen_generate(cg, root);
    fclose(out);
    
    char cmd[512];
    sprintf(cmd, "nasm -f elf64 %s -o %s", asm_filename, obj_filename);
    if (system(cmd) != 0) return 1;

    if (!compile_only) {
        sprintf(cmd, "gcc %s -o %s -no-pie -nostartfiles -lc", obj_filename, base_name);
        if (system(cmd) != 0) return 1;
        printf("Compilation successful! Executable generated: ./%s\n", base_name);
        remove(asm_filename);
        remove(obj_filename);
    } else {
        printf("Object file generated: %s\n", obj_filename);
        remove(asm_filename);
    }
    return 0;
}
