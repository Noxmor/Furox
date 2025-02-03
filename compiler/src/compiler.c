#include "compiler.h"

#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "arena.h"
#include "lexer.h"
#include "module.h"
#include "string_table.h"
#include "symbol_table.h"
#include "codegen.h"

static Arena* arena;

static void compiler_init(void)
{
    FRX_LOG_INFO("Initializing compiler...");

    lexer_init_keyword_table();

    arena = arena_create();
}

static void compiler_shutdown(void)
{
    FRX_LOG_INFO("Shutting down compiler...");

    symbol_table_shutdown();
    string_table_shutdown();

    arena_destroy(arena);
}

void* compiler_alloc(usize size)
{
    return arena_alloc(arena, size);
}

int compiler_run(int argc, char** argv)
{
    compiler_init();

    List projects;
    list_init(&projects);

    for (int i = 1; i < argc; ++i)
    {
        char* project_path = argv[i];
        if (project_path[strlen(project_path) - 1] == '/')
        {
            project_path[strlen(project_path) - 1] = '\0';
        }

        Module* mod = module_create(project_path);
        module_compile(mod);
        list_add(&projects, mod);
    }

    for (usize i = 0; i < list_size(&projects); ++i)
    {
        Module* mod = list_get(&projects, i);
        if (module_failed(mod))
        {
            return EXIT_FAILURE;
        }
    }

    codegen_begin("frx.c");

    for (usize i = 0; i < list_size(&projects); ++i)
    {
        Module* mod = list_get(&projects, i);
        module_codegen(mod);
    }

    codegen_end();

    system("gcc frx.c");

    for (usize i = 0; i < list_size(&projects); ++i)
    {
        Module* mod = list_get(&projects, i);
        module_destroy(mod);
    }

    compiler_shutdown();

    return EXIT_SUCCESS;
}
