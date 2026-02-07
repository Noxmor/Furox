#ifndef FRX_CODEGEN_H
#define FRX_CODEGEN_H

#include "ast.h"
#include "module.h"

#include <stdio.h>

typedef struct CodegenContext
{
    FILE* source;
    FILE* header;
    const Module* root_mod;
    List* src_files;
    List symbol_list;
} CodegenContext;

u8 codegen_context_init(CodegenContext* ctx, const Module* root_mod,
                        List* src_files, const char* filename);

void codegen_context_transpile(CodegenContext* ctx);

void codegen_context_end(CodegenContext* ctx);

void codegen_mangle_module(FILE* f, const Module* mod);

void translation_unit_codegen(AST* ast, CodegenContext* ctx);

void func_param_codegen(AST* ast, CodegenContext* ctx);

void func_decl_codegen(AST* ast, CodegenContext* ctx);

void struct_def_codegen(AST* ast, CodegenContext* ctx);

void enum_def_codegen(AST* ast, CodegenContext* ctx);

void type_specifier_codegen(AST* ast, FILE* f);

void scope_codegen(AST* ast, CodegenContext* ctx);

void expr_stmt_codegen(AST* ast, CodegenContext* ctx);

void let_stmt_codegen(AST* ast, CodegenContext* ctx);

void if_stmt_codegen(AST* ast, CodegenContext* ctx);

void break_stmt_codegen(AST* ast, CodegenContext* ctx);

void continue_stmt_codegen(AST* ast, CodegenContext* ctx);

void return_stmt_codegen(AST* ast, CodegenContext* ctx);

void int_literal_codegen(AST* ast, CodegenContext* ctx);

void path_expr_codegen(AST* ast, CodegenContext* ctx);

void call_expr_codegen(AST* ast, CodegenContext* ctx);

void unary_expr_codegen(AST* ast, CodegenContext* ctx);

void binary_expr_codegen(AST* ast, CodegenContext* ctx);

#endif
