#include "parser.h"

#include "core/memory.h"

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

    return 0;    
}
