#ifndef FRX_TOKEN_H
#define FRX_TOKEN_H

#include "core/types.h"

enum
{
    FRX_TOKEN_TYPE_EOF = 0,

    FRX_TOKEN_TYPE_KW_U8,
    FRX_TOKEN_TYPE_KW_U16,
    FRX_TOKEN_TYPE_KW_U32,
    FRX_TOKEN_TYPE_KW_U64,
    FRX_TOKEN_TYPE_KW_USIZE,
    FRX_TOKEN_TYPE_KW_I8,
    FRX_TOKEN_TYPE_KW_I16,
    FRX_TOKEN_TYPE_KW_I32,
    FRX_TOKEN_TYPE_KW_I64,
    FRX_TOKEN_TYPE_KW_ISIZE,
    FRX_TOKEN_TYPE_KW_B8,
    FRX_TOKEN_TYPE_KW_B16,
    FRX_TOKEN_TYPE_KW_B32,
    FRX_TOKEN_TYPE_KW_B64,
    FRX_TOKEN_TYPE_KW_CHAR,
    FRX_TOKEN_TYPE_KW_F32,
    FRX_TOKEN_TYPE_KW_F64,
    FRX_TOKEN_TYPE_KW_VOID,

    FRX_TOKEN_TYPE_KW_NULLPTR,
    FRX_TOKEN_TYPE_KW_TRUE,
    FRX_TOKEN_TYPE_KW_FALSE,
    FRX_TOKEN_TYPE_KW_IMPORT,
    FRX_TOKEN_TYPE_KW_RETURN,
    FRX_TOKEN_TYPE_KW_NAMESPACE,
    FRX_TOKEN_TYPE_KW_EXTERN,
    FRX_TOKEN_TYPE_KW_ENUM,
    FRX_TOKEN_TYPE_KW_STRUCT,
    FRX_TOKEN_TYPE_KW_EXPORT,
    FRX_TOKEN_TYPE_KW_IF,
    FRX_TOKEN_TYPE_KW_ELSE,
    FRX_TOKEN_TYPE_KW_SWITCH,
    FRX_TOKEN_TYPE_KW_CASE,
    FRX_TOKEN_TYPE_KW_DEFAULT,
    FRX_TOKEN_TYPE_KW_BREAK,
    FRX_TOKEN_TYPE_KW_FOR,
    FRX_TOKEN_TYPE_KW_WHILE,
    FRX_TOKEN_TYPE_KW_DO,
    FRX_TOKEN_TYPE_KW_MACRO,
    FRX_TOKEN_TYPE_KW_SIZEOF,

    FRX_TOKEN_TYPE_IDENTIFIER,

    FRX_TOKEN_TYPE_NUMBER,

    FRX_TOKEN_TYPE_CHAR_LITERAL,
    FRX_TOKEN_TYPE_STRING_LITERAL,

    FRX_TOKEN_TYPE_PLUS,
    FRX_TOKEN_TYPE_MINUS,
    FRX_TOKEN_TYPE_STAR,
    FRX_TOKEN_TYPE_SLASH,
    FRX_TOKEN_TYPE_MODULO,

    FRX_TOKEN_TYPE_LOGICAL_AND,
    FRX_TOKEN_TYPE_LOGICAL_OR,
    FRX_TOKEN_TYPE_LOGICAL_NEGATION,

    FRX_TOKEN_TYPE_BINARY_AND,
    FRX_TOKEN_TYPE_BINARY_OR,
    FRX_TOKEN_TYPE_BINARY_XOR,
    FRX_TOKEN_TYPE_BINARY_NEGATION,
    FRX_TOKEN_TYPE_BINARY_LEFT_SHIFT,
    FRX_TOKEN_TYPE_BINARY_RIGHT_SHIFT,

    FRX_TOKEN_TYPE_EQUALS,
    FRX_TOKEN_TYPE_COMPARISON,

    FRX_TOKEN_TYPE_GREATER_THAN,
    FRX_TOKEN_TYPE_GREATER_THAN_EQUALS,

    FRX_TOKEN_TYPE_LESS_THAN,
    FRX_TOKEN_TYPE_LESS_THAN_EQUALS,

    FRX_TOKEN_TYPE_LEFT_PARANTHESIS,
    FRX_TOKEN_TYPE_RIGHT_PARANTHESIS,

    FRX_TOKEN_TYPE_LEFT_BRACKET,
    FRX_TOKEN_TYPE_RIGHT_BRACKET,

    FRX_TOKEN_TYPE_LEFT_BRACE,
    FRX_TOKEN_TYPE_RIGHT_BRACE,

    FRX_TOKEN_TYPE_ARROW,

    FRX_TOKEN_TYPE_COMMA,
    FRX_TOKEN_TYPE_DOT,
    FRX_TOKEN_TYPE_COLON,
    FRX_TOKEN_TYPE_SEMICOLON,

    FRX_TOKEN_TYPE_ELLIPSIS,

    FRX_TOKEN_TYPE_NAMESPACE_RESOLUTION,

    FRX_TOKEN_TYPE_COUNT
};

typedef u8 TokenType;

typedef struct SourceLocation
{
    usize pos;
    usize line;
    usize coloumn;
} SourceLocation;

#define FRX_TOKEN_IDENTIFIER_CAPACITY 128

typedef struct Token
{
    TokenType type;

    SourceLocation location;

    char identifier[FRX_TOKEN_IDENTIFIER_CAPACITY];
    usize number;
} Token;

const char* token_type_to_str(TokenType type);

#endif
