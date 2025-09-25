#include "assert.h"
#include "ast.h"
#include "resolution.h"
#include "sema.h"

void binary_expr_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_BINARY_EXPR);

    ASTBinaryExpr* binary_expr = &ast->binary_expr;

    ast_resolve(binary_expr->left, ctx);
    ast_resolve(binary_expr->right, ctx);
}

void binary_expr_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_BINARY_EXPR);

    ASTBinaryExpr* binary_expr = &ast->binary_expr;

    ast_sema(binary_expr->left, ctx);
    ast_sema(binary_expr->right, ctx);
}
