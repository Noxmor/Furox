#ifndef FRX_RESOLUTION_H
#define FRX_RESOLUTION_H

#include "source_file.h"
#include "symbol_table.h"
#include "hir.h"

typedef struct ResolutionContext
{
    SourceFile* src_file;
    b8 failed;
} ResolutionContext;

void resolution_context_init(ResolutionContext* ctx, SourceFile* src_file);

Symbol* resolution_context_lookup_symbol(ResolutionContext* ctx, SymbolType type,
                                         const char* name);

void resolution_context_fail(ResolutionContext* ctx);

b8 resolution_context_failed(const ResolutionContext* ctx);

void ast_resolve(AST* ast, ResolutionContext* ctx);

void translation_unit_resolve(AST* ast, ResolutionContext* ctx);

void use_stmt_resolve(AST* ast, ResolutionContext* ctx);

Type* type_specifier_resolve(AST* ast, ResolutionContext* ctx);

void struct_def_resolve(AST* ast, ResolutionContext* ctx);

void enum_def_resolve(AST* ast, ResolutionContext* ctx);

void trait_resolve(AST* ast, ResolutionContext* ctx);

void impl_block_resolve(AST* ast, ResolutionContext* ctx);

FuncParams* func_params_resolve(AST* ast, ResolutionContext* ctx);

void func_decl_resolve(AST* ast, ResolutionContext* ctx);

void func_def_resolve(AST* ast, ResolutionContext* ctx);

void scope_resolve(AST* ast, ResolutionContext* ctx);

void expr_stmt_resolve(AST* ast, ResolutionContext* ctx);

void return_stmt_resolve(AST* ast, ResolutionContext* ctx);

void let_stmt_resolve(AST* ast, ResolutionContext* ctx);

void if_stmt_resolve(AST* ast, ResolutionContext* ctx);

void unary_expr_resolve(AST* ast, ResolutionContext* ctx);

void binary_expr_resolve(AST* ast, ResolutionContext* ctx);

void field_expr_resolve(AST* ast, ResolutionContext* ctx);

void path_expr_resolve(AST* ast, ResolutionContext* ctx);

void call_expr_resolve(AST* ast, ResolutionContext* ctx);

#endif
