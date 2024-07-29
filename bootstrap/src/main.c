#include <stdlib.h>

#include "core/memory.h"
#include "parser.h"
#include "transpiler.h"

static FRX_NO_DISCARD b8 compile(const char* filepath, ASTProgram* program)
{
    Parser parser;

    if(parser_init(&parser, filepath))
    {
        fprintf(stderr, "%s: parser_init() failed!\n", filepath);
        return FRX_TRUE;
    }

    if(parser_parse(&parser))
    {
        fprintf(stderr, "%s: parser_parse() failed!\n", filepath);
        return FRX_TRUE;
    }

    ast_print_program(parser.program);

    for(usize i = 0; i < list_size(&parser.program->top_level_definitions); ++i)
    {
        AST* top_level_definition = list_get(&parser.program->top_level_definitions, i);
        list_push(&program->top_level_definitions, top_level_definition);
    }

    /*Transpiler transpiler;

    if(ast_transpile_program(&transpiler, parser.program, parser.lexer.filepath))
    {
        fprintf(stderr, "%s: ast_transpile_program() failed!\n", filepath);
        return FRX_TRUE;
    }*/

    return FRX_FALSE;
}

static FRX_NO_DISCARD b8 compile_program(ASTProgram* program, const char* filepath)
{
    Transpiler transpiler;

    if(ast_transpile_program(&transpiler, program, filepath))
    {
        fprintf(stderr, "%s: ast_transpile_program() failed!\n", filepath);
        return FRX_TRUE;
    }

    return FRX_FALSE;
}

int main(int argc, char** argv)
{
    lexer_init_keyword_table();
    init_global_symbol_table();

    ASTProgram program;
    list_init(&program.top_level_definitions, FRX_MEMORY_CATEGORY_AST);

    for(int i = 1; i < argc; ++i)
    {
        if(compile(argv[i], &program))
            return EXIT_FAILURE;
    }

    if(compile_program(&program, "a.out"))
        return EXIT_FAILURE;

    if(generate_executable())
    {
        fprintf(stderr, "generate_executable() failed!\n");
        return 1;
    }

    return EXIT_SUCCESS;
}
