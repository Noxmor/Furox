#ifndef FRX_MODULE_H
#define FRX_MODULE_H

#include <dirent.h>

#include "list.h"
#include "symbol_table.h"
#include "mir.h"

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
    SymbolRegistry symbol_registry;
} Module;

Module* module_create(const char* project_path);

void module_compile(Module* mod);

void module_codegen(Module* mod, MIRContext* ctx);

SymbolID module_insert_symbol(Module* mod, SymbolVisibility visibility,
                          SymbolType type, const char* name, void* data);

SymbolID module_lookup_symbol(Module* mod, SymbolType type, const char* name);

void* module_get_def(Module* mod, SymbolID id);

Module* module_find_submodule_by_name(Module* mod, const char* name);

b8 module_failed(const Module* mod);

void module_destroy(Module* mod);

#endif
