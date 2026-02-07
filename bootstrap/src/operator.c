#include "operator.h"
#include "assert.h"

b8 token_type_is_prefix_operator(TokenType type)
{
    return token_type_to_prefix_operator(type) != FRX_OPERATOR_INVALID;
}

Operator token_type_to_prefix_operator(TokenType type)
{
    FRX_ASSERT(type < FRX_TOKEN_TYPE_COUNT);

    switch(type)
    {
        case FRX_TOKEN_TYPE_PLUS: return FRX_OPERATOR_SIGN_PLUS;
        case FRX_TOKEN_TYPE_MINUS: return FRX_OPERATOR_SIGN_MINUS;
        case FRX_TOKEN_TYPE_STAR: return FRX_OPERATOR_DEREF;
        case FRX_TOKEN_TYPE_BIT_AND: return FRX_OPERATOR_ADDRESS_OF;
        case FRX_TOKEN_TYPE_LOG_NOT: return FRX_OPERATOR_LOG_NOT;
        case FRX_TOKEN_TYPE_BIT_NOT: return FRX_OPERATOR_BIT_NOT;
        case FRX_TOKEN_TYPE_PLUS_PLUS: return FRX_OPERATOR_PRE_INC;
        case FRX_TOKEN_TYPE_MINUS_MINUS: return FRX_OPERATOR_PRE_DEC;
    }

    return FRX_OPERATOR_INVALID;
}

b8 token_type_is_infix_operator(TokenType type)
{
    return token_type_to_infix_operator(type) != FRX_OPERATOR_INVALID;
}

Operator token_type_to_infix_operator(TokenType type)
{
    FRX_ASSERT(type < FRX_TOKEN_TYPE_COUNT);
    switch(type)
    {
        case FRX_TOKEN_TYPE_PLUS: return FRX_OPERATOR_ADD;
        case FRX_TOKEN_TYPE_PLUS_EQ: return FRX_OPERATOR_ADD_ASSIGN;
        case FRX_TOKEN_TYPE_MINUS: return FRX_OPERATOR_SUB;
        case FRX_TOKEN_TYPE_MINUS_EQ: return FRX_OPERATOR_SUB_ASSIGN;
        case FRX_TOKEN_TYPE_STAR: return FRX_OPERATOR_MUL;
        case FRX_TOKEN_TYPE_STAR_EQ: return FRX_OPERATOR_MUL_ASSIGN;
        case FRX_TOKEN_TYPE_SLASH: return FRX_OPERATOR_DIV;
        case FRX_TOKEN_TYPE_SLASH_EQ: return FRX_OPERATOR_DIV_ASSIGN;
        case FRX_TOKEN_TYPE_MODULO: return FRX_OPERATOR_MOD;
        case FRX_TOKEN_TYPE_MODULO_EQ: return FRX_OPERATOR_MOD_ASSIGN;
        case FRX_TOKEN_TYPE_LOG_AND: return FRX_OPERATOR_LOG_AND;
        case FRX_TOKEN_TYPE_LOG_OR: return FRX_OPERATOR_LOG_OR;
        case FRX_TOKEN_TYPE_BIT_AND: return FRX_OPERATOR_BIT_AND;
        case FRX_TOKEN_TYPE_BIT_AND_EQ: return FRX_OPERATOR_BIT_AND_ASSIGN;
        case FRX_TOKEN_TYPE_BIT_OR: return FRX_OPERATOR_BIT_OR;
        case FRX_TOKEN_TYPE_BIT_OR_EQ: return FRX_OPERATOR_BIT_OR_ASSIGN;
        case FRX_TOKEN_TYPE_BIT_XOR: return FRX_OPERATOR_BIT_XOR;
        case FRX_TOKEN_TYPE_BIT_XOR_EQ: return FRX_OPERATOR_BIT_XOR_ASSIGN;
        case FRX_TOKEN_TYPE_BIT_LSHIFT: return FRX_OPERATOR_BIT_LSHIFT;
        case FRX_TOKEN_TYPE_BIT_LSHIFT_EQ: return FRX_OPERATOR_BIT_LSHIFT_ASSIGN;
        case FRX_TOKEN_TYPE_BIT_RSHIFT: return FRX_OPERATOR_BIT_RSHIFT;
        case FRX_TOKEN_TYPE_BIT_RSHIFT_EQ: return FRX_OPERATOR_BIT_RSHIFT_ASSIGN;
        case FRX_TOKEN_TYPE_EQ: return FRX_OPERATOR_ASSIGN;
        case FRX_TOKEN_TYPE_LOG_NEQ: return FRX_OPERATOR_LOG_NEQ;
        case FRX_TOKEN_TYPE_LOG_EQ: return FRX_OPERATOR_LOG_EQ;
        case FRX_TOKEN_TYPE_GT: return FRX_OPERATOR_GT;
        case FRX_TOKEN_TYPE_GEQ: return FRX_OPERATOR_GEQ;
        case FRX_TOKEN_TYPE_LT: return FRX_OPERATOR_LT;
        case FRX_TOKEN_TYPE_LEQ: return FRX_OPERATOR_LEQ;
        case FRX_TOKEN_TYPE_LBRACKET: return FRX_OPERATOR_ARRAY_SUBSCRIPT;
        case FRX_TOKEN_TYPE_LPAREN: return FRX_OPERATOR_CALL;
        case FRX_TOKEN_TYPE_DOT: return FRX_OPERATOR_MEMBER_ACCESS;
    }

    return FRX_OPERATOR_INVALID;
}

