#include "ast.h"
#include "parser.h"

AST* break_stmt_parse(Parser* parser)
{
    AST* ast = parser_create_ast(parser, FRX_AST_TYPE_BREAK_STMT);
    ast->span.lo = parser_current_span(parser).lo;

    if (parser_eat(parser, FRX_TOKEN_TYPE_KW_BREAK))
    {
        ast->type = FRX_AST_TYPE_ERROR;
    }

    ast->span.hi = parser_current_span(parser).hi;
    parser_eat(parser, FRX_TOKEN_TYPE_SEMI);


    return ast;
}
