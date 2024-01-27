#include "core/memory.h"

#include "parser.h"
#include "transpiler.h"

int main(void)
{
    Parser parser;
    
    if(parser_init(&parser, "input.txt"))
    {
        printf("parser_init() failed!\n");
        return 1;
    }

    if(parser_parse(&parser))
    {
        printf("parser_parse() failed!\n");
        return 1;
    }

    ast_print(&parser.root);

    if(transpile_ast(&parser.root, parser.lexer.filepath))
    {
        printf("transpile_ast() failed!\n");
        return 1;
    }

    return 0;    
}
