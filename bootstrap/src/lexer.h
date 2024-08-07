#ifndef FRX_LEXER_H
#define FRX_LEXER_H

#include <stdio.h>

#include "core/core.h"
#include "core/types.h"

#include "token.h"

void lexer_init_keyword_table(void);

#define FRX_LEXER_BUFFER_CAPACITY 1024
#define FRX_LEXER_TOKEN_CAPACITY 8

typedef struct Lexer
{
    FILE* file;
    const char* filepath;

    SourceLocation location;

    char buffer[FRX_LEXER_BUFFER_CAPACITY];
    usize buffer_index;

    Token tokens[FRX_LEXER_TOKEN_CAPACITY];
    usize tokens_count;
} Lexer;

FRX_NO_DISCARD b8 lexer_init(Lexer* lexer, const char* filepath);

Token* lexer_peek(Lexer* lexer, usize offset);

FRX_NO_DISCARD b8 lexer_next_token(Lexer* lexer);

FRX_NO_DISCARD b8 lexer_recover(Lexer* lexer, SourceLocation* location);

void lexer_destroy(Lexer* lexer);

#endif
