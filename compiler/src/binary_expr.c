#include "assert.h"
#include "ast.h"
#include "codegen.h"
#include "compiler.h"
#include "diagnostics.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"

void binary_expr_resolve(AST* ast, Parser* parser)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_BINARY_EXPR);

    BinaryExpr* binary_expr = &ast->binary_expr;

    ast_resolve(binary_expr->left, parser);
    ast_resolve(binary_expr->right, parser);
}

void binary_expr_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_BINARY_EXPR);

    BinaryExpr* binary_expr = &ast->binary_expr;

    ast_sema(binary_expr->left, ctx);
    ast_sema(binary_expr->right, ctx);
}

void binary_expr_codegen(AST* ast, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_BINARY_EXPR);

    FRX_ASSERT(ctx != NULL);

    BinaryExpr* binary_expr = &ast->binary_expr;

    fprintf(ctx->source, "(");
    ast_codegen(binary_expr->left, ctx);
    fprintf(ctx->source, " %s ", token_type_to_str(binary_expr->type));
    ast_codegen(binary_expr->right, ctx);
    fprintf(ctx->source, ")");
}