b8 token_type_is_postfix_operator(TokenType type)
{
    return token_type_to_postfix_operator(type) != FRX_OPERATOR_INVALID;
}

Operator token_type_to_postfix_operator(TokenType type)
{
    FRX_ASSERT(type < FRX_TOKEN_TYPE_COUNT);

    switch(type)
    {
        case FRX_TOKEN_TYPE_PLUS_PLUS: return FRX_OPERATOR_POST_INC;
        case FRX_TOKEN_TYPE_MINUS_MINUS: return FRX_OPERATOR_POST_DEC;
        case FRX_TOKEN_TYPE_LBRACKET: return FRX_OPERATOR_ARRAY_SUBSCRIPT;
        case FRX_TOKEN_TYPE_LPAREN: return FRX_OPERATOR_CALL;
        case FRX_TOKEN_TYPE_DOT: return FRX_OPERATOR_MEMBER_ACCESS;
    }

    return FRX_OPERATOR_INVALID;
}

Precedence operator_to_precedence(Operator operator)
{
    FRX_ASSERT(operator < FRX_OPERATOR_COUNT);

    switch(operator)
    {
        case FRX_OPERATOR_POST_INC:
        case FRX_OPERATOR_POST_DEC:
        case FRX_OPERATOR_ARRAY_SUBSCRIPT:
        case FRX_OPERATOR_CALL:
        case FRX_OPERATOR_MEMBER_ACCESS: return FRX_PRECEDENCE_1;
        case FRX_OPERATOR_PRE_INC:
        case FRX_OPERATOR_PRE_DEC:
        case FRX_OPERATOR_SIGN_PLUS:
        case FRX_OPERATOR_SIGN_MINUS:
        case FRX_OPERATOR_LOG_NOT:
        case FRX_OPERATOR_BIT_NOT:
        case FRX_OPERATOR_DEREF:
        case FRX_OPERATOR_ADDRESS_OF: return FRX_PRECEDENCE_2;
        case FRX_OPERATOR_MUL:
        case FRX_OPERATOR_DIV:
        case FRX_OPERATOR_MOD: return FRX_PRECEDENCE_3;
        case FRX_OPERATOR_ADD:
        case FRX_OPERATOR_SUB: return FRX_PRECEDENCE_4;
        case FRX_OPERATOR_BIT_LSHIFT:
        case FRX_OPERATOR_BIT_RSHIFT: return FRX_PRECEDENCE_5;
        case FRX_OPERATOR_LT:
        case FRX_OPERATOR_LEQ:
        case FRX_OPERATOR_GT:
        case FRX_OPERATOR_GEQ: return FRX_PRECEDENCE_6;
        case FRX_OPERATOR_LOG_EQ:
        case FRX_OPERATOR_LOG_NEQ: return FRX_PRECEDENCE_7;
        case FRX_OPERATOR_BIT_AND: return FRX_PRECEDENCE_8;
        case FRX_OPERATOR_BIT_XOR: return FRX_PRECEDENCE_9;
        case FRX_OPERATOR_BIT_OR: return FRX_PRECEDENCE_10;
        case FRX_OPERATOR_LOG_AND: return FRX_PRECEDENCE_11;
        case FRX_OPERATOR_LOG_OR: return FRX_PRECEDENCE_12;
        case FRX_OPERATOR_TERNARY: return FRX_PRECEDENCE_13;
        case FRX_OPERATOR_ASSIGN:
        case FRX_OPERATOR_ADD_ASSIGN:
        case FRX_OPERATOR_SUB_ASSIGN:
        case FRX_OPERATOR_MUL_ASSIGN:
        case FRX_OPERATOR_DIV_ASSIGN:
        case FRX_OPERATOR_MOD_ASSIGN:
        case FRX_OPERATOR_BIT_LSHIFT_ASSIGN:
        case FRX_OPERATOR_BIT_RSHIFT_ASSIGN:
        case FRX_OPERATOR_BIT_AND_ASSIGN:
        case FRX_OPERATOR_BIT_XOR_ASSIGN:
        case FRX_OPERATOR_BIT_OR_ASSIGN: return FRX_PRECEDENCE_14;
        default: FRX_ASSERT(FRX_FALSE); break;
    }

    return FRX_PRECEDENCE_MIN;
}
