#ifndef FRX_PARSER_H
#define FRX_PARSER_H

#include "lexer.h"
#include "ast.h"

#include "core/arena.h"

typedef struct Parser
{
    Lexer lexer;
    ASTProgram* program;

    Arena arena;
} Parser;

FRX_NO_DISCARD b8 parser_init(Parser* parser, const char* filepath);

FRX_NO_DISCARD b8 parser_parse(Parser* parser);

#endif
