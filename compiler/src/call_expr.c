#include "assert.h"
#include "ast.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "token.h"

static void call_expr_add_arg(ASTCallExpr* call_expr, AST* arg)
{
    FRX_ASSERT(call_expr != NULL);

    FRX_ASSERT(arg != NULL);

    list_add(&call_expr->args, arg);
}

AST* call_expr_parse(Parser* parser, AST* callee)
{
    AST* ast = ast_create(FRX_AST_TYPE_CALL_EXPR);
    ASTCallExpr* call_expr = &ast->call_expr;
    list_init(&call_expr->args);
    call_expr->callee = callee;
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

void call_expr_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_CALL_EXPR);

    (void)ctx;

    ASTCallExpr* call_expr = &ast->call_expr;

    const Symbol* symbol = call_expr->callee->path_expr.symbol;
    if (symbol->associated_type != NULL)
    {
        const char* name = list_get(&call_expr->callee->path_expr.path_segments, list_size(&call_expr->callee->path_expr.path_segments));
        List args = call_expr->args;

        ast->type = FRX_AST_TYPE_METHOD_CALL_EXPR;
        ASTMethodCallExpr* method_call_expr = &ast->method_call_expr;
        method_call_expr->callee = NULL;
        method_call_expr->symbol = symbol;
        method_call_expr->name = name;
        method_call_expr->args = args;
        method_call_expr->resolved_type = symbol_infer_type(method_call_expr->symbol);
    }
}

void call_expr_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_CALL_EXPR);

    FRX_ASSERT(ctx != NULL);

    ASTCallExpr* call_expr = &ast->call_expr;

    ast_sema(call_expr->callee, ctx);

    for (usize i = 0; i < list_size(&call_expr->args); ++i)
    {
        AST* arg = list_get(&call_expr->args, i);
        ast_sema(arg, ctx);
    }
}
