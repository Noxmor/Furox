#include "token.h"

#include "core/assert.h"

const char* token_type_to_str(TokenType type)
{
    FRX_ASSERT(type < FRX_TOKEN_TYPE_COUNT);

    switch(type)
    {
        case FRX_TOKEN_TYPE_EOF: return "EOF";

        case FRX_TOKEN_TYPE_KW_U8: return "u8";
        case FRX_TOKEN_TYPE_KW_U16: return "u16";
        case FRX_TOKEN_TYPE_KW_U32: return "u32";
        case FRX_TOKEN_TYPE_KW_U64: return "u64";
        case FRX_TOKEN_TYPE_KW_USIZE: return "usize";
        case FRX_TOKEN_TYPE_KW_I8: return "i8";
        case FRX_TOKEN_TYPE_KW_I16: return "i16";
        case FRX_TOKEN_TYPE_KW_I32: return "i32";
        case FRX_TOKEN_TYPE_KW_I64: return "i64";
        case FRX_TOKEN_TYPE_KW_ISIZE: return "isize";
        case FRX_TOKEN_TYPE_KW_B8: return "b8";
        case FRX_TOKEN_TYPE_KW_B16: return "b16";
        case FRX_TOKEN_TYPE_KW_B32: return "b32";
        case FRX_TOKEN_TYPE_KW_B64: return "b64";
        case FRX_TOKEN_TYPE_KW_CHAR: return "char";
        case FRX_TOKEN_TYPE_KW_F32: return "f32";
        case FRX_TOKEN_TYPE_KW_F64: return "f64";
        case FRX_TOKEN_TYPE_KW_VOID: return "void";

        case FRX_TOKEN_TYPE_KW_NULLPTR: return "nullptr";
        case FRX_TOKEN_TYPE_KW_TRUE: return "true";
        case FRX_TOKEN_TYPE_KW_FALSE: return "false";
        case FRX_TOKEN_TYPE_KW_IMPORT: return "import";
        case FRX_TOKEN_TYPE_KW_RETURN: return "return";
        case FRX_TOKEN_TYPE_KW_NAMESPACE: return "namespace";
        case FRX_TOKEN_TYPE_KW_EXTERN: return "extern";
        case FRX_TOKEN_TYPE_KW_ENUM: return "enum";
        case FRX_TOKEN_TYPE_KW_STRUCT: return "struct";
        case FRX_TOKEN_TYPE_KW_EXPORT: return "export";
        case FRX_TOKEN_TYPE_KW_IF: return "if";
        case FRX_TOKEN_TYPE_KW_ELSE: return "else";
        case FRX_TOKEN_TYPE_KW_SWITCH: return "switch";
        case FRX_TOKEN_TYPE_KW_CASE: return "case";
        case FRX_TOKEN_TYPE_KW_DEFAULT: return "default";
        case FRX_TOKEN_TYPE_KW_BREAK: return "break";
        case FRX_TOKEN_TYPE_KW_FOR: return "for";
        case FRX_TOKEN_TYPE_KW_WHILE: return "while";
        case FRX_TOKEN_TYPE_KW_DO: return "do";

        case FRX_TOKEN_TYPE_IDENTIFIER: return "Identifier";

        case FRX_TOKEN_TYPE_NUMBER: return "Number";

        case FRX_TOKEN_TYPE_CHAR_LITERAL: return "Char Literal";
        case FRX_TOKEN_TYPE_STRING_LITERAL: return "String Literal";

        case FRX_TOKEN_TYPE_PLUS: return "Plus";
        case FRX_TOKEN_TYPE_MINUS: return "Minus";
        case FRX_TOKEN_TYPE_STAR: return "Star";
        case FRX_TOKEN_TYPE_SLASH: return "Slash";
        case FRX_TOKEN_TYPE_MODULO: return "Modulo";

        case FRX_TOKEN_TYPE_LOGICAL_AND: return "Logical And";
        case FRX_TOKEN_TYPE_LOGICAL_OR: return "Logical Or";
        case FRX_TOKEN_TYPE_LOGICAL_NEGATION: return "Logical Negation";

        case FRX_TOKEN_TYPE_BINARY_AND: return "Binary And";
        case FRX_TOKEN_TYPE_BINARY_OR: return "Binary Or";
        case FRX_TOKEN_TYPE_BINARY_XOR: return "Binary Xor";
        case FRX_TOKEN_TYPE_BINARY_NEGATION: return "Binary Negation";
        case FRX_TOKEN_TYPE_BINARY_LEFT_SHIFT: return "Binary Left Shift";
        case FRX_TOKEN_TYPE_BINARY_RIGHT_SHIFT: return "Binary Right Shift";

        case FRX_TOKEN_TYPE_EQUALS: return "Equals";
        case FRX_TOKEN_TYPE_COMPARISON: return "Comparison";

        case FRX_TOKEN_TYPE_GREATER_THAN: return "Greater than";
        case FRX_TOKEN_TYPE_GREATER_THAN_EQUALS: return "Greater than or equals";

        case FRX_TOKEN_TYPE_LESS_THAN: return "Less than";
        case FRX_TOKEN_TYPE_LESS_THAN_EQUALS: return "Less than or equals";

        case FRX_TOKEN_TYPE_LEFT_PARANTHESIS: return "Left Paranthesis";
        case FRX_TOKEN_TYPE_RIGHT_PARANTHESIS: return "Right Paranthesis";

        case FRX_TOKEN_TYPE_LEFT_BRACKET: return "Left Bracket";
        case FRX_TOKEN_TYPE_RIGHT_BRACKET: return "Right Bracket";

        case FRX_TOKEN_TYPE_LEFT_BRACE: return "Left Brace";
        case FRX_TOKEN_TYPE_RIGHT_BRACE: return "Right Brace";

        case FRX_TOKEN_TYPE_ARROW: return "Arrow";

        case FRX_TOKEN_TYPE_COMMA: return "Comma";
        case FRX_TOKEN_TYPE_DOT: return "Dot";
        case FRX_TOKEN_TYPE_COLON: return "Colon";
        case FRX_TOKEN_TYPE_SEMICOLON: return "Semicolon";

        case FRX_TOKEN_TYPE_ELLIPSIS: return "Ellipsis";

        case FRX_TOKEN_TYPE_NAMESPACE_RESOLUTION: return "Namespace Resolution";

        default: FRX_ASSERT(FRX_FALSE); break;
    }

    return "Unknown";
}
