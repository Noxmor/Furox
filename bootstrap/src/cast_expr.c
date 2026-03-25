#include "assert.h"
#include "resolution.h"
#include "sema.h"

void cast_expr_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_CAST_EXPR);

    FRX_ASSERT(ctx != NULL);

    ASTCastExpr* cast_expr = &ast->cast_expr;

    ast_resolve(cast_expr->expr, ctx);
    type_specifier_resolve(cast_expr->type_specifier, ctx);
}

void cast_expr_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_CAST_EXPR);

    FRX_ASSERT(ctx != NULL);

    ASTCastExpr* cast_expr = &ast->cast_expr;

    ast_sema(cast_expr->expr, ctx);
}
