#include "token.h"

#include "assert.h"

static char* token_type_names[] = {
    [FRX_TOKEN_TYPE_EOF]  = "eof",

    [FRX_TOKEN_TYPE_IDENT] = "identifier",
    [FRX_TOKEN_TYPE_INT_LIT] = "int-literal",
    [FRX_TOKEN_TYPE_FLOAT_LIT] = "float-literal",
    [FRX_TOKEN_TYPE_CHAR_LIT] = "char-literal",
    [FRX_TOKEN_TYPE_STR_LIT] = "string-literal",

    [FRX_TOKEN_TYPE_KW_U8] = "u8",
    [FRX_TOKEN_TYPE_KW_U16] = "u16",
    [FRX_TOKEN_TYPE_KW_U32] = "u32",
    [FRX_TOKEN_TYPE_KW_U64] = "u64",
    [FRX_TOKEN_TYPE_KW_USIZE] = "usize",
    [FRX_TOKEN_TYPE_KW_I8] = "i8",
    [FRX_TOKEN_TYPE_KW_I16] = "i16",
    [FRX_TOKEN_TYPE_KW_I32] = "i32",
    [FRX_TOKEN_TYPE_KW_I64] = "i64",
    [FRX_TOKEN_TYPE_KW_ISIZE] = "isize",
    [FRX_TOKEN_TYPE_KW_B8] = "b8",
    [FRX_TOKEN_TYPE_KW_B16] = "b16",
    [FRX_TOKEN_TYPE_KW_B32] = "b32",
    [FRX_TOKEN_TYPE_KW_B64] = "b64",
    [FRX_TOKEN_TYPE_KW_CHAR] = "char",
    [FRX_TOKEN_TYPE_KW_F32] = "f32",
    [FRX_TOKEN_TYPE_KW_F64] = "f64",
    [FRX_TOKEN_TYPE_KW_VOID] = "void",

    [FRX_TOKEN_TYPE_KW_NULLPTR] = "nullptr",
    [FRX_TOKEN_TYPE_KW_TRUE] = "true",
    [FRX_TOKEN_TYPE_KW_FALSE] = "false",
    [FRX_TOKEN_TYPE_KW_USE] = "use",
    [FRX_TOKEN_TYPE_KW_PUB] = "pub",
    [FRX_TOKEN_TYPE_KW_MUT] = "mut",
    [FRX_TOKEN_TYPE_KW_RETURN] = "return",
    [FRX_TOKEN_TYPE_KW_EXTERN] = "extern",
    [FRX_TOKEN_TYPE_KW_STRUCT] = "struct",
    [FRX_TOKEN_TYPE_KW_ENUM] = "enum",
    [FRX_TOKEN_TYPE_KW_IF] = "if",
    [FRX_TOKEN_TYPE_KW_ELSE] = "else",
    [FRX_TOKEN_TYPE_KW_SWITCH] = "switch",
    [FRX_TOKEN_TYPE_KW_CASE] = "case",
    [FRX_TOKEN_TYPE_KW_DEFAULT] = "default",
    [FRX_TOKEN_TYPE_KW_BREAK] = "break",
    [FRX_TOKEN_TYPE_KW_CONTINUE] = "continue",
    [FRX_TOKEN_TYPE_KW_FOR] = "for",
    [FRX_TOKEN_TYPE_KW_WHILE] = "while",
    [FRX_TOKEN_TYPE_KW_DO] = "do",
    [FRX_TOKEN_TYPE_KW_IMPL] = "impl",
    [FRX_TOKEN_TYPE_KW_SELF] = "self",
    [FRX_TOKEN_TYPE_KW_FN] = "fn",
    [FRX_TOKEN_TYPE_KW_DEFER] = "defer",

    [FRX_TOKEN_TYPE_PLUS] = "+",
    [FRX_TOKEN_TYPE_PLUS_EQ] = "+=",

    [FRX_TOKEN_TYPE_MINUS] = "-",
    [FRX_TOKEN_TYPE_MINUS_EQ] = "-=",

    [FRX_TOKEN_TYPE_STAR] = "*",
    [FRX_TOKEN_TYPE_STAR_EQ] = "*=",

    [FRX_TOKEN_TYPE_SLASH] = "/",
    [FRX_TOKEN_TYPE_SLASH_EQ] = "/=",

    [FRX_TOKEN_TYPE_MODULO] = "%",
    [FRX_TOKEN_TYPE_MODULO_EQ] = "%=",

    [FRX_TOKEN_TYPE_LOG_AND] = "&&",
    [FRX_TOKEN_TYPE_LOG_OR] = "||",
    [FRX_TOKEN_TYPE_LOG_NOT] = "!",
    [FRX_TOKEN_TYPE_LOG_NEQ] = "!=",
    [FRX_TOKEN_TYPE_LOG_EQ] = "==",

    [FRX_TOKEN_TYPE_BIT_AND] = "&",
    [FRX_TOKEN_TYPE_BIT_AND_EQ] = "&=",

    [FRX_TOKEN_TYPE_BIT_OR] = "|",
    [FRX_TOKEN_TYPE_BIT_OR_EQ] = "|=",

    [FRX_TOKEN_TYPE_BIT_XOR] = "^",
    [FRX_TOKEN_TYPE_BIT_XOR_EQ] = "^=",

    [FRX_TOKEN_TYPE_BIT_NOT] = "~",

    [FRX_TOKEN_TYPE_BIT_LSHIFT] = "<<",
    [FRX_TOKEN_TYPE_BIT_LSHIFT_EQ] = "<<=",

    [FRX_TOKEN_TYPE_BIT_RSHIFT] = ">>",
    [FRX_TOKEN_TYPE_BIT_RSHIFT_EQ] = ">>=",

    [FRX_TOKEN_TYPE_EQ] = "=",

    [FRX_TOKEN_TYPE_GT] = ">",
    [FRX_TOKEN_TYPE_GEQ] = ">=",

    [FRX_TOKEN_TYPE_LT] = "<",
    [FRX_TOKEN_TYPE_LEQ] = "<=",

    [FRX_TOKEN_TYPE_PLUS_PLUS] = "++",
    [FRX_TOKEN_TYPE_MINUS_MINUS] = "--",

    [FRX_TOKEN_TYPE_LPAREN] = "(",
    [FRX_TOKEN_TYPE_RPAREN] = ")",

    [FRX_TOKEN_TYPE_LBRACKET] = "[",
    [FRX_TOKEN_TYPE_RBRACKET] = "]",

    [FRX_TOKEN_TYPE_LBRACE] = "{",
    [FRX_TOKEN_TYPE_RBRACE] = "}",

    [FRX_TOKEN_TYPE_ARROW] = "->",
    [FRX_TOKEN_TYPE_DOT] = ".",
    [FRX_TOKEN_TYPE_COMMA] = ",",
    [FRX_TOKEN_TYPE_COLON] = ":",
    [FRX_TOKEN_TYPE_SEMI] = ";",

    [FRX_TOKEN_TYPE_ELLIPSIS] = "...",

    [FRX_TOKEN_TYPE_RESOLUTION] = "::"
};

