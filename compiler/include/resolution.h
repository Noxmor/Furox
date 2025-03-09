#ifndef FRX_RESOLUTION_H
#define FRX_RESOLUTION_H

#include "parser.h"

void translation_unit_resolve(Parser* parser, TranslationUnit* unit);

void use_stmt_resolve(Parser* parser, UseStmt* use_stmt);

void type_specifier_resolve(Parser* parser, TypeSpecifier* type);

void item_resolve(Parser* parser, Item* item);

void struct_def_resolve(Parser* parser, StructDef* struct_def);

void trait_resolve(Parser* parser, Trait* trait);

void impl_block_resolve(Parser* parser, ImplBlock* impl_block);

void func_params_resolve(Parser* parser, FuncParams* params);

void func_decl_resolve(Parser* parser, FuncDecl* func_decl);

void func_def_resolve(Parser* parser, FuncDef* func_def);

void scope_resolve(Parser* parser, Scope* scope);

void stmt_resolve(Parser* parser, Stmt* stmt);

void expr_stmt_resolve(Parser* parser, ExprStmt* expr_stmt);

void return_stmt_resolve(Parser* parser, ReturnStmt* return_stmt);

void let_stmt_resolve(Parser* parser, LetStmt* let_stmt);

void if_stmt_resolve(Parser* parser, IfStmt* if_stmt);

void expr_resolve(Parser* parser, Expr* expr);

void unary_expr_resolve(Parser* parser, UnaryExpr* unary_expr);

void binary_expr_resolve(Parser* parser, BinaryExpr* binary_expr);

void func_call_resolve(Parser* parser, FuncCall* func_call);

void var_resolve(Parser* parser, Var* var);

#endif
