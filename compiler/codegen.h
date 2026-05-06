#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>
#include "ast.h"
#include "symtab.h"

typedef struct {
    FILE *out;
    SymbolTable *tab;
    int stack_pos;
    char *module_name;
} Codegen;

Codegen *codegen_new(FILE *out, const char *module_name);
void codegen_generate(Codegen *cg, ASTNode *node);

#endif
