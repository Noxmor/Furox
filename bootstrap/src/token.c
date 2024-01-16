#include "token.h"

#include "core/assert.h"

const char* token_type_to_str(TokenType type)
{
    FRX_ASSERT(type < FRX_TOKEN_TYPE_COUNT);

    switch(type)
    {
        case FRX_TOKEN_TYPE_EOF: return "EOF";
        
        case FRX_TOKEN_TYPE_IDENTIFIER: return "Identifier";

        case FRX_TOKEN_TYPE_NUMBER: return "Number";

        case FRX_TOKEN_TYPE_CHAR_LITERAL: return "Char Literal";
        case FRX_TOKEN_TYPE_STRING_LITERAL: return "String Literal";

        case FRX_TOKEN_TYPE_PLUS: return "Plus";
        case FRX_TOKEN_TYPE_MINUS: return "Minus";
        case FRX_TOKEN_TYPE_STAR: return "Star";
        case FRX_TOKEN_TYPE_SLASH: return "Slash";

        case FRX_TOKEN_TYPE_LOGICAL_NEGATION: return "Logical Negation";

        case FRX_TOKEN_TYPE_EQUALS: return "Equals";

        case FRX_TOKEN_TYPE_LEFT_PARANTHESIS: return "Left Paranthesis";
        case FRX_TOKEN_TYPE_RIGHT_PARANTHESIS: return "Right Paranthesis";

        case FRX_TOKEN_TYPE_LEFT_BRACKET: return "Left Bracket";
        case FRX_TOKEN_TYPE_RIGHT_BRACKET: return "Right Bracket";

        case FRX_TOKEN_TYPE_LEFT_BRACE: return "Left Brace";
        case FRX_TOKEN_TYPE_RIGHT_BRACE: return "Right Brace";

        case FRX_TOKEN_TYPE_COMMA: return "Comma";
        case FRX_TOKEN_TYPE_COLON: return "Colon";
        case FRX_TOKEN_TYPE_SEMICOLON: return "Semicolon";

        default: FRX_ASSERT(FRX_FALSE); break;
    }

    return "Unknown";
}
