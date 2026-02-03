#ifndef FRX_MODULE_H
#define FRX_MODULE_H

#include "list.h"
#include "symbol_table.h"

typedef struct Parser Parser;

typedef struct Module
{
    struct Module* parent;
    const char* name;
    List submodules;
    SymbolTable symbol_table;
} Module;

Module* module_create_root(void);

Module* module_create(Module* parent, const char* name);

Symbol* module_insert_symbol(Module* mod, Symbol* symbol);

Symbol* module_lookup_symbol(Module* mod, const char* name);

Module* module_find_submodule_by_name(Module* mod, const char* name);

#endif
