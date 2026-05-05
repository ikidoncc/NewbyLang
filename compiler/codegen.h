#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>
#include "ast.h"
#include "symtab.h"

typedef struct {
    FILE *out;
    SymbolTable *tab;
    int stack_pos;
} Codegen;

Codegen *codegen_new(FILE *out);
void codegen_generate(Codegen *cg, ASTNode *node);

#endif
