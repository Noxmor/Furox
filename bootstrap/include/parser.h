#ifndef FRX_PARSER_H
#define FRX_PARSER_H

#include "lexer.h"
#include "ast.h"
#include "scope.h"
#include "source_file.h"
#include "symbol.h"

typedef struct Parser
{
    SourceFile* src_file;
    Lexer lexer;
    Scope* global_scope;
    Scope* current_scope;
    b8 external;
    b8 failed;
    b8 recovery;
} Parser;

void parser_init(Parser* parser, SourceFile* src_file);

AST* parser_parse(Parser* parser);

void parser_add_diagnostic(Parser* parser, Diagnostic* d);

SourceLocation parser_current_location(const Parser* parser);

const SourceFile* parser_source_file(const Parser* parser);

TokenType parser_current_type(Parser* parser);

Token* parser_current_token(Parser* parser);

Token* parser_peek(Parser* parser, usize offset);

b8 parser_match(Parser* parser, TokenType type);

b8 parser_eat(Parser* parser, TokenType type);

SymbolVisibility parse_visibility(Parser* parser);

void parser_recover(Parser* parser);

Scope* parser_push_scope(Parser* parser);

void parser_pop_scope(Parser* parser);

Symbol* parser_insert_symbol(Parser* parser, SymbolVisibility visibility,
                             SymbolType type, const char* name, void* data);

Symbol* parser_lookup_symbol(Parser* parser, const char* name);

void parser_fail(Parser* parser);

b8 parser_failed(const Parser* parser);

void parser_destroy(Parser* parser);

AST* translation_unit_parse(Parser* parser);

AST* mod_decl_parse(Parser* parser);

AST* item_parse(Parser* parser);

AST* use_tree_parse(Parser* parser);

AST* use_stmt_parse(Parser* parser);

AST* type_alias_parse(Parser* parser, SymbolVisibility visibility);

AST* type_specifier_parse(Parser* parser);

AST* struct_def_parse(Parser* parser, SymbolVisibility visibility);

AST* enum_def_parse(Parser* parser, SymbolVisibility visibility);

AST* trait_parse(Parser* parser, SymbolVisibility visibility);

AST* impl_block_parse(Parser* parser);

AST* func_param_parse(Parser* parser);

AST* generic_params_parse(Parser* parser);

AST* func_decl_parse(Parser* parser, SymbolVisibility visibility);

AST* block_parse(Parser* parser);

AST* stmt_parse(Parser* parser);

AST* expr_stmt_parse(Parser* parser);

AST* break_stmt_parse(Parser* parser);

AST* continue_stmt_parse(Parser* parser);

AST* return_stmt_parse(Parser* parser);

AST* let_stmt_parse(Parser* parser);

AST* if_stmt_parse(Parser* parser);

AST* for_loop_parse(Parser* parser);

AST* loop_parse(Parser* parser);

AST* expr_parse(Parser* parser);

AST* self_expr_parse(Parser* parser);

AST* bool_expr_parse(Parser* parser);

AST* path_expr_parse(Parser* parser, PathStyle style);

AST* call_expr_parse(Parser* parser, AST* callee);

AST* method_call_expr_parse(Parser* parser, const char* name, AST* callee);

AST* int_literal_parse(Parser* parser);

AST* char_literal_parse(Parser* parser);

AST* string_literal_parse(Parser* parser);

AST* struct_literal_parse(Parser* parser, AST* path_expr);

#endif
