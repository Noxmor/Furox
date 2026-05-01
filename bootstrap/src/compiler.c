#include "compiler.h"

#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <sys/stat.h>

#include "assert.h"
#include "parser.h"
#include "early_resolution.h"
#include "late_resolution.h"
#include "sema.h"
#include "type_system.h"
#include "codegen.h"
#include "log.h"
#include "arena.h"
#include "module.h"
#include "string_table.h"
#include "source_map.h"
#include "temp_dir.h"
#include "config.h"

#define FRX_STDLIB_SRC_DIR "/usr/local/lib/furox/stdlib/src"

static Arena* arena;
static Arena* ast_arena;

static Module* root_module;

static List type_list;

static List diagnostics;

static void compiler_init(void)
{
    FRX_LOG_INFO("Initializing compiler...");

    temp_dir_init();

    lexer_init_keyword_table();
    source_map_init();

    arena = arena_create();
    ast_arena = arena_create();

    root_module = module_create_root();

    type_system_init();

    list_init(&type_list);

    list_init(&diagnostics);
}

static void compiler_shutdown(void)
{
    FRX_LOG_INFO("Shutting down compiler...");

    string_table_shutdown();

    arena_destroy(arena);
    arena_destroy(ast_arena);
}

static void compiler_emit_diagnostics(void)
{
    for (usize i = 0; i < list_size(&diagnostics); ++i)
    {
        const Diagnostic* d = list_get(&diagnostics, i);
        diagnostic_emit(d);
    }
}

static b8 is_source_file(const char* filepath)
{
    FRX_ASSERT(filepath != NULL);

    const char* ext = strrchr(filepath, '.');

    return ext && strcmp(ext, ".frx") == 0;
}

static void walk_directory(const char* path)
{
    DIR* dir = opendir(path);
    if (!dir)
    {
        return;
    }

    struct dirent* entry;
    char full_path[PATH_MAX];

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        struct stat st;
        if (stat(full_path, &st) != 0)
        {
            continue;
        }

        if (S_ISDIR(st.st_mode))
        {
            walk_directory(full_path);
        }
        else if (S_ISREG(st.st_mode) && is_source_file(full_path))
        {
            source_map_add_source_file(full_path);
        }
    }

    closedir(dir);
}

static void compiler_compile_stdlib(void)
{
    walk_directory(FRX_STDLIB_SRC_DIR);
}

void* compiler_alloc(usize size)
{
    return arena_alloc(arena, size);
}

void* compiler_alloc_ast(usize size)
{
    return arena_alloc(ast_arena, size);
}

int compiler_run(int argc, char** argv)
{
    compiler_init();

    config_parse(argc, argv);

    if (config_get()->use_stdlib == FRX_TRUE)
    {
        compiler_compile_stdlib();
    }

    for (int i = config_get_optind(); i < argc; ++i)
    {
        const char* filepath = argv[i];

        if (is_source_file(filepath))
        {
            source_map_add_source_file(filepath);
        }
    }

    List* src_files = source_map_get_source_files();

    u8 parsing_failed = FRX_FALSE;
    for (usize i = 0; i < list_size(src_files); ++i)
    {
        SourceFile* src_file = list_get(src_files, i);
        Parser parser;
        parser_init(&parser, src_file);
        src_file->ast = parser_parse(&parser);
        parsing_failed |= parser_failed(&parser);
    }

    if (parsing_failed)
    {
        compiler_emit_diagnostics();
        return EXIT_FAILURE;
    }

    b8 resolution_failed = FRX_FALSE;
    for (usize i = 0; i < list_size(src_files); ++i)
    {
        SourceFile* src_file = list_get(src_files, i);
        ResolutionContext ctx;
        resolution_context_init(&ctx, src_file, root_module);
        translation_unit_resolve_early(src_file->ast, &ctx);
        resolution_failed |= resolution_context_failed(&ctx);
    }

    for (usize i = 0; i < list_size(src_files); ++i)
    {
        SourceFile* src_file = list_get(src_files, i);
        ResolutionContext ctx;
        resolution_context_init(&ctx, src_file, root_module);
        translation_unit_resolve_late(src_file->ast, &ctx);
        resolution_failed |= resolution_context_failed(&ctx);
    }

    if (resolution_failed)
    {
        compiler_emit_diagnostics();
        return EXIT_FAILURE;
    }

    b8 sema_failed = FRX_FALSE;
    for (usize i = 0; i < list_size(src_files); ++i)
    {
        SourceFile* src_file = list_get(src_files, i);
        SemaContext ctx;
        sema_context_init(&ctx, src_file);
        ast_sema(src_file->ast, &ctx);
        sema_failed |= sema_context_failed(&ctx);
    }

    if (sema_failed)
    {
        compiler_emit_diagnostics();
        return EXIT_FAILURE;
    }

    CodegenContext ctx;
    codegen_context_init(&ctx, root_module, src_files, "frx");
    codegen_context_transpile(&ctx);
    codegen_context_end(&ctx);

    const Config* config = config_get();
    const char* temp_dir = temp_dir_path();
    char command[strlen("gcc -o ") + strlen(config->output) + 1 + strlen(temp_dir) + strlen("/frx.c") + 1];
    sprintf(command, "gcc -o %s %s/frx.c", config->output, temp_dir);
    FRX_LOG_INFO("Executing command: %s\n", command);
    if (system(command) != 0)
    {
        return EXIT_FAILURE;
    }

    compiler_shutdown();

    return EXIT_SUCCESS;
}

Module* compiler_root_module(void)
{
    return root_module;
}

void compiler_register_type(const Type* type)
{
    FRX_ASSERT(type != NULL);

    list_add(&type_list, (Type*)type);
}

List* compiler_get_types(void)
{
    return &type_list;
}

void compiler_add_diagnostic(Diagnostic* d)
{
    FRX_ASSERT(d != NULL);

    list_add(&diagnostics, d);
}
