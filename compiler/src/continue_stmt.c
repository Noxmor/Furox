#include "ast.h"
#include "parser.h"

AST* continue_stmt_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_CONTINUE_STMT);
    ast->range.start = parser_current_location(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_KW_CONTINUE);
    ast->range.end = parser_current_location(parser);
    parser_eat(parser, FRX_TOKEN_TYPE_SEMI);

    return ast;
}
