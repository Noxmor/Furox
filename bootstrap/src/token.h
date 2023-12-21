#ifndef FRX_TOKEN_H
#define FRX_TOKEN_H

#include "core/types.h"

enum
{
    FRX_TOKEN_TYPE_EOF = 0,
    
    FRX_TOKEN_TYPE_IDENTIFIER,

    FRX_TOKEN_TYPE_NUMBER,

    FRX_TOKEN_TYPE_CHAR_LITERAL,
    FRX_TOKEN_TYPE_STRING_LITERAL,

    FRX_TOKEN_TYPE_PLUS,
    FRX_TOKEN_TYPE_MINUS,
    FRX_TOKEN_TYPE_STAR,
    FRX_TOKEN_TYPE_SLASH,

    FRX_TOKEN_TYPE_LEFT_PARANTHESIS,
    FRX_TOKEN_TYPE_RIGHT_PARANTHESIS,

    FRX_TOKEN_TYPE_LEFT_BRACKET,
    FRX_TOKEN_TYPE_RIGHT_BRACKET,

    FRX_TOKEN_TYPE_LEFT_BRACE,
    FRX_TOKEN_TYPE_RIGHT_BRACE,

    FRX_TOKEN_TYPE_COMMA,
    FRX_TOKEN_TYPE_COLON,
    FRX_TOKEN_TYPE_SEMICOLON,

    FRX_TOKEN_TYPE_COUNT
};

typedef u8 TokenType;

#define FRX_TOKEN_IDENTIFIER_CAPACITY 128

typedef struct Token
{
    TokenType type;

    usize line;
    usize coloumn;

    char identifier[FRX_TOKEN_IDENTIFIER_CAPACITY];
    usize number;
} Token;

const char* token_type_to_str(TokenType type);

#endif
