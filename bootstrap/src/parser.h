#ifndef FRX_PARSER_H
#define FRX_PARSER_H

#include "lexer.h"
#include "ast.h"

#include "core/arena.h"

#include "symbols/symbol_table.h"

typedef struct Parser
{
    Lexer lexer;
    ASTProgram* program;

    Namespace* current_namespace;

    SymbolTable symbol_table;

    Arena arena;
} Parser;

FRX_NO_DISCARD b8 parser_init(Parser* parser, const char* filepath);

FRX_NO_DISCARD b8 parser_parse(Parser* parser);

#endif
