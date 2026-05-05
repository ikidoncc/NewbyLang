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

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <filename.nb>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    char *src = read_file_content(filename);
    if (!src) {
        return 1;
    }

    // Determinar o nome base (ex: hello.nb -> hello)
    char base_name[256];
    strncpy(base_name, filename, sizeof(base_name) - 1);
    char *dot = strrchr(base_name, '.');
    if (dot) *dot = '\0';

    char asm_filename[260], obj_filename[260];
    sprintf(asm_filename, "%s.asm", base_name);
    sprintf(obj_filename, "%s.o", base_name);

    Lexer *lexer = lexer_new(src);
    Parser *parser = parser_new(lexer);
    ASTNode *root = parser_parse(parser);

    // Análise Semântica
    SymbolTable *sem_tab = symtab_new(NULL);
    semantic_analyze(root, sem_tab);

    FILE *out = fopen(asm_filename, "w");
    if (!out) {
        perror("Error opening assembly file");
        free(src);
        return 1;
    }

    Codegen *cg = codegen_new(out);
    codegen_generate(cg, root);

    fclose(out);
    free(src);
    
    // Automatizar a geração do executável
    printf("Compiling Newby source: %s\n", filename);
    printf("Assembling and linking...\n");

    char cmd[512];
    sprintf(cmd, "nasm -f elf64 %s -o %s", asm_filename, obj_filename);
    if (system(cmd) != 0) {
        fprintf(stderr, "Error during assembly (nasm)\n");
        return 1;
    }

    sprintf(cmd, "ld %s -o %s", obj_filename, base_name);
    if (system(cmd) != 0) {
        fprintf(stderr, "Error during linking (ld)\n");
        return 1;
    }

    // Remover arquivos intermediários
    remove(asm_filename);
    remove(obj_filename);

    printf("Compilation successful! Executable generated: ./%s\n", base_name);

    return 0;
}
