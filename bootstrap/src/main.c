#include <stdlib.h>

#include "parser.h"
#include "transpiler.h"

static FRX_NO_DISCARD b8 compile(const char* filepath)
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

    Transpiler transpiler;
    transpiler.symbol_table = &parser.symbol_table;

    if(ast_transpile_program(&transpiler, parser.program, parser.lexer.filepath))
    {
        fprintf(stderr, "%s: ast_transpile_program() failed!\n", filepath);
        return FRX_TRUE;
    }

    return FRX_FALSE;
}

int main(int argc, char** argv)
{
    lexer_init_keyword_table();

    for(int i = 1; i < argc; ++i)
    {
        if(compile(argv[i]))
            return EXIT_FAILURE;
    }

    if(generate_executable())
    {
        fprintf(stderr, "generate_executable() failed!\n");
        return 1;
    }

    return EXIT_SUCCESS;
}
