#include "assert.h"
#include "ast.h"
#include "attributes_table.h"
#include "operator.h"
#include "resolution.h"
#include "sema.h"

void unary_expr_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_UNARY_EXPR);

    FRX_ASSERT(ctx != NULL);

    ASTUnaryExpr* unary_expr = &ast->unary_expr;

    ast_resolve(unary_expr->operand, ctx);

    const Type* type = expr_infer_type(unary_expr->operand);

    if (unary_expr->operator == FRX_OPERATOR_ADDRESS_OF)
    {
        type = type_intern_ptr(type, FRX_TRUE);
    }
    else if (unary_expr->operator == FRX_OPERATOR_DEREF
        && type->kind == FRX_TYPE_KIND_PTR)
    {
        type = type->ptr.base;
    }
    else if (unary_expr->operator == FRX_OPERATOR_DEREF
        && type->kind == FRX_TYPE_KIND_ARRAY)
    {
        type = type->array.base;
    }

    attributes_table_insert_type(ast->id, type);
}

void unary_expr_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_UNARY_EXPR);

    FRX_ASSERT(ctx != NULL);

    ASTUnaryExpr* unary_expr = &ast->unary_expr;

    ast_sema(unary_expr->operand, ctx);
}
