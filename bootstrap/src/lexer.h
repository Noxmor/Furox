#ifndef FRX_LEXER_H
#define FRX_LEXER_H

#include <stdio.h>

#include "core/types.h"

#include "token.h"

#define FRX_LEXER_BUFFER_CAPACITY 1024
#define FRX_LEXER_TOKEN_CAPACITY 8

typedef struct Lexer
{
    FILE* file;

    usize line;
    usize coloumn;

    char buffer[FRX_LEXER_BUFFER_CAPACITY];
    usize buffer_index;

    Token tokens[FRX_LEXER_TOKEN_CAPACITY];
    usize tokens_count;
} Lexer;

void lexer_init(Lexer* lexer, const char* filepath);

Token* lexer_peek(Lexer* lexer, usize offset);

void lexer_next_token(Lexer* lexer);

void lexer_destroy(Lexer* lexer);

#endif