b8 token_type_is_primitive(TokenType type)
{
    return type >= FRX_TOKEN_TYPE_KW_U8 && type <= FRX_TOKEN_TYPE_KW_VOID;
}

usize primitive_type_to_size(TokenType primitive_type)
{
    switch(primitive_type)
    {
        case FRX_TOKEN_TYPE_KW_U8:
        {
            return 1;
        }
        case FRX_TOKEN_TYPE_KW_U16:
        {
            return 2;
        }
        case FRX_TOKEN_TYPE_KW_U32:
        {
            return 4;
        }
        case FRX_TOKEN_TYPE_KW_U64:
        {
            return 8;
        }
        case FRX_TOKEN_TYPE_KW_USIZE:
        {
            //TODO: Depends on the target architecture
            return 8;
        }
        case FRX_TOKEN_TYPE_KW_I8:
        {
            return 1;
        }
        case FRX_TOKEN_TYPE_KW_I16:
        {
            return 2;
        }
        case FRX_TOKEN_TYPE_KW_I32:
        {
            return 4;
        }
        case FRX_TOKEN_TYPE_KW_I64:
        {
            return 8;
        }
        case FRX_TOKEN_TYPE_KW_ISIZE:
        {
            //TODO: Depends on the target architecture
            return 8;
        }
        case FRX_TOKEN_TYPE_KW_B8:
        {
            return 1;
        }
        case FRX_TOKEN_TYPE_KW_B16:
        {
            return 2;
        }
        case FRX_TOKEN_TYPE_KW_B32:
        {
            return 4;
        }
        case FRX_TOKEN_TYPE_KW_B64:
        {
            return 8;
        }
        case FRX_TOKEN_TYPE_KW_CHAR:
        {
            return 1;
        }
        case FRX_TOKEN_TYPE_KW_F32:
        {
            return 4;
        }
        case FRX_TOKEN_TYPE_KW_F64:
        {
            return 8;
        }
        default:
        {
            FRX_ASSERT(FRX_FALSE);
            return 0;
        }
    }
}

b8 token_type_is_sync(TokenType type)
{
    switch(type)
    {
        case FRX_TOKEN_TYPE_EOF:
        case FRX_TOKEN_TYPE_RBRACE:
        case FRX_TOKEN_TYPE_SEMI:
        {
            return FRX_TRUE;
        }
    }

    return FRX_FALSE;
}

char* token_type_to_str(TokenType type)
{
    FRX_ASSERT(type < (sizeof(token_type_names) / sizeof(token_type_names[0])));

    return token_type_names[type];
}
