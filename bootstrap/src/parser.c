#include "parser.h"

#include <string.h>

#include "ast.h"
#include "compiler.h"
#include "lexer.h"
#include "assert.h"
#include "log.h"
#include "scope.h"
#include "source_file.h"
#include "token.h"
#include "module.h"

void parser_init(Parser* parser, SourceFile* src_file)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(src_file != NULL);

    FRX_LOG_INFO("Initializing parser for file: %s...", src_file->path);

    parser->src_file = src_file;
    lexer_init(&parser->lexer, src_file);
    parser->global_scope = src_file->global_scope;
    parser->current_scope = parser->global_scope;
    parser->next_local_id = 0;
    parser->failed = FRX_FALSE;
    parser->recovery = FRX_FALSE;
}

AST* parser_parse(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    AST* ast = translation_unit_parse(parser);
    parser->src_file->ast = ast;

    return ast;
}

SourceSpan parser_current_span(Parser* parser)
{
    return parser_current_token(parser)->span;
}

const SourceFile* parser_source_file(const Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    return parser->src_file;
}

TokenType parser_current_type(Parser* parser)
{
    return parser_current_token(parser)->type;
}

Token* parser_current_token(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    return lexer_current_token(&parser->lexer);
}

Token* parser_peek(Parser* parser, usize offset)
{
    FRX_ASSERT(parser != NULL);

    return lexer_peek(&parser->lexer, offset);
}

b8 parser_match(Parser* parser, TokenType type)
{
    FRX_ASSERT(parser != NULL);

    TokenType current_type = parser_current_type(parser);

    return current_type == type || current_type == FRX_TOKEN_TYPE_EOF;
}

b8 parser_eat(Parser* parser, TokenType type)
{
    FRX_ASSERT(parser != NULL);
    FRX_ASSERT(type < FRX_TOKEN_TYPE_COUNT);

    if (parser_match(parser, type))
    {
        parser->recovery = FRX_FALSE;
        lexer_next_token(&parser->lexer);

        return FRX_FALSE;
    }

    if (!parser->recovery)
    {
        Diagnostic* d = diagnostic_create(FRX_DIAGNOSTIC_ID_UNEXPECTED_TOKEN,
                                          FRX_DIAGNOSTIC_LVL_ERROR,
                                          parser_current_span(parser),
                                          token_type_to_str(type),
                                          token_type_to_str(parser_current_type(parser)));
        compiler_add_diagnostic(d);
    }

    parser_recover(parser);

    return FRX_TRUE;
}

const char* parse_ident(Parser* parser)
{
    const char* ident = parser_current_token(parser)->identifier;

    return parser_eat(parser, FRX_TOKEN_TYPE_IDENT) ? NULL : ident;
}

u64 parse_int_literal(Parser* parser)
{
    u64 int_literal = parser_current_token(parser)->int_literal;

    return parser_eat(parser, FRX_TOKEN_TYPE_INT_LIT) ? 0 : int_literal;
}

const char* parse_char_literal(Parser* parser)
{
    const char* char_literal = parser_current_token(parser)->identifier;

    return parser_eat(parser, FRX_TOKEN_TYPE_CHAR_LIT) ? NULL : char_literal;
}

const char* parse_string_literal(Parser* parser)
{
    const char* string_literal = parser_current_token(parser)->identifier;

    return parser_eat(parser, FRX_TOKEN_TYPE_STR_LIT) ? NULL : string_literal;
}

SymbolVisibility parse_visibility(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    SymbolVisibility visibility = FRX_SYMBOL_VISIBILITY_PRIVATE;

    if (parser_match(parser, FRX_TOKEN_TYPE_KW_PUB))
    {
        visibility = FRX_SYMBOL_VISIBILITY_PUBLIC;
        parser_eat(parser, FRX_TOKEN_TYPE_KW_PUB);
    }
    else if (parser_match(parser, FRX_TOKEN_TYPE_KW_MOD))
    {
        visibility = FRX_SYMBOL_VISIBILITY_MODULE;
        parser_eat(parser, FRX_TOKEN_TYPE_KW_MOD);
    }

    return visibility;
}

void parser_recover(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    parser_fail(parser);
    parser->recovery = FRX_TRUE;

    lexer_next_token(&parser->lexer);

    while (!token_type_is_sync(parser_current_token(parser)->type))
    {
        lexer_next_token(&parser->lexer);
    }
}

Scope* parser_push_scope(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    Scope* scope = scope_create(parser->current_scope);
    parser->current_scope = scope;

    return parser->current_scope;
}

void parser_pop_scope(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(parser->current_scope != NULL);

    parser->current_scope = parser->current_scope->parent;
}

Symbol* parser_insert_symbol(Parser* parser, SymbolVisibility visibility,
                             SymbolType type, const char* name, void* data)
{
    FRX_ASSERT(parser != NULL);

    Symbol* symbol = scope_insert_symbol(parser->current_scope, visibility, type, name, data);

    if (visibility != FRX_SYMBOL_VISIBILITY_PRIVATE && symbol != NULL)
    {
        module_insert_symbol(parser->src_file->module, symbol);
    }

    return symbol;
}

Symbol* parser_lookup_symbol(Parser* parser, const char* name)
{
    FRX_ASSERT(parser != NULL);

    return scope_lookup_symbol(parser->current_scope, name);
}

ASTNodeID parser_next_node_id(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    ASTNodeID id = {
        .owner = parser->src_file->id,
        .local = parser->next_local_id++
    };

    return id;
}

AST* parser_create_ast(Parser* parser, ASTType type)
{
    FRX_ASSERT(parser != NULL);

    return ast_create(type, parser_next_node_id(parser));
}

void parser_fail(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    parser->failed = FRX_TRUE;
}

b8 parser_failed(const Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    return parser->failed || lexer_failed(&parser->lexer);
}

void parser_destroy(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    lexer_destroy(&parser->lexer);
}
