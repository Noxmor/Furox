#include "assert.h"
#include "ast.h"
#include "codegen.h"
#include "compiler.h"
#include "diagnostics.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"

void binary_expr_resolve(Parser* parser, BinaryExpr* binary_expr)
{
    FRX_ASSERT(binary_expr != NULL);

    expr_resolve(parser, binary_expr->left);
    expr_resolve(parser, binary_expr->right);
}

void binary_expr_sema(BinaryExpr* binary_expr)
{
    FRX_ASSERT(binary_expr != NULL);

    expr_sema(binary_expr->left);
    expr_sema(binary_expr->right);
}

void binary_expr_codegen(BinaryExpr* binary_expr, CodegenContext* ctx)
{
    FRX_ASSERT(binary_expr != NULL);

    FRX_ASSERT(ctx != NULL);

    fprintf(ctx->source, "(");
    expr_codegen(binary_expr->left, ctx);
    fprintf(ctx->source, " %s ", token_type_to_str(binary_expr->type));
    expr_codegen(binary_expr->right, ctx);
    fprintf(ctx->source, ")");
}
