#include "assert.h"
#include "ast.h"
#include "parser.h"

AST* nullptr_expr_parse(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    AST* ast = ast_create(FRX_AST_TYPE_NULLPTR_EXPR);

    ast->span = parser_current_span(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_KW_NULLPTR);

    return ast;
}
