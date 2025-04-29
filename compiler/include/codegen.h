#ifndef FRX_CODEGEN_H
#define FRX_CODEGEN_H

#include "ast.h"

#include <stdio.h>

typedef struct CodegenContext
{
    FILE* source;
    FILE* header;
} CodegenContext;

u8 codegen_context_begin(CodegenContext* ctx, const char* name);

void codegen_context_end(CodegenContext* ctx);

void translation_unit_codegen(TranslationUnit* unit, CodegenContext* ctx);

void item_codegen(Item* item, CodegenContext* ctx);

void func_params_codegen(FuncParams* params, FILE* f);

void func_decl_codegen(FuncDecl* func_decl, CodegenContext* ctx);

void func_def_codegen(FuncDef* func_def, CodegenContext* ctx);

void struct_def_codegen(StructDef* struct_def, CodegenContext* ctx);

void type_specifier_codegen(TypeSpecifier* type, FILE* f);

void scope_codegen(Scope* scope, CodegenContext* ctx);

void stmt_codegen(Stmt* stmt, CodegenContext* ctx);

void let_stmt_codegen(LetStmt* let_stmt, CodegenContext* ctx);

void return_stmt_codegen(ReturnStmt* return_stmt, CodegenContext* ctx);

void expr_codegen(Expr* expr, CodegenContext* ctx);

void int_literal_codegen(IntLiteral* literal, CodegenContext* ctx);

void path_expr_codegen(PathExpr* path_expr, CodegenContext* ctx);

void call_expr_codegen(CallExpr* call_expr, CodegenContext* ctx);

void unary_expr_codegen(UnaryExpr* unary_expr, CodegenContext* ctx);

void binary_expr_codegen(BinaryExpr* binary_expr, CodegenContext* ctx);

#endif
