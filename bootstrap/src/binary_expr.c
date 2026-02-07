#include "assert.h"
#include "ast.h"
#include "sema.h"

void binary_expr_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_BINARY_EXPR);

    ASTBinaryExpr* binary_expr = &ast->binary_expr;

    ast_sema(binary_expr->left, ctx);
    ast_sema(binary_expr->right, ctx);

    // TODO: Type checking
    binary_expr->resolved_type = expr_infer_type(binary_expr->left);
}
