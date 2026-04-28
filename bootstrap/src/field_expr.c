#include "ast.h"
#include "assert.h"
#include "attributes_table.h"
#include "resolution.h"
#include "sema.h"
#include "symbol.h"
#include "type_system.h"

void field_expr_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FIELD_EXPR);

    FRX_ASSERT(ctx != NULL);

    ASTFieldExpr* field_expr = &ast->field_expr;

    ast_resolve(field_expr->base, ctx);

    const Type* struct_type = expr_infer_type(field_expr->base);
    while (struct_type->kind == FRX_TYPE_KIND_PTR)
    {
        struct_type = struct_type->ptr.base;
    }

    List* fields = &struct_type->strct.symbol->data->struct_def.fields;

    for (usize i = 0; i < list_size(fields); ++i)
    {
        AST* field = list_get(fields, i);
        if (field->struct_field.name == field_expr->field_name)
        {
            const Type* type = attributes_table_lookup_type(field->struct_field.type->id);
            attributes_table_insert_type(ast->id, type);

            break;
        }
    }
}

void field_expr_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_FIELD_EXPR);

    FRX_ASSERT(ctx != NULL);

    ASTFieldExpr* field_expr = &ast->field_expr;

    ast_sema(field_expr->base, ctx);

    const Type* struct_type = expr_infer_type(field_expr->base);
    FRX_ASSERT(struct_type != NULL);
    if (struct_type->kind == FRX_TYPE_KIND_PTR)
    {
        struct_type = struct_type->ptr.base;
    }

    ASTStructDef* struct_def = &struct_type->strct.symbol->data->struct_def;
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
            && (ctx->current_impl_block == NULL || symbol_infer_type(ctx->current_impl_block->type_path->path.symbol) != type))
        {
            sema_context_fail(ctx);
        }
    }
}
