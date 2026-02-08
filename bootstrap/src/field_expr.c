#include "ast.h"
#include "assert.h"
#include "sema.h"
#include "symbol.h"

void field_expr_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FIELD_EXPR);

    FRX_ASSERT(ctx != NULL);

    ASTFieldExpr* field_expr = &ast->field_expr;

    ast_sema(field_expr->base, ctx);

    ASTStructDef* struct_def = expr_infer_type(field_expr->base)->strct.symbol->data;

    for (usize i = 0; i < list_size(&struct_def->fields); ++i)
    {
        AST* field = list_get(&struct_def->fields, i);

        if (field->struct_field.name != field_expr->field_name)
        {
            continue;
        }

        const Type* type = expr_infer_type(field_expr->base);
        while (type->kind == FRX_TYPE_KIND_PTR || type->kind == FRX_TYPE_KIND_ARRAY)
        {
            type = type->kind == FRX_TYPE_KIND_PTR ? type->ptr.base : type->array.base;
        }

        if (field->struct_field.visibility == FRX_SYMBOL_VISIBILITY_PRIVATE
            && (ctx->current_impl_block == NULL || expr_infer_type(ctx->current_impl_block->path_expr) != type))
        {
            sema_context_fail(ctx);
        }
    }
}
