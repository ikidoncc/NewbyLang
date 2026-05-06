#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>
#include "ast.h"
#include "symtab.h"

typedef struct {
    char *name;
    struct { char *name; int offset; Type type; } members[16];
    int member_count;
    int size;
} StructMeta;

typedef struct {
    FILE *out;
    SymbolTable *tab;
    int stack_pos;
    char *module_name;
    StructMeta structs[16];
    int struct_count;
    int emit_entry;
} Codegen;

Codegen *codegen_new(FILE *out, const char *module_name);
void codegen_generate(Codegen *cg, ASTNode *node);

#endif
