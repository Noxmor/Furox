#include "assert.h"
#include "ast.h"
#include "compiler.h"
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

static AST* path_expr_create(AST* path)
{
    FRX_ASSERT(path != NULL);

    AST* ast = ast_create(FRX_AST_TYPE_PATH_EXPR);
    ASTPathExpr* path_expr = &ast->path_expr;

    path_expr->path = path;
    path_expr->resolved_type = NULL;

    return ast;
}

static AST* cast_expr_create(AST* expr, AST* type_specifier)
{
    FRX_ASSERT(expr != NULL);

    FRX_ASSERT(type_specifier != NULL);

    AST* ast = ast_create(FRX_AST_TYPE_CAST_EXPR);
    ASTCastExpr* cast_expr = &ast->cast_expr;

    cast_expr->expr = expr;
    cast_expr->type_specifier = type_specifier;

    return ast;
}

static AST* expr_parse_primary(Parser* parser)
{
    switch (parser_current_type(parser))
    {
        case FRX_TOKEN_TYPE_INT_LIT: return int_literal_parse(parser);
        case FRX_TOKEN_TYPE_CHAR_LIT: return char_literal_parse(parser);
        case FRX_TOKEN_TYPE_STR_LIT: return string_literal_parse(parser);
        case FRX_TOKEN_TYPE_KW_TRUE:
        case FRX_TOKEN_TYPE_KW_FALSE: return bool_expr_parse(parser);
        case FRX_TOKEN_TYPE_KW_NULLPTR: return nullptr_expr_parse(parser);
        case FRX_TOKEN_TYPE_KW_SELF_LOWER: return self_expr_parse(parser);
        case FRX_TOKEN_TYPE_KW_EXTERN:
        case FRX_TOKEN_TYPE_IDENT:
        case FRX_TOKEN_TYPE_KW_SELF_UPPER:
        {
            AST* path = path_parse(parser, FRX_PATH_STYLE_EXPR);
            if (parser_current_type(parser) == FRX_TOKEN_TYPE_LBRACE)
            {
                return struct_literal_parse(parser, path);
            }

            return path_expr_create(path);
        }
        default:
        {
            Diagnostic* d = diagnostic_create(FRX_DIAGNOSTIC_ID_EXPECTED_EXPR,
                                              FRX_DIAGNOSTIC_LVL_ERROR,
                                              parser_current_span(parser),
                                              token_type_to_str(parser_current_type(parser)));
            compiler_add_diagnostic(d);

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
                const char* name = parser_current_token(parser)->identifier;
                if (parser_eat(parser, FRX_TOKEN_TYPE_IDENT))
                {
                    return NULL;
                }

                if (parser_current_type(parser) == FRX_TOKEN_TYPE_LPAREN)
                {
                    AST* method_call_expr = method_call_expr_parse(parser, name, expr);
                    expr = method_call_expr;
                }
                else
                {
                    expr = field_expr_create(expr, name);
                }
            }
            else if (operator == FRX_OPERATOR_CALL)
            {
                expr = call_expr_parse(parser, expr);
            }
            else if (operator == FRX_OPERATOR_CAST)
            {
                AST* casted_type = type_specifier_parse(parser);
                expr = cast_expr_create(expr, casted_type);
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
