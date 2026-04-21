#include "ast.h"
#include "assert.h"
#include "early_resolution.h"
#include "late_resolution.h"
#include "sema.h"
#include "compiler.h"
#include "symbol.h"
#include "type_system.h"
#include "attributes_table.h"

AST* ast_create(ASTType type, ASTNodeID id)
{
    FRX_ASSERT(type < FRX_AST_TYPE_COUNT);

    AST* ast = compiler_alloc_ast(sizeof(AST));

    ast->type = type;
    ast->id = id;

    return ast;
}

void ast_resolve_early(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    switch (ast->type)
    {
        case FRX_AST_TYPE_TRANSLATION_UNIT: translation_unit_resolve_early(ast, ctx); break;
        case FRX_AST_TYPE_USE_STMT: use_stmt_resolve_early(ast, ctx); break;
        case FRX_AST_TYPE_STATIC: static_resolve_early(ast, ctx); break;
        case FRX_AST_TYPE_TYPE_ALIAS: return type_alias_resolve_early(ast, ctx); break;
        case FRX_AST_TYPE_ENUM_DEF: enum_def_resolve_early(ast, ctx); break;
        case FRX_AST_TYPE_STRUCT_DEF: struct_def_resolve_early(ast, ctx); break;
        case FRX_AST_TYPE_TRAIT: trait_resolve_early(ast, ctx); break;
        case FRX_AST_TYPE_IMPL_BLOCK: impl_block_resolve_early(ast, ctx); break;
        case FRX_AST_TYPE_FUNC_DECL: func_decl_resolve_early(ast, ctx); break;

        default: FRX_ASSERT(FRX_FALSE); break;
    }
}

void ast_resolve_late(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    switch (ast->type)
    {
        case FRX_AST_TYPE_TRANSLATION_UNIT: translation_unit_resolve_late(ast, ctx); break;
        case FRX_AST_TYPE_USE_STMT: break;
        case FRX_AST_TYPE_STATIC: break;
        case FRX_AST_TYPE_TYPE_ALIAS: break;
        case FRX_AST_TYPE_ENUM_DEF: break;
        case FRX_AST_TYPE_STRUCT_DEF: break;
        case FRX_AST_TYPE_TRAIT: break;
        case FRX_AST_TYPE_IMPL_BLOCK: impl_block_resolve_late(ast, ctx); break;
        case FRX_AST_TYPE_FUNC_DECL: func_decl_resolve_late(ast, ctx); break;

        default: FRX_ASSERT(FRX_FALSE); break;
    }
}

void ast_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    switch (ast->type)
    {
        case FRX_AST_TYPE_ERROR: break;
        case FRX_AST_TYPE_TYPE_SPECIFIER: type_specifier_resolve(ast, ctx); break;
        case FRX_AST_TYPE_GENERIC_PARAMS: break;
        case FRX_AST_TYPE_DEFER_STMT: defer_stmt_resolve(ast, ctx); break;
        case FRX_AST_TYPE_BLOCK: block_resolve(ast, ctx); break;
        case FRX_AST_TYPE_EXPR_STMT: expr_stmt_resolve(ast, ctx); break;
        case FRX_AST_TYPE_BREAK_STMT: break;
        case FRX_AST_TYPE_CONTINUE_STMT: break;
        case FRX_AST_TYPE_RETURN_STMT: return_stmt_resolve(ast, ctx); break;
        case FRX_AST_TYPE_LET_STMT: let_stmt_resolve(ast, ctx); break;
        case FRX_AST_TYPE_IF_STMT: if_stmt_resolve(ast, ctx); break;
        case FRX_AST_TYPE_FOR_LOOP: for_loop_resolve(ast, ctx); break;
        case FRX_AST_TYPE_WHILE_LOOP: while_loop_resolve(ast, ctx); break;
        case FRX_AST_TYPE_LOOP: loop_resolve(ast, ctx); break;
        case FRX_AST_TYPE_UNARY_EXPR: unary_expr_resolve(ast, ctx); break;
        case FRX_AST_TYPE_BINARY_EXPR: binary_expr_resolve(ast, ctx); break;
        case FRX_AST_TYPE_FIELD_EXPR: field_expr_resolve(ast, ctx); break;
        case FRX_AST_TYPE_SELF_EXPR: self_expr_resolve(ast, ctx); break;
        case FRX_AST_TYPE_BOOL_EXPR: break;
        case FRX_AST_TYPE_NULLPTR_EXPR: break;
        case FRX_AST_TYPE_PATH_EXPR: path_expr_resolve(ast, ctx); break;
        case FRX_AST_TYPE_CALL_EXPR: call_expr_resolve(ast, ctx); break;
        case FRX_AST_TYPE_METHOD_CALL_EXPR: method_call_expr_resolve(ast, ctx); break;
        case FRX_AST_TYPE_CAST_EXPR: cast_expr_resolve(ast, ctx); break;
        case FRX_AST_TYPE_SIZEOF_EXPR: sizeof_expr_resolve(ast, ctx); break;
        case FRX_AST_TYPE_INT_LIT: int_literal_resolve(ast, ctx); break;
        case FRX_AST_TYPE_CHAR_LIT: break;
        case FRX_AST_TYPE_STRING_LIT: break;
        case FRX_AST_TYPE_STRUCT_LIT: struct_literal_resolve(ast, ctx); break;
        default: FRX_ASSERT(FRX_FALSE); break;
    }
}

