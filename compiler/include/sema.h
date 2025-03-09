#ifndef FRX_SEMA_H
#define FRX_SEMA_H

#include "ast.h"

void translation_unit_sema(TranslationUnit* unit);

void type_specifier_sema(TypeSpecifier* type);

void item_sema(Item* item);

void struct_def_sema(StructDef* struct_def);

void trait_sema(Trait* trait);

void impl_block_sema(ImplBlock* impl_block);

void func_params_sema(FuncParams* params);

void func_decl_sema(FuncDecl* func_decl);

void func_def_sema(FuncDef* func_def);

void scope_sema(Scope* scope);

void stmt_sema(Stmt* stmt);

void expr_stmt_sema(ExprStmt* expr_stmt);

void break_stmt_sema(BreakStmt* break_stmt);

void continue_stmt_sema(ContinueStmt* continue_stmt);

void return_stmt_sema(ReturnStmt* return_stmt);

void let_stmt_sema(LetStmt* let_stmt);

void if_stmt_sema(IfStmt* if_stmt);

void expr_sema(Expr* expr);

TypeSpecifier* expr_infer_type(Expr* expr);

void unary_expr_sema(UnaryExpr* unary_expr);

void binary_expr_sema(BinaryExpr* binary_expr);

void func_call_sema(FuncCall* func_call);

void var_sema(Var* var);

void int_literal_sema(IntLiteral* literal);

#endif
