#include "ast.h"
#include "assert.h"
#include "resolution.h"
#include "sema.h"
#include "compiler.h"

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
        case FRX_AST_TYPE_STRUCT_DEF: struct_def_resolve(ast, ctx); break;
        case FRX_AST_TYPE_ENUM_DEF: enum_def_resolve(ast, ctx); break;
        case FRX_AST_TYPE_TRAIT: trait_resolve(ast, ctx); break;
        case FRX_AST_TYPE_IMPL_BLOCK: impl_block_resolve(ast, ctx); break;
        case FRX_AST_TYPE_GENERIC_PARAMS: break;
        case FRX_AST_TYPE_FUNC_DECL: func_decl_resolve(ast, ctx); break;
        case FRX_AST_TYPE_SCOPE: scope_resolve(ast, ctx); break;
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
        case FRX_AST_TYPE_STRUCT_DEF: break;
        case FRX_AST_TYPE_ENUM_DEF: enum_def_sema(ast, ctx); break;
        case FRX_AST_TYPE_TRAIT: trait_sema(ast, ctx); break;
        case FRX_AST_TYPE_IMPL_BLOCK: impl_block_sema(ast, ctx); break;
        case FRX_AST_TYPE_GENERIC_PARAMS: break;
        case FRX_AST_TYPE_FUNC_DECL: func_decl_sema(ast, ctx); break;
        case FRX_AST_TYPE_SCOPE: scope_sema(ast, ctx); break;
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
        default: FRX_ASSERT(FRX_FALSE); break;
    }
}

Type* expr_infer_type(AST* expr)
{
    FRX_ASSERT(expr != NULL);

    switch (expr->type)
    {
        case FRX_AST_TYPE_UNARY_EXPR: return expr->unary_expr.resolved_type;
        case FRX_AST_TYPE_BINARY_EXPR: return expr->binary_expr.resolved_type;
        case FRX_AST_TYPE_FIELD_EXPR: return expr->field_expr.resolved_type;
        case FRX_AST_TYPE_PATH_EXPR: return symbol_infer_type(expr->path_expr.symbol);
        case FRX_AST_TYPE_CALL_EXPR: return expr_infer_type(expr->call_expr.callee);
        case FRX_AST_TYPE_METHOD_CALL_EXPR: return expr->method_call_expr.resolved_type;
        case FRX_AST_TYPE_INT_LIT: return expr->int_literal.resolved_type;

        default: FRX_ASSERT(FRX_FALSE); return NULL;
    }
}
