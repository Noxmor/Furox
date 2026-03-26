#include "assert.h"
#include "parser.h"

static void bool_expr_init(ASTBoolExpr* bool_expr, b8 value)
{
    FRX_ASSERT(bool_expr != NULL);

    bool_expr->value = value;
}

AST* bool_expr_parse(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    AST* ast = ast_create(FRX_AST_TYPE_BOOL_EXPR);
    ASTBoolExpr* bool_expr = &ast->bool_expr;

    ast->span = parser_current_span(parser);

    b8 value;
    if (parser_current_type(parser) == FRX_TOKEN_TYPE_KW_TRUE)
    {
        value = FRX_TRUE;
        parser_eat(parser, FRX_TOKEN_TYPE_KW_TRUE);
    }
    else
    {
        value = FRX_FALSE;
        parser_eat(parser, FRX_TOKEN_TYPE_KW_FALSE);
    }

    bool_expr_init(bool_expr, value);

    return ast;
}
