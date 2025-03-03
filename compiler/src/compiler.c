#include "compiler.h"

#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "arena.h"
#include "lexer.h"
#include "module.h"
#include "project.h"
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

        ProjectSpecificiation spec;
        spec.type = i == argc - 1 ? FRX_PROJECT_TYPE_APP : FRX_PROJECT_TYPE_LIB;

        Project* project = project_create(spec, project_path);
        list_add(&projects, project);
    }

    for (usize i = 0; i < list_size(&projects); ++i)
    {
        Project* project = list_get(&projects, i);
        project_compile(project);
    }

    for (usize i = 0; i < list_size(&projects); ++i)
    {
        Project* project = list_get(&projects, i);
        if (project_failed(project))
        {
            return EXIT_FAILURE;
        }
    }

    codegen_begin("frx.c");

    for (usize i = 0; i < list_size(&projects); ++i)
    {
        Project* project = list_get(&projects, i);
        Module* mod = project->root_module;
        module_codegen(mod);
    }

    codegen_end();

    system("gcc frx.c");

    for (usize i = 0; i < list_size(&projects); ++i)
    {
        Project* project = list_get(&projects, i);
        project_destroy(project);
    }

    compiler_shutdown();

    return EXIT_SUCCESS;
}
