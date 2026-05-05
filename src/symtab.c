#include <stdlib.h>
#include <string.h>
#include "symtab.h"

SymbolTable *symtab_new(SymbolTable *parent) {
    SymbolTable *tab = malloc(sizeof(SymbolTable));
    tab->symbols = NULL;
    tab->count = 0;
    tab->parent = parent;
    return tab;
}

void symtab_add(SymbolTable *tab, char *name, int offset, Type type, int is_array, int array_size) {
    tab->count++;
    tab->symbols = realloc(tab->symbols, sizeof(Symbol) * tab->count);
    tab->symbols[tab->count - 1].name = strdup(name);
    tab->symbols[tab->count - 1].stack_offset = offset;
    tab->symbols[tab->count - 1].type = type;
    tab->symbols[tab->count - 1].is_array = is_array;
    tab->symbols[tab->count - 1].array_size = array_size;
}

Symbol *symtab_lookup(SymbolTable *tab, char *name) {
    for (int i = 0; i < tab->count; i++) {
        if (strcmp(tab->symbols[i].name, name) == 0) {
            return &tab->symbols[i];
        }
    }
    if (tab->parent) return symtab_lookup(tab->parent, name);
    return NULL;
}
