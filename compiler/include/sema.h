#ifndef FRX_SEMA_H
#define FRX_SEMA_H

#include "ast.h"

void translation_unit_sema(TranslationUnit* unit);

void type_specifier_sema(TypeSpecifier* type);

void item_sema(Item* item);

void func_def_sema(FuncDef* func_def);

void scope_sema(Scope* scope);

void stmt_sema(Stmt* stmt);

void expr_stmt_sema(ExprStmt* expr_stmt);

void break_stmt_sema(BreakStmt* break_stmt);

void continue_stmt_sema(ContinueStmt* continue_stmt);

void return_stmt_sema(ReturnStmt* return_stmt);

void expr_sema(Expr* expr);

void int_literal_sema(IntLiteral* literal);

#endif
