#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "symtab.h"

void semantic_analyze(ASTNode *node, SymbolTable *tab);
void semantic_add_module(const char *name);
int semantic_is_enum(const char *name);
void semantic_cleanup();

#endif
