#include "assert.h"
#include "ast.h"
#include "sema.h"

void unary_expr_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_UNARY_EXPR);

    FRX_ASSERT(ctx != NULL);

    ASTUnaryExpr* unary_expr = &ast->unary_expr;

    ast_sema(unary_expr->operand, ctx);

    // TODO: Type checking
    unary_expr->resolved_type = expr_infer_type(unary_expr->operand);
}
