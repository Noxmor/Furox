#include "assert.h"
#include "ast.h"
#include "diagnostics.h"
#include "operator.h"
#include "parser.h"

static AST* unary_expr_create(TokenType type, Operator operator, AST* operand)
{
    FRX_ASSERT(token_type_is_prefix_operator(type) || token_type_is_postfix_operator(type));

    FRX_ASSERT(operator < FRX_OPERATOR_COUNT);

    AST* ast = ast_create(FRX_AST_TYPE_UNARY_EXPR);
    ASTUnaryExpr* unary_expr = &ast->unary_expr;

    unary_expr->type = type;
    unary_expr->operator = operator;
    unary_expr->operand = operand;
    unary_expr->resolved_type = NULL;

    return ast;
}

static AST* binary_expr_create(TokenType type, Operator operator,
                               AST* left, AST* right)
{
    FRX_ASSERT(token_type_is_infix_operator(type));

    FRX_ASSERT(operator < FRX_OPERATOR_COUNT);

    AST* ast = ast_create(FRX_AST_TYPE_BINARY_EXPR);
    ASTBinaryExpr* binary_expr = &ast->binary_expr;

    binary_expr->type = type;
    binary_expr->operator = operator;
    binary_expr->left = left;
    binary_expr->right = right;
    binary_expr->resolved_type = NULL;

    return ast;
}

static AST* field_expr_create(AST* base, const char* field_name)
{
    FRX_ASSERT(base != NULL);

    FRX_ASSERT(field_name != NULL);

    AST* ast = ast_create(FRX_AST_TYPE_FIELD_EXPR);
    ASTFieldExpr* field_expr = &ast->field_expr;

    field_expr->base = base;
    field_expr->field_name = field_name;

    return ast;
}

static AST* expr_parse_primary(Parser* parser)
{
    switch (parser_current_type(parser))
    {
        case FRX_TOKEN_TYPE_INT_LIT: return int_literal_parse(parser);
        case FRX_TOKEN_TYPE_KW_EXTERN:
        case FRX_TOKEN_TYPE_IDENT: return path_expr_parse(parser);
        default:
        {
            Diagnostic* d = diagnostic_create(FRX_DIAGNOSTIC_ID_EXPECTED_EXPR,
                                              FRX_DIAGNOSTIC_LVL_ERROR,
                                              parser_current_token(parser)->range,
                                              token_type_to_str(parser_current_type(parser)));
            parser_add_diagnostic(parser, d);

            parser_recover(parser);

            return ast_create(FRX_AST_TYPE_ERROR);
        }
    }
}

static AST* expr_parse_with_precedence(Parser* parser, Precedence min_precedence)
{
    FRX_ASSERT(parser != NULL);

    AST* expr = NULL;

    if (parser_current_type(parser) == FRX_TOKEN_TYPE_LPAREN)
    {
        if (parser_eat(parser, FRX_TOKEN_TYPE_LPAREN))
        {
            return NULL;
        }

        expr = expr_parse(parser);

        if (parser_eat(parser, FRX_TOKEN_TYPE_RPAREN))
        {
            return NULL;
        }
    }
    else if (token_type_is_prefix_operator(parser_current_type(parser)))
    {
        TokenType type = parser_current_type(parser);

        if (parser_eat(parser, type))
        {
            return NULL;
        }

        Operator operator = token_type_to_prefix_operator(type);
        Precedence precedence = operator_to_precedence(operator);
        expr = unary_expr_create(type, operator, expr_parse_with_precedence(parser, precedence));
    }
    else
    {
        expr = expr_parse_primary(parser);
    }

    FRX_ASSERT(expr != NULL);

    while (FRX_TRUE)
    {
        TokenType type = parser_current_type(parser);

        if (token_type_is_postfix_operator(type))
        {
            Operator operator = token_type_to_postfix_operator(type);
            Precedence precedence = operator_to_precedence(operator);
            if (precedence >= min_precedence)
            {
                break;
            }

            if (parser_eat(parser, type))
            {
                return NULL;
            }

            if (operator == FRX_OPERATOR_ARRAY_SUBSCRIPT)
            {
                AST* index = expr_parse(parser);
                if (parser_eat(parser, FRX_TOKEN_TYPE_RBRACKET))
                {
                    return NULL;
                }

                expr = binary_expr_create(type, operator, expr, index);
            }
            else if (operator == FRX_OPERATOR_MEMBER_ACCESS)
            {
                const char* field_name = parser_current_token(parser)->identifier;
                if (parser_eat(parser, FRX_TOKEN_TYPE_IDENT))
                {
                    return NULL;
                }

                expr = field_expr_create(expr, field_name);
            }
            else if (operator == FRX_OPERATOR_CALL)
            {
                AST* call_expr = call_expr_parse(parser);
                expr = binary_expr_create(type, operator, expr, call_expr);
            }
            else
            {
                expr = unary_expr_create(type, operator, expr);
            }

            continue;
        }
        else if (token_type_is_infix_operator(type))
        {
            Operator operator = token_type_to_infix_operator(type);
            Precedence precedence = operator_to_precedence(operator);
            if (precedence >= min_precedence)
            {
                break;
            }

            parser_eat(parser, type);

            expr = binary_expr_create(type, operator, expr,
                                      expr_parse_with_precedence(parser, precedence));

            continue;
        }

        break;
    }

    return expr;
}

AST* expr_parse(Parser* parser)
{
    return expr_parse_with_precedence(parser, FRX_PRECEDENCE_MIN);
}
