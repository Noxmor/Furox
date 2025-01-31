#ifndef FRX_LEXER_H
#define FRX_LEXER_H

#include <stdio.h>

#include "token.h"

#ifndef FRX_LEXER_BUFFER_CAPACITY
#define FRX_LEXER_BUFFER_CAPACITY 1024
#endif
#ifndef FRX_LEXER_TOKEN_CAPACITY
#define FRX_LEXER_TOKEN_CAPACITY 8
#endif

void lexer_init_keyword_table(void);

typedef struct Lexer
{
    FILE* file;
    char* filepath;

    SourceLocation location;

    char buffer[FRX_LEXER_BUFFER_CAPACITY];
    usize buffer_index;

    Token tokens[FRX_LEXER_TOKEN_CAPACITY];
    usize tokens_count;

    char* identifier_placeholder;
    usize identifier_placeholder_size;

    b8 failed;
} Lexer;

void lexer_init(Lexer* lexer, const char* filepath);

void lexer_read(Lexer* lexer);

void lexer_fail(Lexer* lexer);

b8 lexer_failed(const Lexer* lexer);

Token* lexer_peek(Lexer* lexer, usize offset);

Token* lexer_current_token(Lexer* lexer);

void lexer_next_token(Lexer* lexer);

const char* lexer_source_file(const Lexer* lexer);

void lexer_destroy(Lexer* lexer);

#endif
