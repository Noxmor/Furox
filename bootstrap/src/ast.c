#include "ast.h"
#include "assert.h"
#include "resolution.h"
#include "sema.h"
#include "compiler.h"
#include "symbol.h"
#include "type_system.h"

AST* ast_create(ASTType type)
{
    FRX_ASSERT(type < FRX_AST_TYPE_COUNT);

    AST* ast = compiler_alloc_ast(sizeof(AST));

    ast->type = type;

    return ast;
}

void ast_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    switch (ast->type)
    {
        case FRX_AST_TYPE_ERROR: break;
        case FRX_AST_TYPE_TRANSLATION_UNIT: translation_unit_resolve(ast, ctx); break;
        case FRX_AST_TYPE_USE_STMT: use_stmt_resolve(ast, ctx); break;
        case FRX_AST_TYPE_TYPE_SPECIFIER: type_specifier_resolve(ast, ctx); break;
        case FRX_AST_TYPE_TYPE_ALIAS: return type_alias_resolve(ast, ctx); break;
        case FRX_AST_TYPE_STRUCT_DEF: struct_def_resolve(ast, ctx); break;
        case FRX_AST_TYPE_ENUM_DEF: enum_def_resolve(ast, ctx); break;
        case FRX_AST_TYPE_TRAIT: trait_resolve(ast, ctx); break;
        case FRX_AST_TYPE_IMPL_BLOCK: impl_block_resolve(ast, ctx); break;
        case FRX_AST_TYPE_GENERIC_PARAMS: break;
        case FRX_AST_TYPE_FUNC_DECL: func_decl_resolve(ast, ctx); break;
        case FRX_AST_TYPE_BLOCK: block_resolve(ast, ctx); break;
        case FRX_AST_TYPE_EXPR_STMT: break;
        case FRX_AST_TYPE_BREAK_STMT: break;
        case FRX_AST_TYPE_CONTINUE_STMT: break;
        case FRX_AST_TYPE_RETURN_STMT: break;
        case FRX_AST_TYPE_LET_STMT: let_stmt_resolve(ast, ctx); break;
        case FRX_AST_TYPE_IF_STMT: if_stmt_resolve(ast, ctx); break;
        case FRX_AST_TYPE_UNARY_EXPR: break;
        case FRX_AST_TYPE_BINARY_EXPR: break;
        case FRX_AST_TYPE_FIELD_EXPR: break;
        case FRX_AST_TYPE_PATH_EXPR: path_expr_resolve(ast, ctx); break;
        case FRX_AST_TYPE_CALL_EXPR: call_expr_resolve(ast, ctx); break;
        case FRX_AST_TYPE_METHOD_CALL_EXPR: method_call_expr_resolve(ast, ctx); break;
        case FRX_AST_TYPE_INT_LIT: int_literal_resolve(ast, ctx); break;
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
        case FRX_AST_TYPE_TYPE_SPECIFIER: break;
        case FRX_AST_TYPE_TYPE_ALIAS: break;
        case FRX_AST_TYPE_STRUCT_DEF: break;
        case FRX_AST_TYPE_ENUM_DEF: enum_def_sema(ast, ctx); break;
        case FRX_AST_TYPE_TRAIT: trait_sema(ast, ctx); break;
        case FRX_AST_TYPE_IMPL_BLOCK: impl_block_sema(ast, ctx); break;
        case FRX_AST_TYPE_GENERIC_PARAMS: break;
        case FRX_AST_TYPE_FUNC_DECL: func_decl_sema(ast, ctx); break;
        case FRX_AST_TYPE_BLOCK: block_sema(ast, ctx); break;
        case FRX_AST_TYPE_EXPR_STMT: expr_stmt_sema(ast, ctx); break;
        case FRX_AST_TYPE_BREAK_STMT: break;
        case FRX_AST_TYPE_CONTINUE_STMT: break;
        case FRX_AST_TYPE_RETURN_STMT: return_stmt_sema(ast, ctx); break;
        case FRX_AST_TYPE_LET_STMT: let_stmt_sema(ast, ctx); break;
        case FRX_AST_TYPE_IF_STMT: if_stmt_sema(ast, ctx); break;
        case FRX_AST_TYPE_UNARY_EXPR: unary_expr_sema(ast, ctx); break;
        case FRX_AST_TYPE_BINARY_EXPR: binary_expr_sema(ast, ctx); break;
        case FRX_AST_TYPE_FIELD_EXPR: field_expr_sema(ast, ctx); break;
        case FRX_AST_TYPE_PATH_EXPR: break;
        case FRX_AST_TYPE_CALL_EXPR: call_expr_sema(ast, ctx); break;
        case FRX_AST_TYPE_METHOD_CALL_EXPR: method_call_expr_sema(ast, ctx); break;
        case FRX_AST_TYPE_INT_LIT: break;
        case FRX_AST_TYPE_CHAR_LIT: break;
        case FRX_AST_TYPE_STRING_LIT: break;
        default: FRX_ASSERT(FRX_FALSE); break;
    }
}


const Type* expr_infer_type(AST* expr)
{
    FRX_ASSERT(expr != NULL);

    switch (expr->type)
    {
        case FRX_AST_TYPE_UNARY_EXPR: return expr->unary_expr.resolved_type;
        case FRX_AST_TYPE_BINARY_EXPR: return expr->binary_expr.resolved_type;
        case FRX_AST_TYPE_FIELD_EXPR: return expr->field_expr.resolved_type;
        case FRX_AST_TYPE_PATH_EXPR:
        {
            switch (expr->path_expr.symbol->type)
            {
                case FRX_SYMBOL_TYPE_STRUCT: return type_intern_struct(expr->path_expr.symbol, &((AST*)list_get(&expr->path_expr.path_segments, list_size(&expr->path_expr.path_segments) - 1))->path_segment.generic_args);
                case FRX_SYMBOL_TYPE_GENERIC_PARAM: return type_intern_generic(expr->path_expr.symbol);
                default: break;
            }

            return symbol_infer_type(expr->path_expr.symbol);
        }
        case FRX_AST_TYPE_CALL_EXPR: return expr_infer_type(expr->call_expr.callee);
        case FRX_AST_TYPE_METHOD_CALL_EXPR: return expr->method_call_expr.resolved_type;
        case FRX_AST_TYPE_INT_LIT: return expr->int_literal.resolved_type;
        case FRX_AST_TYPE_CHAR_LIT: return type_intern_char_lit();
        case FRX_AST_TYPE_STRING_LIT: return type_intern_string_lit();

        default: FRX_ASSERT(FRX_FALSE); return NULL;
    }
}

ASTEnumConstant* enum_def_lookup_constant(ASTEnumDef* enum_def, const char* name)
{
    FRX_ASSERT(enum_def != NULL);

    FRX_ASSERT(name != NULL);

    for (usize i = 0; i < list_size(&enum_def->constants); ++i)
    {
        AST* constant = list_get(&enum_def->constants, i);
        if (constant->enum_constant.name == name)
        {
            return &constant->enum_constant;
        }
    }

    return NULL;
}
