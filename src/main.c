#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"
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
        printf("Usage: %s <filename.cj>\n", argv[0]);
        return 1;
    }

    char *src = read_file_content(argv[1]);
    if (!src) {
        return 1;
    }

    Lexer *lexer = lexer_new(src);
    Parser *parser = parser_new(lexer);
    ASTNode *root = parser_parse(parser);

    FILE *out = fopen("output.asm", "w");
    if (!out) {
        perror("Error opening output.asm");
        free(src);
        return 1;
    }

    Codegen *cg = codegen_new(out);
    codegen_generate(cg, root);

    fclose(out);
    free(src);
    printf("Transpilation successful! Assembly generated in output.asm\n");

    return 0;
}
