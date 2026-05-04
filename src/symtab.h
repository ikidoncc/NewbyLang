#ifndef SYMTAB_H
#define SYMTAB_H

typedef struct {
    char *name;
    int stack_offset;
} Symbol;

typedef struct {
    Symbol *symbols;
    int count;
} SymbolTable;

SymbolTable *symtab_new();
void symtab_add(SymbolTable *tab, char *name, int offset);
int symtab_lookup(SymbolTable *tab, char *name);

#endif
