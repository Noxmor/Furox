#ifndef FRX_RESOLUTION_H
#define FRX_RESOLUTION_H

#include "source_file.h"

typedef struct ResolutionContext
{
    SourceFile* src_file;
    Module* root_mod;
    Module* current_mod;
    Scope* current_scope;
    AST* current_impl_block;
    AST* current_func_decl;
    b8 failed;
} ResolutionContext;

void resolution_context_init(ResolutionContext* ctx, SourceFile* src_file,
                             Module* root_mod);

void resolution_context_push_scope(ResolutionContext* ctx, Scope* scope);

void resolution_context_pop_scope(ResolutionContext* ctx);

Symbol* resolution_context_lookup_symbol(ResolutionContext* ctx, const char* name);

void resolution_context_fail(ResolutionContext* ctx);

b8 resolution_context_failed(const ResolutionContext* ctx);

void ast_resolve(AST* ast, ResolutionContext* ctx);

void type_specifier_resolve(AST* ast, ResolutionContext* ctx);

void block_resolve(AST* ast, ResolutionContext* ctx);

void expr_stmt_resolve(AST* ast, ResolutionContext* ctx);

void return_stmt_resolve(AST* ast, ResolutionContext* ctx);

void let_stmt_resolve(AST* ast, ResolutionContext* ctx);

void if_stmt_resolve(AST* ast, ResolutionContext* ctx);

void for_loop_resolve(AST* ast, ResolutionContext* ctx);

void while_loop_resolve(AST* ast, ResolutionContext* ctx);

void loop_resolve(AST* ast, ResolutionContext* ctx);

void unary_expr_resolve(AST* ast, ResolutionContext* ctx);

void binary_expr_resolve(AST* ast, ResolutionContext* ctx);

void field_expr_resolve(AST* ast, ResolutionContext* ctx);

void self_expr_resolve(AST* ast, ResolutionContext* ctx);

void path_expr_resolve(AST* ast, ResolutionContext* ctx);

void call_expr_resolve(AST* ast, ResolutionContext* ctx);

void method_call_expr_resolve(AST* ast, ResolutionContext* ctx);

void int_literal_resolve(AST* ast, ResolutionContext* ctx);

void struct_literal_resolve(AST* ast, ResolutionContext* ctx);

#endif
