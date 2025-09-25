#include "ast.h"
#include "parser.h"

AST* break_stmt_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_BREAK_STMT);
    ast->range.start = parser_current_location(parser);

    if (parser_eat(parser, FRX_TOKEN_TYPE_KW_BREAK))
    {
        ast->type = FRX_AST_TYPE_ERROR;
    }

    ast->range.end = parser_current_location(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_SEMI);


    return ast;
}
