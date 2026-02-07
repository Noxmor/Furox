#include "module.h"

#include "assert.h"
#include "compiler.h"
#include "symbol_table.h"

Module* module_create_root(void)
{
    Module* mod = compiler_alloc(sizeof(Module));

    mod->parent = NULL;
    mod->name = "";
    list_init(&mod->submodules);
    symbol_table_init(&mod->symbol_table);

    return mod;
}

Module* module_create(Module* parent, const char* name)
{
    FRX_ASSERT(parent != NULL);

    FRX_ASSERT(name != NULL);

    Module* mod = compiler_alloc(sizeof(Module));

    mod->parent = parent;
    mod->name = name;
    list_init(&mod->submodules);
    symbol_table_init(&mod->symbol_table);

    list_add(&mod->parent->submodules, mod);

    return mod;
}

Symbol* module_insert_symbol(Module* mod, Symbol* symbol)
{
    FRX_ASSERT(mod != NULL);

    return symbol_table_insert_symbol(&mod->symbol_table, symbol);
}

Symbol* module_lookup_symbol(Module* mod, const char* name)
{
    FRX_ASSERT(mod != NULL);

    return symbol_table_lookup(&mod->symbol_table, name);
}

Module* module_find_submodule_by_name(Module* mod, const char* name)
{
    FRX_ASSERT(mod != NULL);

    FRX_ASSERT(name != NULL);

    for (usize i = 0; i < list_size(&mod->submodules); ++i)
    {
        Module* submodule = list_get(&mod->submodules, i);
        if (submodule->name == name)
        {
            return submodule;
        }
    }

    return NULL;
}
