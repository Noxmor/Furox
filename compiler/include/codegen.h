#ifndef FRX_CODEGEN_H
#define FRX_CODEGEN_H

#include "ast.h"

b8 codegen_begin(const char* output);

void codegen_write(const char* format, ...);

void codegen_end(void);

void translation_unit_codegen(TranslationUnit* unit);

void type_specifier_codegen(TypeSpecifier* type);

void item_codegen(Item* item);

void struct_def_codegen(StructDef* struct_def);

void func_params_codegen(FuncParams* params);

void func_def_codegen(FuncDef* func_def);

void scope_codegen(Scope* scope);

void stmt_codegen(Stmt* stmt);

void expr_stmt_codegen(ExprStmt* expr_stmt);

void break_stmt_codegen(BreakStmt* break_stmt);

void continue_stmt_codegen(ContinueStmt* continue_stmt);

void return_stmt_codegen(ReturnStmt* return_stmt);

void let_stmt_codegen(LetStmt* let_stmt);

void if_stmt_codegen(IfStmt* if_stmt);

void expr_codegen(Expr* expr);

void unary_expr_codegen(UnaryExpr* unary_expr);

void binary_expr_codegen(BinaryExpr* binary_expr);

void func_call_codegen(FuncCall* func_call);

void var_codegen(Var* var);

void int_literal_codegen(IntLiteral* literal);

#endif
