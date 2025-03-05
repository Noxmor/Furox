#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "diagnostics.h"
#include "parser.h"
#include "sema.h"
#include "codegen.h"

void unary_expr_sema(UnaryExpr* unary_expr)
{
    FRX_ASSERT(unary_expr != NULL);

    expr_sema(unary_expr->operand);
}

void unary_expr_codegen(UnaryExpr* unary_expr)
{
    FRX_ASSERT(unary_expr != NULL);

    codegen_write("%s(", token_type_to_str(unary_expr->type));
    expr_codegen(unary_expr->operand);
    codegen_write(")");
}
