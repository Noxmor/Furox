#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "diagnostics.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"

void unary_expr_resolve(Parser* parser, UnaryExpr* unary_expr)
{
    FRX_ASSERT(unary_expr != NULL);

    expr_resolve(parser, unary_expr->operand);
}

void unary_expr_sema(UnaryExpr* unary_expr)
{
    FRX_ASSERT(unary_expr != NULL);

    expr_sema(unary_expr->operand);
}
