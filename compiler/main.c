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
        perror("Error opening file");
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *buffer = malloc(length + 1);
    if (!buffer) {
        perror("Error allocating memory");
        fclose(file);
        return NULL;
    }
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
    // Note: in a real compiler we'd free lexer/parser/src here
    return root;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <filename.nb>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    ASTNode *root = compile_file(filename);
    if (!root) return 1;

    // Determinar o nome base
    char base_name[256];
    const char *last_slash = strrchr(filename, '/');
    const char *actual_name = last_slash ? last_slash + 1 : filename;
    strncpy(base_name, actual_name, sizeof(base_name) - 1);
    char *dot = strrchr(base_name, '.');
    if (dot) *dot = '\0';

    char asm_filename[260], obj_filename[260];
    sprintf(asm_filename, "%s.asm", base_name);
    sprintf(obj_filename, "%s.o", base_name);

    // Processar imports (Simplificado: apenas um nível por enquanto)
    for (int i = 0; i < root->data.program.count; i++) {
        ASTNode *n = root->data.program.nodes[i];
        if (n->type == AST_IMPORT) {
            ASTNode *imported = compile_file(n->data.import.module_name);
            if (imported) {
                // Mangle public symbols of imported module
                char mod_name[256];
                strncpy(mod_name, n->data.import.module_name, 255);
                char *dot2 = strrchr(mod_name, '.'); if (dot2) *dot2 = '\0';
                char *slash = strrchr(mod_name, '/');
                char *final_mod = slash ? slash + 1 : mod_name;

                for (int j = 0; j < imported->data.program.count; j++) {
                    ASTNode *in = imported->data.program.nodes[j];
                    if (in->type == AST_FUNC_DECL && in->data.func_decl.is_pub) {
                        char mangled[512];
                        sprintf(mangled, "%s_%s", final_mod, in->data.func_decl.name);
                        free(in->data.func_decl.name);
                        in->data.func_decl.name = strdup(mangled);
                        in->data.func_decl.is_pub = 0; // Already mangled
                        ast_program_add(root, in);
                    }
                }
            }
        }
    }

    SymbolTable *sem_tab = symtab_new(NULL);
    semantic_analyze(root, sem_tab);

    FILE *out = fopen(asm_filename, "w");
    if (!out) { perror("Error opening assembly file"); return 1; }

    Codegen *cg = codegen_new(out, base_name);
    codegen_generate(cg, root);

    fclose(out);
    
    printf("Compiling Newby source: %s\n", filename);
    char cmd[512];
    sprintf(cmd, "nasm -f elf64 %s -o %s", asm_filename, obj_filename);
    if (system(cmd) != 0) return 1;

    sprintf(cmd, "gcc %s -o %s -no-pie -nostartfiles -lc", obj_filename, base_name);
    if (system(cmd) != 0) return 1;

    remove(asm_filename);
    remove(obj_filename);
    printf("Compilation successful! Executable generated: ./%s\n", base_name);

    return 0;
}
