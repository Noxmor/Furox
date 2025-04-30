#include "parser.h"

#include <string.h>

#include "compiler.h"
#include "diagnostics.h"
#include "lexer.h"
#include "assert.h"
#include "log.h"
#include "symbol_table.h"
#include "token.h"
#include "module.h"

Parser* parser_create(Module* module, const char* filepath)
{
    FRX_ASSERT(filepath != NULL);

    FRX_LOG_INFO("Creating parser for file: %s...", filepath);

    Parser* parser = compiler_alloc(sizeof(Parser));

    parser->module = module;
    lexer_init(&parser->lexer, filepath);
    list_init(&parser->diagnostics);
    symbol_table_init(&parser->symbol_table, &parser->module->symbol_table);
    list_init(&parser->use_stmts);
    parser->failed = FRX_FALSE;
    parser->recovery = FRX_FALSE;

    return parser;
}

void parser_parse(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    parser->translation_unit = translation_unit_parse(parser);
}

void parser_emit_diagnostics(const Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    for (usize i = 0; i < list_size(&parser->diagnostics); ++i)
    {
        Diagnostic* d = list_get(&parser->diagnostics, i);
        diagnostic_emit(d);
    }
}

SourceLocation parser_current_location(const Parser* parser)
{
    return parser->lexer.location;
}

const char* parser_source_file(const Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    return lexer_source_file(&parser->lexer);
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
        FRX_PARSER_ADD_DIAGNOSTIC(parser, FRX_DIAGNOSTIC_ID_UNEXPECTED_TOKEN,
                                  FRX_DIAGNOSTIC_LVL_ERROR,
                                  parser_current_token(parser)->range,
                                  token_type_to_str(type),
                                  token_type_to_str(parser_current_type(parser)));
    }

    parser_recover(parser);

    return FRX_TRUE;
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

void parser_insert_symbol(Parser* parser, SymbolVisibility visibility,
                          SymbolType type, const char* name, void* data)
{
    FRX_ASSERT(parser != NULL);

    if (visibility == FRX_SYMBOL_VISIBILITY_PRIVATE)
    {
        symbol_table_insert(&parser->symbol_table, visibility, type, name, data);
    }
    else
    {
        module_insert_symbol(parser->module, visibility, type, name, data);
    }
}

Symbol* parser_lookup_symbol(Parser* parser, SymbolType type, const char* name)
{
    FRX_ASSERT(parser != NULL);

    Symbol* symbol = symbol_table_lookup(&parser->symbol_table, type, name);
    if (symbol == NULL)
    {
        symbol = module_lookup_symbol(parser->module, type, name);
    }

    return symbol;
}

Module* parser_find_module_by_path_segments(Parser* parser, const List* path_segments)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(path_segments != NULL);

    if (list_empty(path_segments))
    {
        return parser->module;
    }

    Module* initial_mod = parser->module;
    Module* mod = initial_mod;

    for (usize i = 0; i < list_size(path_segments); ++i)
    {
        Module* submodule = module_find_submodule_by_name(mod, list_get(path_segments, i));
        if (submodule != NULL)
        {
            mod = submodule;
            if (i == list_size(path_segments) - 1)
            {
                return mod;
            }
        }
    }

    mod = initial_mod;

    while (mod != NULL)
    {
        if (strcmp(mod->name, list_get(path_segments, 0)) == 0)
        {
            for (usize i = 1; i < list_size(path_segments); ++i)
            {
                Module* submodule = module_find_submodule_by_name(mod, list_get(path_segments, i));
                if (submodule != NULL)
                {
                    mod = submodule;
                    if (i == list_size(path_segments) - 1)
                    {
                        return mod;
                    }
                }
            }

            initial_mod = initial_mod->parent;
            mod = initial_mod;
        }
        else
        {
            initial_mod = initial_mod->parent;
            mod = initial_mod;
        }
    }

    return NULL;
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
