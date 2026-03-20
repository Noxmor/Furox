#ifndef FRX_SEMA_H
#define FRX_SEMA_H

#include "ast.h"
#include "source_file.h"

typedef struct SemaContext
{
    const SourceFile* src_file;
    ASTImplBlock* current_impl_block;
    b8 failed;
} SemaContext;

void sema_context_init(SemaContext* ctx, const SourceFile* src_file);

void sema_context_fail(SemaContext* ctx);

b8 sema_context_failed(const SemaContext* ctx);

void ast_sema(AST* ast, SemaContext* ctx);

void translation_unit_sema(AST* ast, SemaContext* ctx);

void enum_def_sema(AST* ast, SemaContext* ctx);

void trait_sema(AST* ast, SemaContext* ctx);

void impl_block_sema(AST* ast, SemaContext* ctx);

void func_decl_sema(AST* ast, SemaContext* ctx);

void block_sema(AST* ast, SemaContext* ctx);

void expr_stmt_sema(AST* ast, SemaContext* ctx);

void return_stmt_sema(AST* ast, SemaContext* ctx);

void let_stmt_sema(AST* ast, SemaContext* ctx);

void if_stmt_sema(AST* ast, SemaContext* ctx);

void for_loop_sema(AST* ast, SemaContext* ctx);

void while_loop_sema(AST* ast, SemaContext* ctx);

void loop_sema(AST* ast, SemaContext* ctx);

void unary_expr_sema(AST* ast, SemaContext* ctx);

void binary_expr_sema(AST* ast, SemaContext* ctx);

void field_expr_sema(AST* ast, SemaContext* ctx);

void call_expr_sema(AST* ast, SemaContext* ctx);

void method_call_expr_sema(AST* ast, SemaContext* ctx);

void struct_literal_sema(AST* ast, SemaContext* ctx);

#endif
