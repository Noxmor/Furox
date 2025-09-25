#include "type_inference.h"

#include "assert.h"

Type default_type = {
    .kind = FRX_TYPE_KIND_PRIMITIVE,
    .primitive_type = FRX_TOKEN_TYPE_KW_I32
};

static Type* int_literal_infer_type(const ASTIntLiteral* literal)
{
    //TODO: Add other pre-allocated types for different int types if the value is too big for i32
    (void)literal;

    return &default_type;
}

static Type* unary_expr_infer_type(const ASTUnaryExpr* unary_expr)
{
    return expr_infer_type(unary_expr->operand);
}

static Type* binary_expr_infer_type(const ASTBinaryExpr* binary_expr)
{
    return expr_infer_type(binary_expr->left);
}

Type* expr_infer_type(const AST* expr)
{
    FRX_ASSERT(expr != NULL);

    switch (expr->type)
    {
        case FRX_AST_TYPE_INT_LIT: return int_literal_infer_type(&expr->int_literal);
        case FRX_AST_TYPE_UNARY_EXPR: return unary_expr_infer_type(&expr->unary_expr);
        case FRX_AST_TYPE_BINARY_EXPR: return binary_expr_infer_type(&expr->binary_expr);
        default: FRX_ASSERT(FRX_FALSE); break;
    }

    return &default_type;
}