void ast_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    switch (ast->type)
    {
        case FRX_AST_TYPE_ERROR: break;
        case FRX_AST_TYPE_TRANSLATION_UNIT: translation_unit_sema(ast, ctx); break;
        case FRX_AST_TYPE_USE_STMT: break;
        case FRX_AST_TYPE_STATIC: static_sema(ast, ctx); break;
        case FRX_AST_TYPE_TYPE_SPECIFIER: break;
        case FRX_AST_TYPE_TYPE_ALIAS: break;
        case FRX_AST_TYPE_STRUCT_DEF: break;
        case FRX_AST_TYPE_ENUM_DEF: enum_def_sema(ast, ctx); break;
        case FRX_AST_TYPE_TRAIT: trait_sema(ast, ctx); break;
        case FRX_AST_TYPE_IMPL_BLOCK: impl_block_sema(ast, ctx); break;
        case FRX_AST_TYPE_GENERIC_PARAMS: break;
        case FRX_AST_TYPE_FUNC_DECL: func_decl_sema(ast, ctx); break;
        case FRX_AST_TYPE_DEFER_STMT: defer_stmt_sema(ast, ctx); break;
        case FRX_AST_TYPE_BLOCK: block_sema(ast, ctx); break;
        case FRX_AST_TYPE_EXPR_STMT: expr_stmt_sema(ast, ctx); break;
        case FRX_AST_TYPE_NULLPTR_EXPR: break;
        case FRX_AST_TYPE_BREAK_STMT: break;
        case FRX_AST_TYPE_CONTINUE_STMT: break;
        case FRX_AST_TYPE_RETURN_STMT: return_stmt_sema(ast, ctx); break;
        case FRX_AST_TYPE_LET_STMT: let_stmt_sema(ast, ctx); break;
        case FRX_AST_TYPE_IF_STMT: if_stmt_sema(ast, ctx); break;
        case FRX_AST_TYPE_FOR_LOOP: for_loop_sema(ast, ctx); break;
        case FRX_AST_TYPE_WHILE_LOOP: while_loop_sema(ast, ctx); break;
        case FRX_AST_TYPE_LOOP: loop_sema(ast, ctx); break;
        case FRX_AST_TYPE_UNARY_EXPR: unary_expr_sema(ast, ctx); break;
        case FRX_AST_TYPE_BINARY_EXPR: binary_expr_sema(ast, ctx); break;
        case FRX_AST_TYPE_FIELD_EXPR: field_expr_sema(ast, ctx); break;
        case FRX_AST_TYPE_SELF_EXPR: break;
        case FRX_AST_TYPE_BOOL_EXPR: break;
        case FRX_AST_TYPE_PATH_EXPR: break;
        case FRX_AST_TYPE_CALL_EXPR: call_expr_sema(ast, ctx); break;
        case FRX_AST_TYPE_METHOD_CALL_EXPR: method_call_expr_sema(ast, ctx); break;
        case FRX_AST_TYPE_CAST_EXPR: cast_expr_sema(ast, ctx); break;
        case FRX_AST_TYPE_SIZEOF_EXPR: sizeof_expr_sema(ast, ctx); break;
        case FRX_AST_TYPE_INT_LIT: break;
        case FRX_AST_TYPE_CHAR_LIT: break;
        case FRX_AST_TYPE_STRING_LIT: break;
        case FRX_AST_TYPE_STRUCT_LIT: struct_literal_sema(ast, ctx); break;
        default: FRX_ASSERT(FRX_FALSE); break;
    }
}


const Type* expr_infer_type(AST* expr)
{
    FRX_ASSERT(expr != NULL);

    switch (expr->type)
    {
        case FRX_AST_TYPE_UNARY_EXPR: return attributes_table_lookup_type(expr->id);
        case FRX_AST_TYPE_BINARY_EXPR: return attributes_table_lookup_type(expr->id);
        case FRX_AST_TYPE_FIELD_EXPR: return expr->field_expr.resolved_type;
        case FRX_AST_TYPE_SELF_EXPR: return attributes_table_lookup_type(expr->id);
        case FRX_AST_TYPE_BOOL_EXPR: return type_intern_bool();
        case FRX_AST_TYPE_NULLPTR_EXPR: return type_intern_nullptr();
        case FRX_AST_TYPE_PATH_EXPR: return attributes_table_lookup_type(expr->id);
        case FRX_AST_TYPE_CALL_EXPR: return attributes_table_lookup_type(expr->id);
        case FRX_AST_TYPE_METHOD_CALL_EXPR: return attributes_table_lookup_type(expr->id);
        case FRX_AST_TYPE_CAST_EXPR: return attributes_table_lookup_type(expr->cast_expr.type_specifier->id);
        case FRX_AST_TYPE_SIZEOF_EXPR: return type_intern_primitive(FRX_TOKEN_TYPE_KW_USIZE);
        case FRX_AST_TYPE_INT_LIT: return attributes_table_lookup_type(expr->id);
        case FRX_AST_TYPE_CHAR_LIT: return type_intern_char_lit();
        case FRX_AST_TYPE_STRING_LIT: return type_intern_string_lit();
        case FRX_AST_TYPE_STRUCT_LIT: return symbol_infer_type(expr->struct_literal.path->path.symbol);

        default: FRX_ASSERT(FRX_FALSE); return NULL;
    }
}

AST* enum_def_lookup_variant(ASTEnumDef* enum_def, const char* name)
{
    FRX_ASSERT(enum_def != NULL);

    FRX_ASSERT(name != NULL);

    for (usize i = 0; i < list_size(&enum_def->variants); ++i)
    {
        AST* variant = list_get(&enum_def->variants, i);
        if (variant->enum_variant.name == name)
        {
            return variant;
        }
    }

    return NULL;
}
