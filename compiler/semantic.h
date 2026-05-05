#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "symtab.h"

void semantic_analyze(ASTNode *node, SymbolTable *tab);

#endif
