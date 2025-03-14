#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "token.h"

static CallExpr* call_expr_create(void)
{
    CallExpr* call_expr = compiler_alloc_ast(sizeof(CallExpr));

    list_init(&call_expr->args);

    return call_expr;
}

static void call_expr_add_arg(CallExpr* call_expr, Expr* arg)
{
    FRX_ASSERT(call_expr != NULL);

    FRX_ASSERT(arg != NULL);

    list_add(&call_expr->args, arg);
}

CallExpr* call_expr_parse(Parser* parser)
{
    parser_eat(parser, FRX_TOKEN_TYPE_LPAREN);

    CallExpr* call_expr = call_expr_create();
    while (!parser_match(parser, FRX_TOKEN_TYPE_RPAREN))
    {
        if (!list_empty(&call_expr->args))
        {
            parser_eat(parser, FRX_TOKEN_TYPE_COMMA);
        }

        Expr* arg = expr_parse(parser);
        call_expr_add_arg(call_expr, arg);
    }

    parser_eat(parser, FRX_TOKEN_TYPE_RPAREN);

    return call_expr;
}
