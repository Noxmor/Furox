#include "module.h"

#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "assert.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "symbol_table.h"

static b8 str_has_suffix(const char* str, const char* suffix)
{
    FRX_ASSERT(str != NULL);
    FRX_ASSERT(suffix != NULL);

    return strcmp(str + strlen(str) - strlen(suffix), suffix) == 0;
}

static Module* submodule_create(Module* parent, const char* filepath)
{
    FRX_ASSERT(filepath != NULL);

    Module* mod = malloc(sizeof(Module));

    symbol_table_init(&mod->symbol_table);

    symbol_registry_init(&mod->symbol_registry);

    mod->parent = parent;
    strcpy(mod->filepath, filepath);
    mod->name = strrchr(mod->filepath, '/') + 1;
    mod->failed = FRX_FALSE;

    list_init(&mod->submodules);
    list_init(&mod->parsers);

    DIR* dir = opendir(mod->filepath);
    if (dir == NULL)
    {
        module_destroy(mod);
        return NULL;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        char buffer[PATH_MAX];

        FRX_ASSERT(strlen(mod->filepath) + strlen("/") + strlen(entry->d_name) < sizeof(buffer));
        sprintf(buffer, "%s/%s", mod->filepath, entry->d_name);


        if (entry->d_type == DT_DIR)
        {
            Module* submodule = submodule_create(mod, buffer);
            list_add(&mod->submodules, submodule);
        }
        else if (entry->d_type == DT_REG && str_has_suffix(entry->d_name, ".frx"))
        {

            Parser* parser = parser_create(mod, buffer);
            list_add(&mod->parsers, parser);
        }
    }

    closedir(dir);

    return mod;
}

Module* module_create(const char* project_path)
{
    const char* name = strrchr(project_path, '/');
    if (name == NULL)
    {
        name = project_path;
    }
    else
    {
        ++name;
    }

    char src_path[PATH_MAX];
    sprintf(src_path, "%s/src", project_path);

    Module* mod = submodule_create(NULL, src_path);
    mod->name = name;

    return mod;
}

void module_compile(Module* mod)
{
    FRX_ASSERT(mod != NULL);

    for (usize i = 0; i < list_size(&mod->submodules); ++i)
    {
        Module* submodule = list_get(&mod->submodules, i);

        module_compile(submodule);
        if (module_failed(submodule))
        {
            mod->failed = FRX_TRUE;
        }
    }

    for (usize i = 0; i < list_size(&mod->parsers); ++i)
    {
        Parser* parser = list_get(&mod->parsers, i);
        parser_parse(parser);

        if (parser->translation_unit != NULL)
        {
            translation_unit_resolve(parser, parser->translation_unit);
            translation_unit_sema(parser->translation_unit);
        }

        parser_emit_diagnostics(parser);

        if (parser_failed(parser))
        {
            mod->failed = FRX_TRUE;
        }
    }
}

SymbolID module_insert_symbol(Module* mod, SymbolVisibility visibility,
                           SymbolType type, const char* name, void* data)
{
    FRX_ASSERT(mod != NULL);

    SymbolID id = symbol_registry_add(&mod->symbol_registry, data);

    if (visibility > FRX_SYMBOL_VISIBILITY_PRIVATE)
    {
        symbol_table_insert(&mod->symbol_table, type, name, id);
    }

    return id;
}

SymbolID module_lookup_symbol(Module* mod, SymbolType type, const char* name)
{
    FRX_ASSERT(mod != NULL);

    return symbol_table_lookup(&mod->symbol_table, type, name);
}

void* module_get_def(Module* mod, SymbolID id)
{
    FRX_ASSERT(mod != NULL);

    return symbol_registry_get(&mod->symbol_registry, id);
}

Module* module_find_submodule_by_name(Module* mod, const char* name)
{
    FRX_ASSERT(mod != NULL);

    FRX_ASSERT(name != NULL);

    for (usize i = 0; i < list_size(&mod->submodules); ++i)
    {
        Module* submodule = list_get(&mod->submodules, i);
        if (strcmp(submodule->name, name) == 0)
        {
            return submodule;
        }
    }

    return NULL;
}

b8 module_failed(const Module* mod)
{
    return mod->failed;
}

void module_destroy(Module* mod)
{
    FRX_ASSERT(mod != NULL);

    for(usize i = 0; i < list_size(&mod->parsers); ++i)
    {
        Parser* parser = list_get(&mod->parsers, i);
        parser_destroy(parser);
    }

    free(mod);
}
