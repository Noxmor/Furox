#include "parser.h"

#include "compiler.h"
#include "diagnostics.h"
#include "lexer.h"
#include "assert.h"
#include "log.h"
#include "token.h"

Parser* parser_create(const char* filepath)
{
    FRX_ASSERT(filepath != NULL);

    FRX_LOG_INFO("Creating parser for file: %s...", filepath);

    Parser* parser = compiler_alloc(sizeof(Parser));

    lexer_init(&parser->lexer, filepath);
    list_init(&parser->diagnostics);
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

    return parser_current_token(parser)->type == type;
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

    parser->failed = FRX_TRUE;
    parser->recovery = FRX_TRUE;

    lexer_next_token(&parser->lexer);

    while (!token_type_is_sync(parser_current_token(parser)->type))
    {
        lexer_next_token(&parser->lexer);
    }
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
