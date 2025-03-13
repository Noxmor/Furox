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
#include "codegen.h"

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

#include "mir.h"

static b8 compiler_mir_test(void)
{
    MIRContext* ctx = mir_context_create();

    MIRFuncContext* func = mir_func_context_create("main", mir_type_create_primitive(FRX_MIR_TYPE_KIND_I32));

    MIRVariable* argc = mir_variable_create_func_param(mir_type_create_primitive(FRX_MIR_TYPE_KIND_I32), "argc");
    mir_func_context_add_param(func, argc);
    mir_func_context_add_param(func, mir_variable_create_func_param(mir_type_create_ptr(mir_type_create_ptr(mir_type_create_primitive(FRX_MIR_TYPE_KIND_U8))), "argv"));

    func->block = mir_block_create();
    MIRVariable* call_result = mir_variable_create_temp(mir_type_create_primitive(FRX_MIR_TYPE_KIND_I32), 1);
    MIRInstruction* call_instruction = mir_instruction_create_call(call_result, "add");
    MIRVariable* call_arg_a = argc;
    MIRVariable* call_arg_b = argc;
    mir_instruction_add_func_arg(call_instruction, call_arg_a);
    mir_instruction_add_func_arg(call_instruction, call_arg_b);
    mir_block_add_instruction(func->block, call_instruction);
    mir_block_add_instruction(func->block, mir_instruction_create(FRX_MIR_INSTRUCTION_TYPE_RET, call_result, NULL, NULL));

    MIRFuncContext* add = mir_func_context_create("add", mir_type_create_primitive(FRX_MIR_TYPE_KIND_I32));

    MIRVariable* param_a = mir_variable_create_func_param(mir_type_create_primitive(FRX_MIR_TYPE_KIND_I32), "a");
    MIRVariable* param_b = mir_variable_create_func_param(mir_type_create_primitive(FRX_MIR_TYPE_KIND_I32), "b");
    mir_func_context_add_param(add, param_a);
    mir_func_context_add_param(add, param_b);

    add->block = mir_block_create();
    MIRVariable* result = mir_variable_create_temp(mir_type_create_primitive(FRX_MIR_TYPE_KIND_I32), 1);
    mir_block_add_instruction(add->block, mir_instruction_create(FRX_MIR_INSTRUCTION_TYPE_ADD, result, param_a, param_b));
    mir_block_add_instruction(add->block, mir_instruction_create(FRX_MIR_INSTRUCTION_TYPE_RET, result, NULL, NULL));


    mir_context_add_func(ctx, add);
    mir_context_add_func(ctx, func);

    mir_context_emit_c(ctx);

    return system("gcc frx.c") != 0;
}

int compiler_run(int argc, char** argv)
{
    compiler_init();

    //TODO: Remove (only for testing)
    if (argc == 2 && strcmp(argv[1], "mir") == 0)
    {
        return compiler_mir_test();
    }

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

    b8 success = system("gcc frx.c") != 0;

    for (usize i = 0; i < list_size(&projects); ++i)
    {
        Project* project = list_get(&projects, i);
        project_destroy(project);
    }

    compiler_shutdown();

    return success;
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
