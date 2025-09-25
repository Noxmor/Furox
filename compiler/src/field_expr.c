#include "ast.h"
#include "assert.h"
#include "resolution.h"
#include "sema.h"

void field_expr_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FIELD_EXPR);

    FRX_ASSERT(ctx != NULL);

    ASTFieldExpr* field_expr = &ast->field_expr;

    ast_resolve(field_expr->base, ctx);
}

void field_expr_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FIELD_EXPR);

    FRX_ASSERT(ctx != NULL);

    ASTFieldExpr* field_expr = &ast->field_expr;

    ast_sema(field_expr->base, ctx);
}
