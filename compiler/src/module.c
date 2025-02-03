#include "module.h"

#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "assert.h"
#include "codegen.h"
#include "parser.h"
#include "sema.h"

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

            Parser* parser = parser_create(buffer);
            list_add(&mod->parsers, parser);
        }
    }

    closedir(dir);

    return mod;
}

Module* module_create(const char* project_path)
{
    const char* name = strrchr(project_path, '/') + 1;
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
            translation_unit_sema(parser->translation_unit);
        }

        parser_emit_diagnostics(parser);

        if (parser_failed(parser))
        {
            mod->failed = FRX_TRUE;
        }
    }
}

void module_codegen(Module* mod)
{
    for (usize i = 0; i < list_size(&mod->submodules); ++i)
    {
        Module* submodule = list_get(&mod->submodules, i);

        module_codegen(submodule);
    }

    for (usize i = 0; i < list_size(&mod->parsers); ++i)
    {
        Parser* parser = list_get(&mod->parsers, i);
        translation_unit_codegen(parser->translation_unit);
    }
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
