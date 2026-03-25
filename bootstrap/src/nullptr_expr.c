#include "assert.h"
#include "parser.h"

AST* nullptr_expr_parse(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    parser_eat(parser, FRX_TOKEN_TYPE_KW_NULLPTR);

    return ast_create(FRX_AST_TYPE_NULLPTR_EXPR);
}
