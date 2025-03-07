#ifndef FRX_MODULE_H
#define FRX_MODULE_H

#include <dirent.h>

#include "list.h"
#include "symbol_table.h"

typedef struct Parser Parser;

typedef struct Module
{
    struct Module* parent;
    char filepath[PATH_MAX];
    const char* name;
    b8 failed;
    List submodules;
    List parsers;
    SymbolTable symbol_table;
} Module;

Module* module_create(const char* project_path);

void module_compile(Module* mod);

void module_codegen(Module* mod);

void module_insert_symbol(Module* mod, Parser* parser, SymbolVisibility visibility,
                          SymbolID id, SymbolType type, void* data);

Symbol* module_lookup_symbol(Module* mod, Parser* parser, SymbolID id);

Module* module_find_submodule_by_name(Module* mod, const char* name);

b8 module_failed(const Module* mod);

void module_destroy(Module* mod);

#endif
