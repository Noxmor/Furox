#ifndef FRX_PARSER_H
#define FRX_PARSER_H

#include "lexer.h"
#include "ast.h"
#include "diagnostics.h"
#include "symbol_table.h"

#define FRX_PARSER_ADD_DIAGNOSTIC(parser, id, lvl, range, ...) do { FRX_ASSERT(parser != NULL);\
    Diagnostic* d = diagnostic_create(id, lvl, parser_source_file(parser), range, ##__VA_ARGS__);\
    list_add(&parser->diagnostics, d);\
} while (0)

typedef struct Module Module;

typedef struct Parser
{
    Module* module;
    Lexer lexer;
    List diagnostics;
    TranslationUnit* translation_unit;
    SymbolTable symbol_table;
    List use_stmts;
    SymbolVisibility visibility;
    b8 external;
    b8 failed;
    b8 recovery;
} Parser;

Parser* parser_create(Module* module, const char* filepath);

void parser_parse(Parser* parser);

void parser_emit_diagnostics(const Parser* parser);

SourceLocation parser_current_location(const Parser* parser);

const char* parser_source_file(const Parser* parser);

TokenType parser_current_type(Parser* parser);

Token* parser_current_token(Parser* parser);

Token* parser_peek(Parser* parser, usize offset);

b8 parser_match(Parser* parser, TokenType type);

b8 parser_eat(Parser* parser, TokenType type);

void parser_recover(Parser* parser);

void parser_insert_symbol(Parser* parser, SymbolVisibility visibility,
                          SymbolType type, const char* name, void* data);

void* parser_lookup_symbol(Parser* parser, SymbolType type, const char* name);

Module* parser_find_module_by_path_segments(Parser* parser, const List* path_segments);

void parser_fail(Parser* parser);

b8 parser_failed(const Parser* parser);

void parser_destroy(Parser* parser);

TranslationUnit* translation_unit_parse(Parser* parser);

UseStmt* use_stmt_parse(Parser* parser);

TypeSpecifier* type_specifier_parse(Parser* parser);

Item* item_parse(Parser* parser);

StructDef* struct_def_parse(Parser* parser);

Trait* trait_parse(Parser* parser);

FuncParams* func_params_parse(Parser* parser);

FuncDecl* func_decl_parse(Parser* parser);

FuncDef* func_def_parse(Parser* parser);

Scope* scope_parse(Parser* parser);

Stmt* stmt_parse(Parser* parser);

ExprStmt* expr_stmt_parse(Parser* parser);

BreakStmt* break_stmt_parse(Parser* parser);

ContinueStmt* continue_stmt_parse(Parser* parser);

ReturnStmt* return_stmt_parse(Parser* parser);

LetStmt* let_stmt_parse(Parser* parser);

IfStmt* if_stmt_parse(Parser* parser);

Expr* expr_parse(Parser* parser);

FuncCall* func_call_parse(Parser* parser);

Var* var_parse(Parser* parser);

IntLiteral* int_literal_parse(Parser* parser);

#endif
