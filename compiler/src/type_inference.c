#include "type_inference.h"
#include "assert.h"
#include "ast.h"

TypeSpecifier default_type = {
    .kind = FRX_TYPE_KIND_PRIMITIVE,
    .primitive = FRX_TOKEN_TYPE_KW_I32
};

static TypeSpecifier* int_literal_infer_type(IntLiteral* literal)
{
    //TODO: Add other pre-allocated types for different int types if the value is too big for i32
    return &default_type;
}

static TypeSpecifier* unary_expr_infer_type(UnaryExpr* unary_expr)
{
    return expr_infer_type(unary_expr->operand);
}

static TypeSpecifier* binary_expr_infer_type(BinaryExpr* binary_expr)
{
    return expr_infer_type(binary_expr->left);
}

TypeSpecifier* expr_infer_type(Expr* expr)
{
    FRX_ASSERT(expr != NULL);

    switch (expr->type)
    {
        case FRX_EXPR_TYPE_INT_LIT: return int_literal_infer_type(expr->node);
        case FRX_EXPR_TYPE_UNARY_EXPR: return unary_expr_infer_type(expr->node);
        case FRX_EXPR_TYPE_BINARY_EXPR: return binary_expr_infer_type(expr->node);
        default: FRX_ASSERT(FRX_FALSE); break;
    }

    return &default_type;
}
