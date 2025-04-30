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
    AST* translation_unit;
    SymbolTable symbol_table;
    SymbolTable* current_symbol_table;
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

Symbol* parser_lookup_symbol(Parser* parser, SymbolType type, const char* name);

Module* parser_find_module_by_path_segments(Parser* parser, const List* path_segments);

void parser_fail(Parser* parser);

b8 parser_failed(const Parser* parser);

void parser_destroy(Parser* parser);

AST* translation_unit_parse(Parser* parser);

AST* item_parse(Parser* parser);

AST* use_stmt_parse(Parser* parser);

AST* type_specifier_parse(Parser* parser);

AST* struct_def_parse(Parser* parser);

AST* enum_def_parse(Parser* parser);

AST* trait_parse(Parser* parser);

AST* impl_block_parse(Parser* parser);

AST* func_params_parse(Parser* parser);

AST* generic_params_parse(Parser* parser);

AST* func_decl_parse(Parser* parser);

AST* func_def_parse(Parser* parser);

AST* scope_parse(Parser* parser);

AST* stmt_parse(Parser* parser);

AST* expr_stmt_parse(Parser* parser);

AST* break_stmt_parse(Parser* parser);

AST* continue_stmt_parse(Parser* parser);

AST* return_stmt_parse(Parser* parser);

AST* let_stmt_parse(Parser* parser);

AST* if_stmt_parse(Parser* parser);

AST* expr_parse(Parser* parser);

AST* path_expr_parse(Parser* parser);

AST* call_expr_parse(Parser* parser);

AST* int_literal_parse(Parser* parser);

#endif
