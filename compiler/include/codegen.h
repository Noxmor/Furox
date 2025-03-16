#ifndef FRX_CODEGEN_H
#define FRX_CODEGEN_H

#include "ast.h"
#include "mir.h"

void translation_unit_lower_to_mir(TranslationUnit* unit, MIRContext* ctx);

void item_lower_to_mir(Item* item, MIRContext* ctx);

void func_def_lower_to_mir(FuncDef* func_def, MIRContext* ctx);

MIRBlock* scope_lower_to_mir(Scope* scope);

void stmt_lower_to_mir(Stmt* stmt, MIRBlock* block);

void return_stmt_lower_to_mir(ReturnStmt* return_stmt, MIRBlock* block);

#endif
