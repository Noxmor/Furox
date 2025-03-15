#include "assert.h"
#include "ast.h"
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
