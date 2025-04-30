#ifndef FRX_SEMA_H
#define FRX_SEMA_H

#include "ast.h"

typedef struct SemaContext
{

} SemaContext;

void ast_sema(AST* ast, SemaContext* ctx);

void translation_unit_sema(AST* ast, SemaContext* ctx);

void type_specifier_sema(AST* ast, SemaContext* ctx);

void struct_def_sema(AST* ast, SemaContext* ctx);

void enum_def_sema(AST* ast, SemaContext* ctx);

void trait_sema(AST* ast, SemaContext* ctx);

void impl_block_sema(AST* ast, SemaContext* ctx);

void func_params_sema(AST* ast, SemaContext* ctx);

void func_decl_sema(AST* ast, SemaContext* ctx);

void func_def_sema(AST* ast, SemaContext* ctx);

void scope_sema(AST* ast, SemaContext* ctx);

void expr_stmt_sema(AST* ast, SemaContext* ctx);

void return_stmt_sema(AST* ast, SemaContext* ctx);

void let_stmt_sema(AST* ast, SemaContext* ctx);

void if_stmt_sema(AST* ast, SemaContext* ctx);

void unary_expr_sema(AST* ast, SemaContext* ctx);

void binary_expr_sema(AST* ast, SemaContext* ctx);

void call_expr_sema(AST* ast, SemaContext* ctx);

TypeSpecifier* expr_infer_type(AST* expr);

#endif
