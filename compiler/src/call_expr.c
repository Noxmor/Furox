#include "assert.h"
#include "ast.h"
#include "parser.h"
#include "sema.h"
#include "codegen.h"
#include "token.h"

static void call_expr_add_arg(CallExpr* call_expr, AST* arg)
{
    FRX_ASSERT(call_expr != NULL);

    FRX_ASSERT(arg != NULL);

    list_add(&call_expr->args, arg);
}

AST* call_expr_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_CALL_EXPR);
    CallExpr* call_expr = &ast->call_expr;
    ast->range.start = parser_current_location(parser);

    while (!parser_match(parser, FRX_TOKEN_TYPE_RPAREN))
    {
        if (!list_empty(&call_expr->args))
        {
            parser_eat(parser, FRX_TOKEN_TYPE_COMMA);
        }

        AST* arg = expr_parse(parser);
        call_expr_add_arg(call_expr, arg);
    }

    parser_eat(parser, FRX_TOKEN_TYPE_RPAREN);

    return ast;
}

void call_expr_resolve(AST* ast, Parser* parser)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_CALL_EXPR);

    // TODO: Implement
    (void)parser;
}

void call_expr_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_CALL_EXPR);

    FRX_ASSERT(ctx != NULL);

    // TODO: Implement
    (void)ctx;
}

void call_expr_codegen(AST* ast, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_CALL_EXPR);

    FRX_ASSERT(ctx != NULL);

    CallExpr* call_expr = &ast->call_expr;

    for (usize i = 0; i < list_size(&call_expr->args); ++i)
    {
        if (i > 0)
        {
            fprintf(ctx->source, ", ");
        }

        AST* arg = list_get(&call_expr->args, i);
        ast_codegen(arg, ctx);

    }

    fprintf(ctx->source, ")");
}
