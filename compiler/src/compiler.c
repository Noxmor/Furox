#include "compiler.h"

#include <stdlib.h>
#include <string.h>

#include "assert.h"
#include "log.h"
#include "arena.h"
#include "lexer.h"
#include "module.h"
#include "project.h"
#include "string_table.h"

static Arena* arena;
static Arena* ast_arena;
static Arena* mir_arena;

static List projects;

static void compiler_init(void)
{
    FRX_LOG_INFO("Initializing compiler...");

    lexer_init_keyword_table();

    arena = arena_create();
    ast_arena = arena_create();
    mir_arena = arena_create();
    list_init(&projects);

    ProjectSpecificiation spec;
    spec.type = FRX_PROJECT_TYPE_LIB;

    Project* stdlib = project_create(spec, "/usr/local/lib/furox/std");
    list_add(&projects, stdlib);
}

static void compiler_shutdown(void)
{
    FRX_LOG_INFO("Shutting down compiler...");

    string_table_shutdown();

    arena_destroy(arena);
    arena_destroy(ast_arena);
    arena_destroy(mir_arena);
}

void* compiler_alloc(usize size)
{
    return arena_alloc(arena, size);
}

void* compiler_alloc_ast(usize size)
{
    return arena_alloc(ast_arena, size);
}

void* compiler_alloc_mir(usize size)
{
    return arena_alloc(mir_arena, size);
}

int compiler_run(int argc, char** argv)
{
    compiler_init();

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

    for (usize i = 0; i < list_size(&projects); ++i)
    {
        Project* project = list_get(&projects, i);
        if (project_codegen(project))
        {
            return EXIT_FAILURE;
        }
    }

    for (usize i = 0; i < list_size(&projects); ++i)
    {
        Project* project = list_get(&projects, i);
        project_destroy(project);
    }

    compiler_shutdown();

    return EXIT_SUCCESS;
}

Module* compiler_find_module_by_path_segments(const List* path_segments)
{
    FRX_ASSERT(path_segments != NULL);

    for (usize i = 0; i < list_size(&projects); ++i)
    {
        Project* project = list_get(&projects, i);
        Module* mod = project_find_module_by_path_segments(project, path_segments);
        if (mod != NULL)
        {
            return mod;
        }
    }

    return NULL;
}
