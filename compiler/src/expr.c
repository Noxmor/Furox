#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "diagnostics.h"
#include "parser.h"
#include "sema.h"
#include "codegen.h"

typedef void (*ExprSemaFunc)(void*);

static const ExprSemaFunc expr_type_to_sema[FRX_EXPR_TYPE_COUNT] = {
    [FRX_EXPR_TYPE_INT_LIT] = (ExprSemaFunc)int_literal_sema,
    [FRX_EXPR_TYPE_UNARY_EXPR] = (ExprSemaFunc)unary_expr_sema,
    [FRX_EXPR_TYPE_BINARY_EXPR] = (ExprSemaFunc)binary_expr_sema,
    [FRX_EXPR_TYPE_FUNC_CALL] = (ExprSemaFunc)func_call_sema,
    [FRX_EXPR_TYPE_VAR] = (ExprSemaFunc)var_sema,
};

typedef void (*ExprCodegenFunc)(void*);

static const ExprCodegenFunc expr_type_to_codegen[FRX_EXPR_TYPE_COUNT] = {
    [FRX_EXPR_TYPE_INT_LIT] = (ExprCodegenFunc)int_literal_codegen,
    [FRX_EXPR_TYPE_UNARY_EXPR] = (ExprCodegenFunc)unary_expr_codegen,
    [FRX_EXPR_TYPE_BINARY_EXPR] = (ExprCodegenFunc)binary_expr_codegen,
    [FRX_EXPR_TYPE_FUNC_CALL] = (ExprCodegenFunc)func_call_codegen,
    [FRX_EXPR_TYPE_VAR] = (ExprCodegenFunc)var_codegen,
};

static Expr* expr_create(ExprType type, void* node)
{
    FRX_ASSERT(type < FRX_EXPR_TYPE_COUNT);
    FRX_ASSERT(node != NULL);

    Expr* expr = compiler_alloc(sizeof(Expr));

    expr->type = type;
    expr->node = node;

    return expr;
}

static UnaryExpr* unary_expr_create(TokenType type, Operator operator, Expr* operand)
{
    FRX_ASSERT(token_type_is_prefix_operator(type) || token_type_is_postfix_operator(type));

    FRX_ASSERT(operator < FRX_OPERATOR_COUNT);

    UnaryExpr* unary_expr = compiler_alloc(sizeof(UnaryExpr));

    unary_expr->type = type;
    unary_expr->operator = operator;
    unary_expr->operand = operand;

    return unary_expr;
}

static BinaryExpr* binary_expr_create(TokenType type, Operator operator, Expr* left, Expr* right)
{
    FRX_ASSERT(token_type_is_infix_operator(type));

    FRX_ASSERT(operator < FRX_OPERATOR_COUNT);

    BinaryExpr* binary_expr = compiler_alloc(sizeof(BinaryExpr));

    binary_expr->type = type;
    binary_expr->operator = operator;
    binary_expr->left = left;
    binary_expr->right = right;

    return binary_expr;
}

static Expr* expr_parse_primary(Parser* parser)
{
    switch (parser_current_type(parser))
    {
        case FRX_TOKEN_TYPE_INT_LIT: return expr_create(FRX_EXPR_TYPE_INT_LIT, int_literal_parse(parser));
        case FRX_TOKEN_TYPE_IDENT:
        {
            if (parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_LPAREN)
            {
                return expr_create(FRX_EXPR_TYPE_FUNC_CALL, func_call_parse(parser));
            }

            return expr_create(FRX_EXPR_TYPE_VAR, var_parse(parser));
        }
        default:
        {
            FRX_PARSER_ADD_DIAGNOSTIC(parser, FRX_DIAGNOSTIC_ID_EXPECTED_EXPR,
                                      FRX_DIAGNOSTIC_LVL_ERROR,
                                      parser_current_token(parser)->range,
                                      token_type_to_str(parser_current_type(parser)));
            parser_recover(parser);

            return NULL;
        }
    }
}

static Expr* expr_parse_with_precedence(Parser* parser, Precedence min_precedence)
{
    FRX_ASSERT(parser != NULL);

    Expr* expr = NULL;

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
        UnaryExpr* unary_expr = unary_expr_create(type, operator,
                                                  expr_parse_with_precedence(parser, precedence));
        expr = expr_create(FRX_EXPR_TYPE_UNARY_EXPR, unary_expr);
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
                Expr* index = expr_parse(parser);
                if (parser_eat(parser, FRX_TOKEN_TYPE_RBRACKET))
                {
                    return NULL;
                }

                BinaryExpr* binary_expr = binary_expr_create(type, operator, expr, index);
                expr = expr_create(FRX_EXPR_TYPE_BINARY_EXPR, binary_expr);
            }
            else
            {
                UnaryExpr* unary_expr = unary_expr_create(type, operator, expr);
                expr = expr_create(FRX_EXPR_TYPE_UNARY_EXPR, unary_expr);
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

            BinaryExpr* binary_expr = binary_expr_create(type, operator, expr,
                                                         expr_parse_with_precedence(parser, precedence));
            expr = expr_create(FRX_EXPR_TYPE_BINARY_EXPR, binary_expr);

            continue;
        }

        break;
    }

    return expr;
}

Expr* expr_parse(Parser* parser)
{
    return expr_parse_with_precedence(parser, FRX_PRECEDENCE_MIN);
}

void expr_sema(Expr* expr)
{
    FRX_ASSERT(expr != NULL);

    expr_type_to_sema[expr->type](expr->node);
}

void expr_codegen(Expr* expr)
{
    FRX_ASSERT(expr != NULL);

    expr_type_to_codegen[expr->type](expr->node);
}
