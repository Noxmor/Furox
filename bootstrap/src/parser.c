#include "parser.h"

#include <string.h>

#include "ast.h"
#include "containers/list.h"
#include "core/assert.h"
#include "core/core.h"
#include "core/log.h"
#include "core/memory.h"
#include "namespace.h"
#include "symbols/function_table.h"
#include "symbols/struct_table.h"
#include "symbols/symbol_table.h"
#include "token.h"

typedef struct ParserInfo
{
    usize tokens_processed;
} ParserInfo;

static ParserInfo parser_info;

static Token* parser_peek(Parser* parser, usize offset)
{
    FRX_ASSERT(parser != NULL);

    return lexer_peek(&parser->lexer, offset);
}

static Token* parser_current_token(Parser* parser)
{
    return parser_peek(parser, 0);
}

static SourceLocation parser_current_location(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    SourceLocation location;
    memcpy(&location, &parser_current_token(parser)->location, sizeof(SourceLocation));

    return location;
}

static void parser_recover(Parser* parser, SourceLocation* location)
{
    FRX_ASSERT(parser != NULL);

    //TODO: What to do when the recovery fails?
    b8 result = lexer_recover(&parser->lexer, location);
    (void)result;
}

static FRX_NO_DISCARD b8 parser_eat(Parser* parser, TokenType type)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(type < FRX_TOKEN_TYPE_COUNT);

    if(parser_current_token(parser)->type != type)
        return FRX_TRUE;

    ++parser_info.tokens_processed;

    if(lexer_next_token(&parser->lexer))
    {
        //TODO: Handle error and ensure that the lexer only returns an error if some file IO failed.
        //If parsing a more complex token failed the lexer should handle this internally and not return an error.
    }

    return FRX_FALSE;
}

static void parser_skip(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    ++parser_info.tokens_processed;

    lexer_next_token(&parser->lexer);
}

static FRX_NO_DISCARD b8 is_primitive(TokenType type)
{
    switch(type)
    {
        case FRX_TOKEN_TYPE_KW_U8:
        case FRX_TOKEN_TYPE_KW_U16:
        case FRX_TOKEN_TYPE_KW_U32:
        case FRX_TOKEN_TYPE_KW_U64:
        case FRX_TOKEN_TYPE_KW_USIZE:
        case FRX_TOKEN_TYPE_KW_I8:
        case FRX_TOKEN_TYPE_KW_I16:
        case FRX_TOKEN_TYPE_KW_I32:
        case FRX_TOKEN_TYPE_KW_I64:
        case FRX_TOKEN_TYPE_KW_ISIZE:
        case FRX_TOKEN_TYPE_KW_B8:
        case FRX_TOKEN_TYPE_KW_B16:
        case FRX_TOKEN_TYPE_KW_B32:
        case FRX_TOKEN_TYPE_KW_B64:
        case FRX_TOKEN_TYPE_KW_CHAR:
        case FRX_TOKEN_TYPE_KW_F32:
        case FRX_TOKEN_TYPE_KW_F64:
        case FRX_TOKEN_TYPE_KW_VOID: return FRX_TRUE;
    }

    return FRX_FALSE;
}

static void parser_skip_optional_keywords(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    b8 parse_optional_keywords = FRX_TRUE;
    while(parse_optional_keywords)
    {
        switch(parser_current_token(parser)->type)
        {
            case FRX_TOKEN_TYPE_KW_EXPORT: parser_skip(parser); break;

            default: parse_optional_keywords = FRX_FALSE; break;
        }
    }
}

static void parser_skip_namespace_resolution(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    while(parser_current_token(parser)->type == FRX_TOKEN_TYPE_IDENTIFIER
            && parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_NAMESPACE_RESOLUTION)
    {
        parser_skip(parser);
        parser_skip(parser);
    }
}

static usize parser_get_precedence(ASTType type)
{
    switch(type)
    {
        case FRX_AST_TYPE_VARIABLE_ASSIGNMENT: return 0;

        case FRX_AST_TYPE_LOGICAL_OR: return 1;

        case FRX_AST_TYPE_LOGICAL_AND: return 2;

        case FRX_AST_TYPE_BINARY_OR: return 3;

        case FRX_AST_TYPE_BINARY_XOR: return 4;

        case FRX_AST_TYPE_BINARY_AND: return 5;

        case FRX_AST_TYPE_NEGATED_COMPARISON:
        case FRX_AST_TYPE_COMPARISON: return 6;

        case FRX_AST_TYPE_GREATER_THAN:
        case FRX_AST_TYPE_GREATER_THAN_EQUALS:
        case FRX_AST_TYPE_LESS_THAN:
        case FRX_AST_TYPE_LESS_THAN_EQUALS: return 7;

        case FRX_AST_TYPE_BINARY_LEFT_SHIFT:
        case FRX_AST_TYPE_BINARY_RIGHT_SHIFT: return 8;

        case FRX_AST_TYPE_ADDITION:
        case FRX_AST_TYPE_SUBTRACTION: return 9;

        case FRX_AST_TYPE_MULTIPLICATION:
        case FRX_AST_TYPE_DIVISION:
        case FRX_AST_TYPE_MODULO: return 10;

        case FRX_AST_TYPE_ARITHMETIC_NEGATION:
        case FRX_AST_TYPE_LOGICAL_NEGATION:
        case FRX_AST_TYPE_BINARY_NEGATION:

        case FRX_AST_TYPE_DEREFERENCE:
        case FRX_AST_TYPE_ADDRESS_OF: return 11;
    }

    return 0xFFFFFFFFFFFFFFFF;
}

static b8 parser_next_token_is_unary_operator(Parser* parser)
{
    switch(parser_current_token(parser)->type)
    {
        case FRX_TOKEN_TYPE_MINUS:
        case FRX_TOKEN_TYPE_LOGICAL_NEGATION:
        case FRX_TOKEN_TYPE_BINARY_NEGATION:

        case FRX_TOKEN_TYPE_STAR:
        case FRX_TOKEN_TYPE_BINARY_AND: return FRX_TRUE;
    }

    return FRX_FALSE;
}

static b8 parser_next_token_is_binary_operator(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    switch(parser_current_token(parser)->type)
    {
        case FRX_TOKEN_TYPE_EQUALS:

        case FRX_TOKEN_TYPE_PLUS:
        case FRX_TOKEN_TYPE_MINUS:
        case FRX_TOKEN_TYPE_STAR:
        case FRX_TOKEN_TYPE_SLASH:
        case FRX_TOKEN_TYPE_MODULO:

        case FRX_TOKEN_TYPE_LOGICAL_AND:
        case FRX_TOKEN_TYPE_LOGICAL_OR:

        case FRX_TOKEN_TYPE_BINARY_AND:
        case FRX_TOKEN_TYPE_BINARY_OR:
        case FRX_TOKEN_TYPE_BINARY_XOR:
        case FRX_TOKEN_TYPE_BINARY_LEFT_SHIFT:
        case FRX_TOKEN_TYPE_BINARY_RIGHT_SHIFT:

        case FRX_TOKEN_TYPE_NOT_EQUALS:
        case FRX_TOKEN_TYPE_COMPARISON:

        case FRX_TOKEN_TYPE_GREATER_THAN:
        case FRX_TOKEN_TYPE_GREATER_THAN_EQUALS:

        case FRX_TOKEN_TYPE_LESS_THAN:
        case FRX_TOKEN_TYPE_LESS_THAN_EQUALS: return FRX_TRUE;
    }

    return FRX_FALSE;
}

static FRX_NO_DISCARD AST* parser_parse_expression(Parser* parser);

static FRX_NO_DISCARD ASTNamespaceRef* parser_parse_namespace_resolution(Parser* parser);

static FRX_NO_DISCARD AST* parser_parse_top_level_definition(Parser* parser);

static FRX_NO_DISCARD ASTScope* parser_parse_scope(Parser* parser);

static FRX_NO_DISCARD AST* parser_parse_statement(Parser* parser);

static FRX_NO_DISCARD ASTSizeof* parser_parse_sizeof(Parser* parser);

static FRX_NO_DISCARD ASTAssert* parser_parse_assert(Parser* parser);

static FRX_NO_DISCARD ASTVariable* parser_parse_variable(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    ASTVariable* variable = arena_alloc(&parser->arena, sizeof(ASTVariable));

    VariableTable* variable_table = get_global_variable_table();

    variable->variable_symbol = variable_table_find(variable_table,
        parser_current_token(parser)->identifier, parser->current_namespace);

    strcpy(variable->name, parser_current_token(parser)->identifier);

    if(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected identifier for variable!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_LEFT_BRACKET)
    {
        parser_skip(parser);

        variable->array_index = parser_parse_expression(parser);
        if(variable->array_index == NULL)
            return NULL;

        if(parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_BRACKET))
        {
            SourceLocation location = parser_current_location(parser);
            FRX_ERROR_FILE("Missing ']' after array-index!", parser->lexer.filepath, location.line, location.coloumn);

            return NULL;
        }
    }
    else
        variable->array_index = NULL;

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_DOT || parser_current_token(parser)->type == FRX_TOKEN_TYPE_ARROW)
    {
        variable->is_pointer = parser_current_token(parser)->type == FRX_TOKEN_TYPE_ARROW;

        parser_skip(parser);

        variable->next = parser_parse_variable(parser);
        if(variable->next == NULL)
            return NULL;
    }
    else
        variable->next = NULL;

    return variable;
}

static FRX_NO_DISCARD b8 is_number(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    switch(parser_current_token(parser)->type)
    {
        case FRX_TOKEN_TYPE_NUMBER:
        case FRX_TOKEN_TYPE_KW_NULLPTR:
        case FRX_TOKEN_TYPE_KW_FALSE:
        case FRX_TOKEN_TYPE_KW_TRUE: return FRX_TRUE;
    }

    return FRX_FALSE;
}

static FRX_NO_DISCARD ASTNumber* parser_parse_number(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    ASTNumber* number = arena_alloc(&parser->arena, sizeof(ASTNumber));

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_NULLPTR)
    {
        parser_skip(parser);

        number->number = 0;

        return number;
    }

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_TRUE)
    {
        parser_skip(parser);

        number->number = 1;

        return number;
    }

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_FALSE)
    {
        parser_skip(parser);

        number->number = 0;

        return number;
    }

    number->number = parser_current_token(parser)->number;

    if(parser_eat(parser, FRX_TOKEN_TYPE_NUMBER))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected number!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    return number;
}

static FRX_NO_DISCARD b8 is_char_literal(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    return parser_current_token(parser)->type == FRX_TOKEN_TYPE_CHAR_LITERAL;
}

static FRX_NO_DISCARD ASTCharLiteral* parser_parse_char_literal(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    ASTCharLiteral* char_literal = arena_alloc(&parser->arena, sizeof(ASTCharLiteral));

    strcpy(char_literal->literal, parser_current_token(parser)->identifier);

    if(parser_eat(parser, FRX_TOKEN_TYPE_CHAR_LITERAL))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected char-literal!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    return char_literal;
}

static FRX_NO_DISCARD b8 is_string_literal(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    return parser_current_token(parser)->type == FRX_TOKEN_TYPE_STRING_LITERAL;
}

static FRX_NO_DISCARD ASTStringLiteral* parser_parse_string_literal(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    ASTStringLiteral* string_literal = arena_alloc(&parser->arena, sizeof(ASTStringLiteral));

    strcpy(string_literal->literal, parser_current_token(parser)->identifier);

    if(parser_eat(parser, FRX_TOKEN_TYPE_STRING_LITERAL))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected string-literal!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    return string_literal;
}

static FRX_NO_DISCARD b8 is_function_call(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    SourceLocation location = parser_current_location(parser);

    parser_skip_namespace_resolution(parser);

    b8 result = parser_current_token(parser)->type == FRX_TOKEN_TYPE_IDENTIFIER && parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_LEFT_PARANTHESIS;

    parser_recover(parser, &location);

    return result;
}

static FRX_NO_DISCARD ASTFunctionCall* parser_parse_function_call(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    FunctionTable* function_table = get_global_function_table();

    ASTFunctionCall* function_call = arena_alloc(&parser->arena, sizeof(ASTFunctionCall));

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_IDENTIFIER
        && parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_NAMESPACE_RESOLUTION)
    {
        SourceLocation location = parser_current_location(parser);

        Namespace* namespace = namespace_create(parser_current_token(parser)->identifier);

        parser_skip(parser);
        parser_skip(parser);

        while(parser_current_token(parser)->type == FRX_TOKEN_TYPE_IDENTIFIER
            && parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_NAMESPACE_RESOLUTION)
        {
            namespace_append(namespace, parser_current_token(parser)->identifier);

            parser_skip(parser);
            parser_skip(parser);
        }

        function_call->function_symbol = function_table_find_or_insert(
            function_table,
            parser_current_token(parser)->identifier, namespace);

        parser_recover(parser, &location);
    }
    else
    {
        //NOTE: For now we assume that a function-call without
        // a namespace is always from the global namespace.

        Namespace* namespace = namespace_create("");

        function_call->function_symbol = function_table_find_or_insert(
            function_table,
            parser_current_token(parser)->identifier, namespace);
    }

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_IDENTIFIER && parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_NAMESPACE_RESOLUTION)
    {
        function_call->namespace_ref = parser_parse_namespace_resolution(parser);
        if(function_call->namespace_ref == NULL)
            return NULL;
    }

    strcpy(function_call->name, parser_current_token(parser)->identifier);

    if(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected identifier for function-call's name!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_PARANTHESIS))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected '(' after function-call's name!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    list_init(&function_call->arguments, FRX_MEMORY_CATEGORY_AST);

    while(parser_current_token(parser)->type != FRX_TOKEN_TYPE_RIGHT_PARANTHESIS)
    {
        AST* argument = parser_parse_expression(parser);
        if(argument == NULL)
            return NULL;

        list_push(&function_call->arguments, argument);

        if(parser_current_token(parser)->type != FRX_TOKEN_TYPE_COMMA)
            break;

        parser_skip(parser);
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_PARANTHESIS))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Missing ')' at the end of function-call!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    return function_call;
}

static FRX_NO_DISCARD AST* parser_parse_expression(Parser* parser);

static FRX_NO_DISCARD AST* parser_parse_primary_expression(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_LEFT_PARANTHESIS)
    {
        parser_skip(parser);

        AST* primary_expression = parser_parse_primary_expression(parser);
        if(primary_expression == NULL)
            return NULL;

        if(parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_PARANTHESIS))
        {
            SourceLocation location = parser_current_location(parser);
            FRX_ERROR_FILE("Missing ')' after primary-expression!", parser->lexer.filepath, location.line, location.coloumn);

            return NULL;
        }

        return primary_expression;
    }

    AST* primary_expression = arena_alloc(&parser->arena, sizeof(AST));

    if(parser_next_token_is_unary_operator(parser))
    {
        primary_expression->type = FRX_AST_TYPE_UNARY_EXPRESSION;
        primary_expression->node = arena_alloc(&parser->arena, sizeof(ASTUnaryExpression));
        ASTUnaryExpression* unary_expression = primary_expression->node;

        switch(parser_current_token(parser)->type)
        {
            case FRX_TOKEN_TYPE_MINUS: unary_expression->type = FRX_AST_TYPE_ARITHMETIC_NEGATION; break;
            case FRX_TOKEN_TYPE_LOGICAL_NEGATION: unary_expression->type = FRX_AST_TYPE_LOGICAL_NEGATION; break;
            case FRX_TOKEN_TYPE_BINARY_NEGATION: unary_expression->type = FRX_AST_TYPE_BINARY_NEGATION; break;

            case FRX_TOKEN_TYPE_STAR: unary_expression->type = FRX_AST_TYPE_DEREFERENCE; break;
            case FRX_TOKEN_TYPE_BINARY_AND: unary_expression->type = FRX_AST_TYPE_ADDRESS_OF; break;

            default:
            {
                FRX_ASSERT(FRX_FALSE); //Since all possible unary operators should be handled in a switch case,
                                       //we should never end up in this default case.

                break;
            }
        }

        parser_skip(parser);

        if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_LEFT_PARANTHESIS)
        {
            unary_expression->operand = parser_parse_expression(parser);
            if(unary_expression->operand == NULL)
                return NULL;

            return primary_expression;
        }

        unary_expression->operand = parser_parse_primary_expression(parser);
        if(unary_expression->operand == NULL)
            return NULL;

        return primary_expression;
    }

    switch(parser_current_token(parser)->type)
    {
        case FRX_TOKEN_TYPE_IDENTIFIER:
        {
            if(parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_NAMESPACE_RESOLUTION)
            {
                primary_expression->type = FRX_AST_TYPE_FUNCTION_CALL;
                primary_expression->node = parser_parse_function_call(parser);
                if(primary_expression->node == NULL)
                    return NULL;

                return primary_expression;
            }

            if(parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_LEFT_PARANTHESIS)
            {
                primary_expression->type = FRX_AST_TYPE_FUNCTION_CALL;
                primary_expression->node = parser_parse_function_call(parser);
                if(primary_expression->node == NULL)
                    return NULL;

                return primary_expression;
            }

            primary_expression->type = FRX_AST_TYPE_VARIABLE;
            primary_expression->node = parser_parse_variable(parser);
            if(primary_expression->node == NULL)
                return NULL;

            return primary_expression;
        }

        case FRX_TOKEN_TYPE_KW_SIZEOF:
        {
            primary_expression->type = FRX_AST_TYPE_SIZEOF;
            primary_expression->node = parser_parse_sizeof(parser);
            if(primary_expression->node == NULL)
                return NULL;

            return primary_expression;
        }

        case FRX_TOKEN_TYPE_KW_NULLPTR:
        case FRX_TOKEN_TYPE_KW_TRUE:
        case FRX_TOKEN_TYPE_KW_FALSE:
        case FRX_TOKEN_TYPE_NUMBER:
        {
            primary_expression->type = FRX_AST_TYPE_NUMBER;
            primary_expression->node = parser_parse_number(parser);
            if(primary_expression->node == NULL)
                return NULL;

            return primary_expression;
        }

        case FRX_TOKEN_TYPE_CHAR_LITERAL:
        {
            primary_expression->type = FRX_AST_TYPE_CHAR_LITERAL;
            primary_expression->node = parser_parse_char_literal(parser);
            if(primary_expression->node == NULL)
                return NULL;

            return primary_expression;
        }

        case FRX_TOKEN_TYPE_STRING_LITERAL:
        {
            primary_expression->type = FRX_AST_TYPE_STRING_LITERAL;
            primary_expression->node = parser_parse_string_literal(parser);
            if(primary_expression->node == NULL)
                return NULL;

            return primary_expression;
        }
    }

    SourceLocation location = parser_current_location(parser);
    FRX_ERROR_FILE("Could not parse token %s, expected namespace-resolution, function-call, variable, number, char-literal or string-literal!", parser->lexer.filepath, location.line, location.coloumn, token_type_to_str(parser_current_token(parser)->type));

    return NULL;
}

static FRX_NO_DISCARD AST* parser_parse_expression(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    AST* expression = NULL;
    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_LEFT_PARANTHESIS)
    {
        parser_skip(parser);

        expression = parser_parse_expression(parser);

        if(expression->type == FRX_AST_TYPE_BINARY_EXPRESSION)
            ((ASTBinaryExpression*)expression->node)->had_paranthesis = FRX_TRUE;

        if(parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_PARANTHESIS))
        {
            SourceLocation location = parser_current_location(parser);
            FRX_ERROR_FILE("Missing ')' after expression!", parser->lexer.filepath, location.line, location.coloumn);

            return NULL;
        }
    }
    else
        expression = parser_parse_primary_expression(parser);

    if(parser_next_token_is_binary_operator(parser))
    {
        AST* temp = expression;
        expression = arena_alloc(&parser->arena, sizeof(AST));
        expression->type = FRX_AST_TYPE_BINARY_EXPRESSION;
        expression->node = arena_alloc(&parser->arena, sizeof(ASTBinaryExpression));
        ASTBinaryExpression* binary_expression = expression->node;
        binary_expression->had_paranthesis = FRX_FALSE;
        binary_expression->left = temp;

        switch(parser_current_token(parser)->type)
        {
            case FRX_TOKEN_TYPE_EQUALS: binary_expression->type = FRX_AST_TYPE_VARIABLE_ASSIGNMENT; break;

            case FRX_TOKEN_TYPE_PLUS: binary_expression->type = FRX_AST_TYPE_ADDITION; break;
            case FRX_TOKEN_TYPE_MINUS: binary_expression->type = FRX_AST_TYPE_SUBTRACTION; break;
            case FRX_TOKEN_TYPE_STAR: binary_expression->type = FRX_AST_TYPE_MULTIPLICATION; break;
            case FRX_TOKEN_TYPE_SLASH: binary_expression->type = FRX_AST_TYPE_DIVISION; break;
            case FRX_TOKEN_TYPE_MODULO: binary_expression->type = FRX_AST_TYPE_MODULO; break;

            case FRX_TOKEN_TYPE_LOGICAL_AND: binary_expression->type = FRX_AST_TYPE_LOGICAL_AND; break;
            case FRX_TOKEN_TYPE_LOGICAL_OR: binary_expression->type = FRX_AST_TYPE_LOGICAL_OR; break;

            case FRX_TOKEN_TYPE_BINARY_AND: binary_expression->type = FRX_AST_TYPE_BINARY_AND; break;
            case FRX_TOKEN_TYPE_BINARY_OR: binary_expression->type = FRX_AST_TYPE_BINARY_OR; break;
            case FRX_TOKEN_TYPE_BINARY_XOR: binary_expression->type = FRX_AST_TYPE_BINARY_XOR; break;
            case FRX_TOKEN_TYPE_BINARY_LEFT_SHIFT: binary_expression->type = FRX_AST_TYPE_BINARY_LEFT_SHIFT; break;
            case FRX_TOKEN_TYPE_BINARY_RIGHT_SHIFT: binary_expression->type = FRX_AST_TYPE_BINARY_RIGHT_SHIFT; break;

            case FRX_TOKEN_TYPE_NOT_EQUALS: binary_expression->type = FRX_AST_TYPE_NEGATED_COMPARISON; break;
            case FRX_TOKEN_TYPE_COMPARISON: binary_expression->type = FRX_AST_TYPE_COMPARISON; break;

            case FRX_TOKEN_TYPE_GREATER_THAN: binary_expression->type = FRX_AST_TYPE_GREATER_THAN; break;
            case FRX_TOKEN_TYPE_GREATER_THAN_EQUALS: binary_expression->type = FRX_AST_TYPE_GREATER_THAN_EQUALS; break;
            case FRX_TOKEN_TYPE_LESS_THAN: binary_expression->type = FRX_AST_TYPE_LESS_THAN; break;
            case FRX_TOKEN_TYPE_LESS_THAN_EQUALS: binary_expression->type = FRX_AST_TYPE_LESS_THAN_EQUALS; break;

            default:
            {
                FRX_ASSERT(FRX_FALSE); //Since we only stay in the while loop as long as the next token is a binary operator,
                                       //we should never end up in this default case.

                break;
            }
        }

        parser_skip(parser);

        binary_expression->right = parser_parse_expression(parser);
        ASTType binary_expr_right_type = binary_expression->right->type;
        if(binary_expr_right_type == FRX_AST_TYPE_BINARY_EXPRESSION)
            binary_expr_right_type = ((ASTBinaryExpression*)binary_expression->right->node)->type;

        if(parser_get_precedence(binary_expression->type) >= parser_get_precedence(binary_expr_right_type))
        {
            AST* lower_expr_wrapper = binary_expression->right;
            ASTBinaryExpression* lower_expr = (ASTBinaryExpression*)lower_expr_wrapper->node;

            AST* left_most_child_wrapper = lower_expr_wrapper;
            ASTBinaryExpression* left_most_child = left_most_child_wrapper->node;

            ASTBinaryExpression* left_most_child_parent = NULL;

            b8 has_higher_precedence_than_subtree = FRX_TRUE;
            while(left_most_child->left->type == FRX_AST_TYPE_BINARY_EXPRESSION)
            {
                left_most_child_wrapper = left_most_child->left;
                left_most_child_parent = left_most_child;
                left_most_child = left_most_child_wrapper->node;

                if(parser_get_precedence(left_most_child->type) > parser_get_precedence(binary_expression->type)
                    || left_most_child->had_paranthesis)
                {
                    has_higher_precedence_than_subtree = FRX_FALSE;
                    break;
                }
            }

            if(has_higher_precedence_than_subtree)
            {
                binary_expression->right = left_most_child->left;
                left_most_child->left = expression;
                expression = lower_expr_wrapper;
            }
            else if(left_most_child->had_paranthesis && left_most_child_parent != NULL)
            {
                binary_expression->right = left_most_child_wrapper;
                left_most_child_parent->left = expression;
                expression = lower_expr_wrapper;
            }
        }
    }

    return expression;
}

//Old iterative approach on parsing expressions, but it does not work properly.
/*
static FRX_NO_DISCARD AST* _parser_parse_expression(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    AST* expression = NULL;

    b8 node_had_paranthesis = FRX_FALSE;

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_LEFT_PARANTHESIS)
    {
        node_had_paranthesis = FRX_TRUE;

        parser_skip(parser);

        expression = parser_parse_expression(parser);
        if(expression == NULL)
            return NULL;

        if(parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_PARANTHESIS))
        {
            SourceLocation location = parser_current_location(parser);
            FRX_ERROR_FILE("Missing ')' after expression!", parser->lexer.filepath, location.line, location.coloumn);

            return NULL;
        }
    }
    else
    {
        expression = parser_parse_primary_expression(parser);
        if(expression == NULL)
            return NULL;
    }

    while(parser_next_token_is_binary_operator(parser))
    {
        AST* new_node = arena_alloc(&parser->arena, sizeof(AST));
        new_node->type = FRX_AST_TYPE_BINARY_EXPRESSION;
        new_node->node = arena_alloc(&parser->arena, sizeof(ASTBinaryExpression));
        ASTBinaryExpression* binary_expression = new_node->node;

        switch(parser_current_token(parser)->type)
        {
            case FRX_TOKEN_TYPE_EQUALS: binary_expression->type = FRX_AST_TYPE_VARIABLE_ASSIGNMENT; break;

            case FRX_TOKEN_TYPE_PLUS: binary_expression->type = FRX_AST_TYPE_ADDITION; break;
            case FRX_TOKEN_TYPE_MINUS: binary_expression->type = FRX_AST_TYPE_SUBTRACTION; break;
            case FRX_TOKEN_TYPE_STAR: binary_expression->type = FRX_AST_TYPE_MULTIPLICATION; break;
            case FRX_TOKEN_TYPE_SLASH: binary_expression->type = FRX_AST_TYPE_DIVISION; break;
            case FRX_TOKEN_TYPE_MODULO: binary_expression->type = FRX_AST_TYPE_MODULO; break;

            case FRX_TOKEN_TYPE_LOGICAL_AND: binary_expression->type = FRX_AST_TYPE_LOGICAL_AND; break;
            case FRX_TOKEN_TYPE_LOGICAL_OR: binary_expression->type = FRX_AST_TYPE_LOGICAL_OR; break;

            case FRX_TOKEN_TYPE_BINARY_AND: binary_expression->type = FRX_AST_TYPE_BINARY_AND; break;
            case FRX_TOKEN_TYPE_BINARY_OR: binary_expression->type = FRX_AST_TYPE_BINARY_OR; break;
            case FRX_TOKEN_TYPE_BINARY_XOR: binary_expression->type = FRX_AST_TYPE_BINARY_XOR; break;
            case FRX_TOKEN_TYPE_BINARY_LEFT_SHIFT: binary_expression->type = FRX_AST_TYPE_BINARY_LEFT_SHIFT; break;
            case FRX_TOKEN_TYPE_BINARY_RIGHT_SHIFT: binary_expression->type = FRX_AST_TYPE_BINARY_RIGHT_SHIFT; break;

            case FRX_TOKEN_TYPE_NOT_EQUALS: binary_expression->type = FRX_AST_TYPE_NEGATED_COMPARISON; break;
            case FRX_TOKEN_TYPE_COMPARISON: binary_expression->type = FRX_AST_TYPE_COMPARISON; break;

            case FRX_TOKEN_TYPE_GREATER_THAN: binary_expression->type = FRX_AST_TYPE_GREATER_THAN; break;
            case FRX_TOKEN_TYPE_GREATER_THAN_EQUALS: binary_expression->type = FRX_AST_TYPE_GREATER_THAN_EQUALS; break;
            case FRX_TOKEN_TYPE_LESS_THAN: binary_expression->type = FRX_AST_TYPE_LESS_THAN; break;
            case FRX_TOKEN_TYPE_LESS_THAN_EQUALS: binary_expression->type = FRX_AST_TYPE_LESS_THAN_EQUALS; break;

            default:
            {
                FRX_ASSERT(FRX_FALSE); //Since we only stay in the while loop as long as the next token is a binary operator,
                                       //we should never end up in this default case.

                break;
            }
        }

        parser_skip(parser);

        ASTType expression_type = expression->type;
        if(expression_type == FRX_AST_TYPE_BINARY_EXPRESSION)
            expression_type = ((ASTBinaryExpression*)(expression->node))->type;
        else if(expression_type == FRX_AST_TYPE_UNARY_EXPRESSION)
            expression_type = ((ASTUnaryExpression*)(expression->node))->type;

        if(parser_get_precedence(binary_expression->type) > parser_get_precedence(expression_type) && !node_had_paranthesis)
        {
            AST* temp = ((ASTBinaryExpression*)expression->node)->right;

            ((ASTBinaryExpression*)expression->node)->right = new_node;

            binary_expression->left = temp;

            if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_LEFT_PARANTHESIS)
            {
                parser_skip(parser);

                binary_expression->right = parser_parse_expression(parser);
                if(binary_expression->right == NULL)
                    return NULL;

                if(parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_PARANTHESIS))
                {
                    SourceLocation location = parser_current_location(parser);
                    FRX_ERROR_FILE("Missing ')' after expression!", parser->lexer.filepath, location.line, location.coloumn);

                    return NULL;
                }
            }
            else
            {
                binary_expression->right = parser_parse_primary_expression(parser);
                if(binary_expression->right == NULL)
                    return NULL;
            }
        }
        else
        {
            AST* temp = expression;

            expression = new_node;

            ((ASTBinaryExpression*)expression->node)->left = temp;

            if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_LEFT_PARANTHESIS)
            {
                parser_skip(parser);

                ((ASTBinaryExpression*)expression->node)->right = parser_parse_expression(parser);
                if(((ASTBinaryExpression*)expression->node)->right == NULL)
                    return NULL;

                if(parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_PARANTHESIS))
                {
                    SourceLocation location = parser_current_location(parser);
                    FRX_ERROR_FILE("Missing ')' after expression!", parser->lexer.filepath, location.line, location.coloumn);

                    return NULL;
                }
            }
            else
            {
                ((ASTBinaryExpression*)expression->node)->right = parser_parse_primary_expression(parser);
                if(((ASTBinaryExpression*)expression->node)->right == NULL)
                    return NULL;
            }
        }
    }

    return expression;
}
*/

static FRX_NO_DISCARD ASTTypename* parser_parse_type(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    ASTTypename* type = arena_alloc(&parser->arena, sizeof(ASTTypename));

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_IDENTIFIER && parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_NAMESPACE_RESOLUTION)
    {
        type->namespace_ref = parser_parse_namespace_resolution(parser);
        if(type->namespace_ref == NULL)
            return NULL;
    }
    else
        type->namespace_ref = NULL;

    strcpy(type->name, parser_current_token(parser)->identifier);

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_IDENTIFIER
        || is_primitive(parser_current_token(parser)->type))
    {
        parser_skip(parser);
    }
    else
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected identifier for the type!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    type->pointer_level = 0;
    while(parser_current_token(parser)->type == FRX_TOKEN_TYPE_STAR)
    {
        parser_skip(parser);

        ++type->pointer_level;
    }

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_LEFT_BRACKET)
    {
        parser_skip(parser);

        type->array_size = parser_parse_expression(parser);

        if(parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_BRACKET))
        {
            SourceLocation location = parser_current_location(parser);
            FRX_ERROR_FILE("Missing ']' after array declaration!", parser->lexer.filepath, location.line, location.coloumn);

            return NULL;
        }
    }

    return type;
}

static FRX_NO_DISCARD b8 is_variable_declaration(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    SourceLocation location = parser_current_location(parser);

    parser_skip_namespace_resolution(parser);

    b8 result = (is_primitive(parser_current_token(parser)->type) || parser_current_token(parser)->type == FRX_TOKEN_TYPE_IDENTIFIER) && parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_IDENTIFIER
        && parser_peek(parser, 2)->type == FRX_TOKEN_TYPE_SEMICOLON;

    if(!result)
    {
        if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_IDENTIFIER
            || is_primitive(parser_current_token(parser)->type))
        {
            parser_skip(parser);

            while(parser_current_token(parser)->type == FRX_TOKEN_TYPE_STAR)
                parser_skip(parser);

            if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_LEFT_BRACKET)
            {
                while(parser_current_token(parser)->type != FRX_TOKEN_TYPE_RIGHT_BRACKET)
                    parser_skip(parser);

                parser_skip(parser);
            }
        }

        result = parser_current_token(parser)->type == FRX_TOKEN_TYPE_IDENTIFIER && parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_SEMICOLON;
    }

    parser_recover(parser, &location);

    return result;
}

static FRX_NO_DISCARD VariableSymbol* parser_generate_variable_symbol(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    VariableTable* variable_table = get_global_variable_table();
    StructTable* struct_table = get_global_struct_table();

    VariableSymbol* variable_symbol = NULL;

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_IDENTIFIER
            && parser_peek(parser, 1)->type ==
            FRX_TOKEN_TYPE_NAMESPACE_RESOLUTION)
    {
        Namespace* namespace = namespace_create(
                parser_current_token(parser)->identifier);

        parser_skip(parser);
        parser_skip(parser);

        while(parser_current_token(parser)->type == FRX_TOKEN_TYPE_IDENTIFIER
                && parser_peek(parser, 1)->type ==
                FRX_TOKEN_TYPE_NAMESPACE_RESOLUTION)
        {
            namespace_append(namespace,
                    parser_current_token(parser)->identifier);

            parser_skip(parser);
            parser_skip(parser);
        }

        const StructSymbol* struct_symbol =
            struct_table_find_or_insert(struct_table,
                    parser_current_token(parser)->identifier, namespace);

        if(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER))
        {
            SourceLocation location = parser_current_location(parser);
            FRX_ERROR_FILE("Expected identifier for the variable's type!",
                    parser->lexer.filepath, location.line, location.coloumn);

            return NULL;
        }

        while(parser_current_token(parser)->type == FRX_TOKEN_TYPE_STAR)
            parser_skip(parser);

        if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_LEFT_BRACKET)
        {
            parser_skip(parser);

            while(parser_current_token(parser)->type
                    != FRX_TOKEN_TYPE_RIGHT_BRACKET)
                parser_skip(parser);

            parser_skip(parser);
        }

        variable_symbol = variable_table_insert(variable_table,
            parser_current_token(parser)->identifier, NULL);

        parser_skip(parser);

        variable_symbol->type_category = FRX_TYPE_CATEGORY_STRUCT;
        variable_symbol->struct_symbol = struct_symbol;
    }
    else
    {
        //TODO: Look recurcively with the parser's current namespace
        //for a valid declaration. drop one namespace each iteration
        //if no declaration is found, until the global scope is reached.

        b8 primitive = is_primitive(parser_current_token(parser)->type);

        const StructSymbol* struct_symbol =
            primitive ? NULL : struct_table_find_or_insert(struct_table,
                parser_current_token(parser)->identifier,
                parser->current_namespace);

        if(primitive)
        {
            parser_skip(parser);
        }
        else
        {
            if(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER))
            {
                SourceLocation location = parser_current_location(parser);
                FRX_ERROR_FILE("Expected identifier for the variable's type!",
                               parser->lexer.filepath, location.line,
                               location.coloumn);

                return NULL;
            }
        }

        while(parser_current_token(parser)->type == FRX_TOKEN_TYPE_STAR)
            parser_skip(parser);

        if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_LEFT_BRACKET)
        {
            parser_skip(parser);

            while(parser_current_token(parser)->type
                    != FRX_TOKEN_TYPE_RIGHT_BRACKET)
                parser_skip(parser);

            parser_skip(parser);
        }

        variable_symbol = variable_table_insert(variable_table,
            parser_current_token(parser)->identifier, NULL);

        if(primitive)
        {
            variable_symbol->type_category = FRX_TYPE_CATEGORY_PRIMITIVE;
            variable_symbol->primitive_type = parser_current_token(parser)->type;
        }
        else
        {
            variable_symbol->type_category = FRX_TYPE_CATEGORY_STRUCT;
            variable_symbol->struct_symbol = struct_symbol;
        }

        parser_skip(parser);
    }

    return variable_symbol;
}

static FRX_NO_DISCARD ASTVariableDeclaration* parser_parse_variable_declaration(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    ASTVariableDeclaration* variable_declaration = arena_alloc(&parser->arena, sizeof(ASTVariableDeclaration));

    SourceLocation location = parser_current_location(parser);
    variable_declaration->variable_symbol = parser_generate_variable_symbol(parser);
    parser_recover(parser, &location);

    variable_declaration->type = parser_parse_type(parser);
    if(variable_declaration->type == NULL)
        return NULL;

    strcpy(variable_declaration->name, parser_current_token(parser)->identifier);

    if(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected identifier for the variable's name!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_SEMICOLON))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Missing ';' after variable-declaration!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    return variable_declaration;
}

static FRX_NO_DISCARD b8 is_variable_definition(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    SourceLocation location = parser_current_location(parser);

    parser_skip_namespace_resolution(parser);

    b8 result = (is_primitive(parser_current_token(parser)->type) || parser_current_token(parser)->type == FRX_TOKEN_TYPE_IDENTIFIER) && parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_IDENTIFIER
        && parser_peek(parser, 2)->type == FRX_TOKEN_TYPE_EQUALS;

    if(!result)
    {
        if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_IDENTIFIER
            || is_primitive(parser_current_token(parser)->type))
        {
            parser_skip(parser);

            while(parser_current_token(parser)->type == FRX_TOKEN_TYPE_STAR)
                parser_skip(parser);

            if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_LEFT_BRACKET)
            {
                while(parser_current_token(parser)->type != FRX_TOKEN_TYPE_RIGHT_BRACKET)
                    parser_skip(parser);

                parser_skip(parser);
            }
        }

        result = parser_current_token(parser)->type == FRX_TOKEN_TYPE_IDENTIFIER && parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_EQUALS;
    }

    parser_recover(parser, &location);

    return result;
}

static FRX_NO_DISCARD ASTVariableDefinition* parser_parse_variable_definition(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    ASTVariableDefinition* variable_definition = arena_alloc(&parser->arena, sizeof(ASTVariableDefinition));

    SourceLocation location = parser_current_location(parser);
    variable_definition->variable_symbol = parser_generate_variable_symbol(parser);
    parser_recover(parser, &location);

    variable_definition->type = parser_parse_type(parser);
    if(variable_definition->type == NULL)
        return NULL;

    strcpy(variable_definition->name, parser_current_token(parser)->identifier);

    if(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected identifier for the variable's name!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_EQUALS))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected '=' before expression!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_LEFT_BRACE)
    {
        parser_skip(parser);

        variable_definition->value = NULL;

        list_init(&variable_definition->array_initialization,
                  FRX_MEMORY_CATEGORY_AST);

        while(parser_current_token(parser)->type
            != FRX_TOKEN_TYPE_RIGHT_BRACE)
        {
            AST* array_entry = parser_parse_expression(parser);
            list_push(&variable_definition->array_initialization, array_entry);

            if(parser_current_token(parser)->type != FRX_TOKEN_TYPE_RIGHT_BRACE
                && parser_eat(parser, FRX_TOKEN_TYPE_COMMA))
            {
                SourceLocation location = parser_current_location(parser);
                FRX_ERROR_FILE("Missing ',' after array-entry!", parser->lexer.filepath, location.line, location.coloumn);

                return NULL;
            }
        }

        parser_skip(parser);
    }
    else
    {
        variable_definition->value = parser_parse_expression(parser);
        if(variable_definition->value == NULL)
            return NULL;
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_SEMICOLON))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Missing ';' after variable-definition!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    return variable_definition;
}

static FRX_NO_DISCARD b8 is_variable_assignment(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    TokenType next_type = parser_peek(parser, 1)->type;

    return parser_current_token(parser)->type == FRX_TOKEN_TYPE_IDENTIFIER
        && (next_type == FRX_TOKEN_TYPE_DOT || next_type == FRX_TOKEN_TYPE_ARROW || next_type == FRX_TOKEN_TYPE_EQUALS);
}

static FRX_NO_DISCARD ASTVariableAssignment* parser_parse_variable_assignment(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    ASTVariableAssignment* variable_assignment = arena_alloc(&parser->arena, sizeof(ASTVariableAssignment));

    variable_assignment->variable = parser_parse_variable(parser);
    if(variable_assignment->variable == NULL)
        return NULL;

    if(parser_eat(parser, FRX_TOKEN_TYPE_EQUALS))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected '=' before expression!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    variable_assignment->value = parser_parse_expression(parser);
    if(variable_assignment->value == NULL)
        return NULL;

    return variable_assignment;
}

static FRX_NO_DISCARD b8 is_if_statement(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    return parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_IF;
}

static FRX_NO_DISCARD ASTIfStatement* parser_parse_if_statement(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    if(parser_eat(parser, FRX_TOKEN_TYPE_KW_IF))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected keyword 'if'!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_PARANTHESIS))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected '(' after keyword 'if'!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    ASTIfStatement* if_statement = arena_alloc(&parser->arena, sizeof(ASTIfStatement));

    if_statement->condition = parser_parse_expression(parser);
    if(if_statement->condition == NULL)
        return NULL;

    if(parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_PARANTHESIS))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected ')' after if-statement!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    if_statement->if_block = parser_parse_scope(parser);
    if(if_statement->if_block == NULL)
        return NULL;

    list_init(&if_statement->else_if_blocks, FRX_MEMORY_CATEGORY_AST);

    while(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_ELSE
        && parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_KW_IF)
    {
        parser_skip(parser);
        parser_skip(parser);

        ASTElseIfBlock* else_if_block = arena_alloc(&parser->arena, sizeof(ASTElseIfBlock));

        if(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_PARANTHESIS))
        {
            SourceLocation location = parser_current_location(parser);
            FRX_ERROR_FILE("Expected '('!", parser->lexer.filepath, location.line, location.coloumn);

            return NULL;
        }

        else_if_block->condition = parser_parse_expression(parser);

        if(parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_PARANTHESIS))
        {
            SourceLocation location = parser_current_location(parser);
            FRX_ERROR_FILE("Expected ')'!", parser->lexer.filepath, location.line, location.coloumn);

            return NULL;
        }

        else_if_block->block = parser_parse_scope(parser);

        list_push(&if_statement->else_if_blocks, else_if_block);
    }

    if(parser_current_token(parser)->type != FRX_TOKEN_TYPE_KW_ELSE)
    {
        if_statement->else_block = NULL;
        return if_statement;
    }

    parser_skip(parser);

    if_statement->else_block = parser_parse_scope(parser);
    if(if_statement->else_block == NULL)
        return NULL;

    return if_statement;
}

static FRX_NO_DISCARD ASTSwitchStatement* parser_parse_switch_statement(
    Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    if(parser_eat(parser, FRX_TOKEN_TYPE_KW_SWITCH))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected keyword 'switch'!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_PARANTHESIS))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected '(' after keyword 'switch'!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    ASTSwitchStatement* switch_statement = arena_alloc(&parser->arena, sizeof(ASTSwitchStatement));

    switch_statement->switch_value = parser_parse_expression(parser);
    if(switch_statement->switch_value == NULL)
        return NULL;

    if(parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_PARANTHESIS))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected ')' after expression!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_BRACE))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected '{' after switch-value!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    list_init(&switch_statement->cases, FRX_MEMORY_CATEGORY_AST);

    while(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_CASE)
    {
        parser_skip(parser);

        ASTSwitchCase* switch_case = arena_alloc(&parser->arena, sizeof(ASTSwitchCase));
        switch_case->case_expr = parser_parse_expression(parser);
        if(switch_case->case_expr == NULL)
            return NULL;

        if(parser_eat(parser, FRX_TOKEN_TYPE_COLON))
        {
            SourceLocation location = parser_current_location(parser);
            FRX_ERROR_FILE("Expected ':' after switch-case-expression!", parser->lexer.filepath, location.line, location.coloumn);

            return NULL;
        }

        switch_case->scope = parser_parse_scope(parser);
        if(switch_case->scope == NULL)
            return NULL;

        list_push(&switch_statement->cases, switch_case);
    }

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_DEFAULT)
    {
        parser_skip(parser);

        if(parser_eat(parser, FRX_TOKEN_TYPE_COLON))
        {
            SourceLocation location = parser_current_location(parser);
            FRX_ERROR_FILE("Expected ':' after keyword 'default'!", parser->lexer.filepath, location.line, location.coloumn);

            return NULL;
        }

        switch_statement->default_case = parser_parse_scope(parser);
        if(switch_statement->default_case == NULL)
            return NULL;
    }
    else
    {
        switch_statement->default_case = NULL;
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_BRACE))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected '}' at the end of switch-statement!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    return switch_statement;
}

static FRX_NO_DISCARD ASTBreakStatement* parser_parse_break_statement(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    if(parser_eat(parser, FRX_TOKEN_TYPE_KW_BREAK))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected keyword 'break'!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_SEMICOLON))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected ';' at the end of break-statement!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    ASTBreakStatement* break_statement = arena_alloc(&parser->arena, sizeof(ASTBreakStatement));

    return break_statement;
}

static FRX_NO_DISCARD ASTContinueStatement* parser_parse_continue_statement(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    if(parser_eat(parser, FRX_TOKEN_TYPE_KW_CONTINUE))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected keyword 'continue'!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_SEMICOLON))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected ';' at the end of continue-statement!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    ASTContinueStatement* continue_statement = arena_alloc(&parser->arena, sizeof(ASTContinueStatement));

    return continue_statement;
}

static FRX_NO_DISCARD b8 is_for_loop(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    return parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_FOR;
}

static FRX_NO_DISCARD ASTForLoop* parser_parse_for_loop(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    if(parser_eat(parser, FRX_TOKEN_TYPE_KW_FOR))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected keyword 'for'!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_PARANTHESIS))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected '(' after keyword 'for'!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    ASTForLoop* for_loop = arena_alloc(&parser->arena, sizeof(ASTForLoop));

    for_loop->expression = parser_parse_expression(parser);
    if(for_loop->expression == NULL)
        return NULL;

    if(parser_eat(parser, FRX_TOKEN_TYPE_SEMICOLON))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected ';' after for-loop expression!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    for_loop->condition = parser_parse_expression(parser);
    if(for_loop->condition == NULL)
        return NULL;

    if(parser_eat(parser, FRX_TOKEN_TYPE_SEMICOLON))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected ';' after for-loop condition!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    for_loop->increment = parser_parse_expression(parser);
    if(for_loop->increment == NULL)
        return NULL;

    if(parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_PARANTHESIS))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected ')' after for-loop increment!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    for_loop->scope = parser_parse_scope(parser);
    if(for_loop->scope == NULL)
        return NULL;

    return for_loop;
}

static FRX_NO_DISCARD b8 is_while_loop(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    return parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_WHILE;
}

static FRX_NO_DISCARD ASTWhileLoop* parser_parse_while_loop(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    if(parser_eat(parser, FRX_TOKEN_TYPE_KW_WHILE))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected keyword 'while'!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_PARANTHESIS))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected '(' after keyword 'while'!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    ASTWhileLoop* while_loop = arena_alloc(&parser->arena, sizeof(ASTWhileLoop));

    while_loop->condition = parser_parse_expression(parser);
    if(while_loop->condition == NULL)
        return NULL;

    if(parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_PARANTHESIS))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected ')' at the end of while-loop!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    while_loop->scope = parser_parse_scope(parser);
    if(while_loop->scope == NULL)
        return NULL;

    return while_loop;
}

static FRX_NO_DISCARD b8 is_do_while_loop(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    return parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_DO;
}

static FRX_NO_DISCARD ASTDoWhileLoop* parser_parse_do_while_loop(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    if(parser_eat(parser, FRX_TOKEN_TYPE_KW_DO))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected keyword 'do'!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    ASTDoWhileLoop* do_while_loop = arena_alloc(&parser->arena, sizeof(ASTDoWhileLoop));

    do_while_loop->scope = parser_parse_scope(parser);
    if(do_while_loop->scope == NULL)
        return NULL;

    if(parser_eat(parser, FRX_TOKEN_TYPE_KW_WHILE))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected keyword 'while'!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_PARANTHESIS))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected '(' after keyword 'while'!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    do_while_loop->condition = parser_parse_expression(parser);
    if(do_while_loop->condition == NULL)
        return NULL;

    if(parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_PARANTHESIS))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected ')' at the end of do-while-loop!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_SEMICOLON))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Missing ';' at the end of do-while-loop!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    return do_while_loop;
}

static FRX_NO_DISCARD b8 is_return_statement(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    return parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_RETURN;
}

static FRX_NO_DISCARD ASTReturnStatement* parser_parse_return_statement(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    if(parser_eat(parser, FRX_TOKEN_TYPE_KW_RETURN))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected keyword 'return'!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    ASTReturnStatement* return_statement = arena_alloc(&parser->arena, sizeof(ASTReturnStatement));

    if(parser_current_token(parser)->type != FRX_TOKEN_TYPE_SEMICOLON)
    {
        return_statement->value = parser_parse_expression(parser);
        if(return_statement->value == NULL)
            return NULL;
    }
    else
        return_statement->value = NULL;

    if(parser_eat(parser, FRX_TOKEN_TYPE_SEMICOLON))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Missing ';' at the end of return-statement!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    return return_statement;
}

static FRX_NO_DISCARD AST* parser_parse_statement(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    AST* ast = arena_alloc(&parser->arena, sizeof(AST));

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_ASSERT)
    {
        ast->type = FRX_AST_TYPE_ASSERT;
        ast->node = parser_parse_assert(parser);
        if(ast->node == NULL)
            return NULL;

        return ast;
    }

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_IF)
    {
        ast->type = FRX_AST_TYPE_IF_STATEMENT;
        ast->node = parser_parse_if_statement(parser);
        if(ast->node == NULL)
            return NULL;

        return ast;
    }

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_SWITCH)
    {
        ast->type = FRX_AST_TYPE_SWITCH_STATEMENT;
        ast->node = parser_parse_switch_statement(parser);
        if(ast->node == NULL)
            return NULL;

        return ast;
    }

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_BREAK)
    {
        ast->type = FRX_AST_TYPE_BREAK_STATEMENT;
        ast->node = parser_parse_break_statement(parser);
        if(ast->node == NULL)
            return NULL;

        return ast;
    }

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_CONTINUE)
    {
        ast->type = FRX_AST_TYPE_CONTINUE_STATEMENT;
        ast->node = parser_parse_continue_statement(parser);
        if(ast->node == NULL)
            return NULL;

        return ast;
    }

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_FOR)
    {
        ast->type = FRX_AST_TYPE_FOR_LOOP;
        ast->node = parser_parse_for_loop(parser);
        if(ast->node == NULL)
            return NULL;

        return ast;
    }

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_WHILE)
    {
        ast->type = FRX_AST_TYPE_WHILE_LOOP;
        ast->node = parser_parse_while_loop(parser);
        if(ast->node == NULL)
            return NULL;

        return ast;
    }

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_DO)
    {
        ast->type = FRX_AST_TYPE_DO_WHILE_LOOP;
        ast->node = parser_parse_do_while_loop(parser);
        if(ast->node == NULL)
            return NULL;

        return ast;
    }

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_RETURN)
    {
        ast->type = FRX_AST_TYPE_RETURN_STATEMENT;
        ast->node = parser_parse_return_statement(parser);
        if(ast->node == NULL)
            return NULL;

        return ast;
    }

    if(is_function_call(parser))
    {
        ast->type = FRX_AST_TYPE_FUNCTION_CALL;
        ast->node = parser_parse_function_call(parser);
        if(ast->node == NULL)
            return NULL;

        if(parser_eat(parser, FRX_TOKEN_TYPE_SEMICOLON))
        {
            SourceLocation location = parser_current_location(parser);
            FRX_ERROR_FILE("Missing ';' at the end of function-call!", parser->lexer.filepath, location.line, location.coloumn);

            return NULL;
        }

        return ast;
    }

    if(is_variable_declaration(parser))
    {
        ast->type = FRX_AST_TYPE_VARIABLE_DECLARATION;
        ast->node = parser_parse_variable_declaration(parser);
        if(ast->node == NULL)
            return NULL;

        return ast;
    }

    if(is_variable_definition(parser))
    {
        ast->type = FRX_AST_TYPE_VARIABLE_DEFINITION;
        ast->node = parser_parse_variable_definition(parser);
        if(ast->node == NULL)
            return NULL;

        return ast;
    }

    if(is_variable_assignment(parser))
    {
        ast->type = FRX_AST_TYPE_VARIABLE_ASSIGNMENT;
        ast->node = parser_parse_variable_assignment(parser);
        if(ast->node == NULL)
            return NULL;

        if(parser_eat(parser, FRX_TOKEN_TYPE_SEMICOLON))
        {
            SourceLocation location = parser_current_location(parser);
            FRX_ERROR_FILE("Missing ';' after variable-assignment!", parser->lexer.filepath, location.line, location.coloumn);

            return NULL;
        }

        return ast;
    }

    //TODO: If no rule applied so far, parse this as an assignment, where the lhs of the assignment is a complex expression.
    //Ensure that the lhs is a valid lvalue and not a rvalue afterwards.

    FRX_ERROR_FILE("Could not parse token %s, expected statement!", parser->lexer.filepath, parser_current_location(parser).line, parser_current_location(parser).coloumn, token_type_to_str(parser_current_token(parser)->type));

    return NULL;
}

static FRX_NO_DISCARD ASTScope* parser_parse_scope(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    if(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_BRACE))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected '{' to start scope!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    ASTScope* scope = arena_alloc(&parser->arena, sizeof(ASTScope));

    list_init(&scope->statements, FRX_MEMORY_CATEGORY_AST);

    while(parser_current_token(parser)->type != FRX_TOKEN_TYPE_RIGHT_BRACE)
    {
        AST* statement = parser_parse_statement(parser);
        if(statement == NULL)
            return NULL;

        list_push(&scope->statements, statement);
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_BRACE))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected '}' at the end of scope!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    return scope;
}

static FRX_NO_DISCARD ASTParameterList* parser_parse_parameter_list(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    if(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_PARANTHESIS))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected '(' to start the parameter-list!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    ASTParameterList* parameter_list = arena_alloc(&parser->arena, sizeof(ASTParameterList));
    parameter_list->is_variadic = FRX_FALSE;

    list_init(&parameter_list->parameters, FRX_MEMORY_CATEGORY_AST);

    while(parser_current_token(parser)->type != FRX_TOKEN_TYPE_RIGHT_PARANTHESIS)
    {
        if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_ELLIPSIS)
        {
            parser_skip(parser);

            if(list_size(&parameter_list->parameters) == 0)
            {
                FRX_ERROR_FILE("Variadic function arguments requiere at least one named argument!", parser->lexer.filepath, parser_current_location(parser).line, parser_current_location(parser).coloumn);

               return NULL;
            }

            parameter_list->is_variadic = FRX_TRUE;

            if(parser_current_token(parser)->type != FRX_TOKEN_TYPE_RIGHT_PARANTHESIS)
            {
                FRX_ERROR_FILE("Variadic function arguments must come last in function signature!", parser->lexer.filepath, parser_current_location(parser).line, parser_current_location(parser).coloumn);

                return NULL;
            }

            break;
        }

        ASTVariableDeclaration* parameter = arena_alloc(&parser->arena, sizeof(ASTVariableDeclaration));

        parameter->type = parser_parse_type(parser);
        if(parameter->type == NULL)
            return NULL;

        strcpy(parameter->name, parser_current_token(parser)->identifier);

        if(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER))
        {
            SourceLocation location = parser_current_location(parser);
            FRX_ERROR_FILE("Expected identifier for the parameter's name!", parser->lexer.filepath, location.line, location.coloumn);

            return NULL;
        }

        if(parser_current_token(parser)->type != FRX_TOKEN_TYPE_RIGHT_PARANTHESIS)
        {
            if(parser_eat(parser, FRX_TOKEN_TYPE_COMMA))
            {
                SourceLocation location = parser_current_location(parser);
                FRX_ERROR_FILE("Missing ',' after parameter!", parser->lexer.filepath, location.line, location.coloumn);

                return NULL;
            }
        }

        list_push(&parameter_list->parameters, parameter);
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_PARANTHESIS))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Missing ')' at the end of parameter-list!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    return parameter_list;
}

static FRX_NO_DISCARD FunctionSymbol* parser_generate_function_symbol(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    FunctionTable* function_table = get_global_function_table();
    StructTable* struct_table = get_global_struct_table();

    parser_skip_optional_keywords(parser);

    FunctionSymbol* function_symbol = NULL;

    if(is_primitive(parser_current_token(parser)->type))
    {
        TokenType return_type = parser_current_token(parser)->type;

        parser_skip(parser);

        while(parser_current_token(parser)->type == FRX_TOKEN_TYPE_STAR)
            parser_skip(parser);

        function_symbol = function_table_find(function_table,
            parser_current_token(parser)->identifier,
            parser->current_namespace);

        if(function_symbol != NULL && function_symbol->defined)
        {
            SourceLocation location = parser_current_location(parser);
            FRX_ERROR_FILE("Encountered redefinition of function '%s'!",
                           parser->lexer.filepath, location.line,
                           location.coloumn, function_symbol->name);

            return NULL;
        }

        function_symbol = function_table_insert(function_table,
            parser_current_token(parser)->identifier,
            parser->current_namespace);

        if(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER))
        {
            SourceLocation location = parser_current_location(parser);
            FRX_ERROR_FILE("Expected identifier for the function-definition's name!",
                           parser->lexer.filepath, location.line,
                           location.coloumn);

            return NULL;
        }

        function_symbol->type_category = FRX_TYPE_CATEGORY_PRIMITIVE;
        function_symbol->primitive_return_type = return_type;
    }
    else
    {
        Namespace* return_type_namespace = namespace_create("");

        while(parser_current_token(parser)->type == FRX_TOKEN_TYPE_IDENTIFIER
            && parser_peek(parser, 1)->type ==
            FRX_TOKEN_TYPE_NAMESPACE_RESOLUTION)
        {
            namespace_append(return_type_namespace,
                             parser_current_token(parser)->identifier);

            parser_skip(parser);
            parser_skip(parser);
        }

        StructSymbol* struct_symbol = struct_table_find_or_insert(struct_table,
            parser_current_token(parser)->identifier, return_type_namespace);

        if(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER))
        {
            SourceLocation location = parser_current_location(parser);
            FRX_ERROR_FILE("Expected identifier for the function's return type!",
                           parser->lexer.filepath, location.line,
                           location.coloumn);

            return NULL;
        }

        while(parser_current_token(parser)->type == FRX_TOKEN_TYPE_STAR)
            parser_skip(parser);

        function_symbol = function_table_insert(function_table,
            parser_current_token(parser)->identifier,
            parser->current_namespace);

        function_symbol->type_category = FRX_TYPE_CATEGORY_STRUCT;
        function_symbol->struct_symbol_return_type = struct_symbol;

        parser_skip(parser);
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_PARANTHESIS))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected '(' to start the parameter-list!",
                       parser->lexer.filepath, location.line,
                       location.coloumn);

        return NULL;
    }

    while(parser_current_token(parser)->type != FRX_TOKEN_TYPE_RIGHT_PARANTHESIS)
    {
        //TODO: Handle function parameters

        parser_skip(parser);
    }

    parser_skip(parser);

    function_symbol->defined = FRX_TRUE;

    return function_symbol;
}

static FRX_NO_DISCARD b8 is_function_definition(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    SourceLocation location = parser_current_location(parser);

    parser_skip_optional_keywords(parser);

    parser_skip_namespace_resolution(parser);

    b8 result = is_primitive(parser_current_token(parser)->type) || parser_current_token(parser)->type == FRX_TOKEN_TYPE_IDENTIFIER;

    parser_skip(parser);

    while(parser_current_token(parser)->type == FRX_TOKEN_TYPE_STAR)
        parser_skip(parser);

    result &= parser_current_token(parser)->type == FRX_TOKEN_TYPE_IDENTIFIER
        && parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_LEFT_PARANTHESIS;

    parser_recover(parser, &location);

    return result;
}

static FRX_NO_DISCARD ASTFunctionDefinition* parser_parse_function_definition(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    ASTFunctionDefinition* function_definition = arena_alloc(&parser->arena, sizeof(ASTFunctionDefinition));
    function_definition->exported = FRX_FALSE;

    SourceLocation location = parser_current_location(parser);
    function_definition->function_symbol =
        parser_generate_function_symbol(parser);
    parser_recover(parser, &location);

    FRX_ASSERT(function_definition->function_symbol != NULL);

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_EXPORT)
    {
        function_definition->exported = FRX_TRUE;
        parser_skip(parser);
    }

    function_definition->type = parser_parse_type(parser);
    if(function_definition->type == NULL)
        return NULL;

    strcpy(function_definition->name, parser_current_token(parser)->identifier);

    if(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected identifier for the function-definition's name!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    function_definition->parameter_list = parser_parse_parameter_list(parser);
    if(function_definition->parameter_list == NULL)
        return NULL;

    function_definition->scope = parser_parse_scope(parser);
    if(function_definition->scope == NULL)
        return NULL;

    return function_definition;
}

static FRX_NO_DISCARD b8 is_function_declaration(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    return parser_current_token(parser)->type == FRX_TOKEN_TYPE_IDENTIFIER && parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_IDENTIFIER;
}

static FRX_NO_DISCARD
ASTFunctionDeclaration* parser_parse_function_declaration(Parser* parser,
                                                          b8 is_extern)
{
    FRX_ASSERT(parser != NULL);

    ASTFunctionDeclaration* function_declaration = arena_alloc(&parser->arena, sizeof(ASTFunctionDeclaration));

    SourceLocation location = parser_current_location(parser);
    function_declaration->function_symbol = parser_generate_function_symbol(
        parser);

    if(!is_extern)
        function_declaration->function_symbol->defined = FRX_FALSE;

    parser_recover(parser, &location);

    function_declaration->type = parser_parse_type(parser);
    if(function_declaration->type == NULL)
        return NULL;

    strcpy(function_declaration->name, parser_current_token(parser)->identifier);

    if(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected identifier for the function-declaration's name!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    function_declaration->parameter_list = parser_parse_parameter_list(parser);
    if(function_declaration->parameter_list == NULL)
        return NULL;

    if(parser_eat(parser, FRX_TOKEN_TYPE_SEMICOLON))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Missing semicolon after function-declaration!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    return function_declaration;
}

static FRX_NO_DISCARD b8 is_enum_definition(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    SourceLocation location = parser_current_location(parser);

    parser_skip_optional_keywords(parser);

    b8 result = parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_ENUM;

    parser_recover(parser, &location);

    return result;
}

static FRX_NO_DISCARD ASTEnumDefinition* parser_parse_enum_definition(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    ASTEnumDefinition* enum_definition = arena_alloc(&parser->arena, sizeof(ASTEnumDefinition));
    enum_definition->exported = FRX_FALSE;

    list_init(&enum_definition->constants, FRX_MEMORY_CATEGORY_AST);

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_EXPORT)
    {
        enum_definition->exported = FRX_TRUE;
        parser_skip(parser);
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_KW_ENUM))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected keyword 'enum'!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    strcpy(enum_definition->name, parser_current_token(parser)->identifier);

    if(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected identifier for the enum's name!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_COLON))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected colon after enum's name!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    enum_definition->type = parser_parse_type(parser);
    if(enum_definition->type == NULL)
        return NULL;

    if(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_BRACE))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected '{' to start the enum-definition!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    while(parser_current_token(parser)->type != FRX_TOKEN_TYPE_RIGHT_BRACE)
    {
        AST* constant = arena_alloc(&parser->arena, sizeof(AST));
        constant->node = NULL;

        if(parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_EQUALS)
        {
            constant->type = FRX_AST_TYPE_VARIABLE_ASSIGNMENT;
            constant->node = parser_parse_variable_assignment(parser);
        }
        else
        {
            constant->type = FRX_AST_TYPE_VARIABLE;
            constant->node = parser_parse_variable(parser);
        }

        if(constant->node == NULL)
            return NULL;

        list_push(&enum_definition->constants, constant);

        if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_COMMA
                && parser_peek(parser, 1)->type != FRX_TOKEN_TYPE_RIGHT_BRACE)
            parser_skip(parser);
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_BRACE))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Missing '}' at the end of enum-definition!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    return enum_definition;
}

static FRX_NO_DISCARD b8 is_struct_definition(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    SourceLocation location = parser_current_location(parser);

    parser_skip_optional_keywords(parser);

    b8 result = parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_STRUCT;

    parser_recover(parser, &location);

    return result;
}

static FRX_NO_DISCARD StructSymbol* parser_generate_struct_symbol(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    StructTable* struct_table = get_global_struct_table();

    parser_skip_optional_keywords(parser);

    if(parser_eat(parser, FRX_TOKEN_TYPE_KW_STRUCT))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected keyword 'struct'!", parser->lexer.filepath,
                       location.line, location.coloumn);

        return NULL;
    }

    //TODO: Allow namespaces e.g. "struct std::foo::Bar"
    Namespace* namespace = namespace_duplicate(parser->current_namespace);

    StructSymbol* struct_symbol = struct_table_find(struct_table,
        parser_current_token(parser)->identifier, namespace);

    if(struct_symbol != NULL && struct_symbol->defined)
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Encountered redefinition of struct '%s'!",
                       parser->lexer.filepath, location.line, location.coloumn,
                       struct_symbol->name);

        return NULL;
    }

    struct_symbol = struct_table_find_or_insert(struct_table,
        parser_current_token(parser)->identifier, namespace);

    if(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected identifier for the struct's name!",
                       parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_BRACE))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected '{' to start the struct-definition!",
                       parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    while(parser_current_token(parser)->type != FRX_TOKEN_TYPE_RIGHT_BRACE)
    {
        //TODO: Handle struct members

        parser_skip(parser);
    }

    parser_skip(parser);

    struct_symbol->defined = FRX_TRUE;

    return struct_symbol;
}

static FRX_NO_DISCARD ASTStructDefinition* parser_parse_struct_definition(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    ASTStructDefinition* struct_definition = arena_alloc(&parser->arena, sizeof(ASTStructDefinition));
    struct_definition->exported = FRX_FALSE;

    SourceLocation location = parser_current_location(parser);
    struct_definition->struct_symbol = parser_generate_struct_symbol(parser);
    parser_recover(parser, &location);

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_EXPORT)
    {
        struct_definition->exported = FRX_TRUE;
        parser_skip(parser);
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_KW_STRUCT))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected keyword 'struct'!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    strcpy(struct_definition->name, parser_current_token(parser)->identifier);

    if(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected identifier for the struct's name!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_BRACE))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected '{' to start the struct-definition!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    while(parser_current_token(parser)->type != FRX_TOKEN_TYPE_RIGHT_BRACE)
    {
        ASTVariableDeclaration* field = parser_parse_variable_declaration(parser);
        if(field == NULL)
            return NULL;

        list_push(&struct_definition->struct_symbol->fields, field);
    }

    parser_skip(parser);

    return struct_definition;
}

static FRX_NO_DISCARD b8 is_namespace_resolution(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    return parser_current_token(parser)->type == FRX_TOKEN_TYPE_IDENTIFIER
        && parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_NAMESPACE_RESOLUTION;
}

static FRX_NO_DISCARD ASTNamespaceRef* parser_parse_namespace_resolution(Parser* parser)
{
    //TODO: Handle case where we just have a namespace resolution operator for referencing the global namespace.

    FRX_ASSERT(parser != NULL);

    ASTNamespaceRef* namespace_ref = arena_alloc(&parser->arena, sizeof(ASTNamespaceRef));

    strcpy(namespace_ref->name, parser_current_token(parser)->identifier);

    if(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected identifier for the namespace-ref's name!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_NAMESPACE_RESOLUTION))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected '::'!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_IDENTIFIER && parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_NAMESPACE_RESOLUTION)
    {
        namespace_ref->next = parser_parse_namespace_resolution(parser);
        if(namespace_ref->next == NULL)
            return NULL;
    }
    else
        namespace_ref->next = NULL;

    return namespace_ref;
}

static FRX_NO_DISCARD b8 is_namespace(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    return parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_NAMESPACE;
}

static FRX_NO_DISCARD ASTNamespace* parser_parse_namespace(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    if(parser_eat(parser, FRX_TOKEN_TYPE_KW_NAMESPACE))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected keyword 'namespace'!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    ASTNamespace* namespace = arena_alloc(&parser->arena, sizeof(ASTNamespace));

    list_init(&namespace->top_level_definitions, FRX_MEMORY_CATEGORY_AST);

    strcpy(namespace->name, parser_current_token(parser)->identifier);

    namespace_append(parser->current_namespace,
            parser_current_token(parser)->identifier);

    if(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected identifier for the namespace's name!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_BRACE))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected '{' to start the namespace!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    while(parser_current_token(parser)->type != FRX_TOKEN_TYPE_RIGHT_BRACE)
    {
        AST* top_level_definition = parser_parse_top_level_definition(parser);
        if(top_level_definition == NULL)
            return NULL;

        list_push(&namespace->top_level_definitions, top_level_definition);
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_BRACE))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Missing '}' at the end of namespace!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    namespace_drop(parser->current_namespace);

    return namespace;
}

static FRX_NO_DISCARD b8 is_extern_block(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    return parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_EXTERN;
}

static FRX_NO_DISCARD ASTExternBlock* parser_parse_extern_block(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    if(parser_eat(parser, FRX_TOKEN_TYPE_KW_EXTERN))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected keyword 'extern'!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_BRACE))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected token '{' to start extern-block!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    ASTExternBlock* extern_block = arena_alloc(&parser->arena, sizeof(ASTExternBlock));

    list_init(&extern_block->struct_definitions, FRX_MEMORY_CATEGORY_AST);
    list_init(&extern_block->function_declarations, FRX_MEMORY_CATEGORY_AST);

    while(parser_current_token(parser)->type != FRX_TOKEN_TYPE_RIGHT_BRACE)
    {
        if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_STRUCT)
        {
            ASTStructDefinition* struct_definition = parser_parse_struct_definition(parser);
            if(struct_definition == NULL)
                return NULL;

            list_push(&extern_block->struct_definitions, struct_definition);
        }
        else
        {
            ASTFunctionDeclaration* function_declaration = parser_parse_function_declaration(parser, FRX_TRUE);
            if(function_declaration == NULL)
                return NULL;

            list_push(&extern_block->function_declarations, function_declaration);
        }
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_BRACE))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Missing '}' at the end of extern-block!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    return extern_block;
}

static FRX_NO_DISCARD b8 is_macro(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    return parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_MACRO
        || (parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_EXPORT
        && parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_KW_MACRO);
}

static FRX_NO_DISCARD ASTMacro* parser_parse_macro(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    ASTMacro* macro = arena_alloc(&parser->arena, sizeof(ASTMacro));
    macro->exported = FRX_FALSE;

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_EXPORT)
    {
        macro->exported = FRX_TRUE;
        parser_skip(parser);
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_KW_MACRO))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected keyword 'macro'!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    strcpy(macro->name, parser_current_token(parser)->identifier);

    if(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected identifier for the macro's name!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_EQUALS))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected '=' after macro name!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    macro->value = parser_parse_expression(parser);
    if(macro->value == NULL)
        return NULL;

    if(parser_eat(parser, FRX_TOKEN_TYPE_SEMICOLON))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected ';' at the end of macro-constant!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    return macro;
}

static FRX_NO_DISCARD ASTSizeof* parser_parse_sizeof(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    ASTSizeof* _sizeof = arena_alloc(&parser->arena, sizeof(ASTSizeof));

    if(parser_eat(parser, FRX_TOKEN_TYPE_KW_SIZEOF))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected keyword 'sizeof'!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_PARANTHESIS))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected '('!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    if(!is_primitive(parser_current_token(parser)->type) && parser_current_token(parser)->type != FRX_TOKEN_TYPE_IDENTIFIER)
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected identifier or primitive inside sizeof!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    strcpy(_sizeof->type, parser_current_token(parser)->identifier);

    parser_skip(parser);

    if(parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_PARANTHESIS))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected ')'!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    return _sizeof;
}

static FRX_NO_DISCARD ASTAssert* parser_parse_assert(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    ASTAssert* assert = arena_alloc(&parser->arena, sizeof(ASTAssert));

    assert->filepath = parser->lexer.filepath;
    assert->line = parser_current_token(parser)->location.line;

    if(parser_eat(parser, FRX_TOKEN_TYPE_KW_ASSERT))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected keyword 'assert'!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_PARANTHESIS))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected '(' after keyword 'assert'!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    assert->condition = parser_parse_expression(parser);

    if(parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_PARANTHESIS))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected ')' at the end of assert-statement!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_SEMICOLON))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected ';' at the end of assert-statement!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    return assert;
}

static FRX_NO_DISCARD b8 is_import_statement(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    return parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_IMPORT;
}

static ASTImportStatement* parser_parse_import_statement(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    if(parser_eat(parser, FRX_TOKEN_TYPE_KW_IMPORT))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected keyword 'import'!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    ASTImportStatement* import_statement = arena_alloc(&parser->arena, sizeof(ASTImportStatement));

    strcpy(import_statement->filepath, parser_current_token(parser)->identifier);

    if(parser_eat(parser, FRX_TOKEN_TYPE_STRING_LITERAL))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected string-literal after import-statement!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_SEMICOLON))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Missing semicolon after import-statement!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    return import_statement;
}

static FRX_NO_DISCARD AST* parser_parse_top_level_definition(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    AST* ast = arena_alloc(&parser->arena, sizeof(AST));

    if(is_namespace(parser))
    {
        ast->type = FRX_AST_TYPE_NAMESPACE;
        ast->node = parser_parse_namespace(parser);
        if(ast->node == NULL)
            return NULL;

        return ast;
    }

    if(is_enum_definition(parser))
    {
        ast->type = FRX_AST_TYPE_ENUM_DEFINITION;
        ast->node = parser_parse_enum_definition(parser);
        if(ast->node == NULL)
            return NULL;

        return ast;
    }

    if(is_struct_definition(parser))
    {
        ast->type = FRX_AST_TYPE_STRUCT_DEFINITION;
        ast->node = parser_parse_struct_definition(parser);
        if(ast->node == NULL)
            return NULL;

        return ast;
    }

    if(is_extern_block(parser))
    {
        ast->type = FRX_AST_TYPE_EXTERN_BLOCK;
        ast->node = parser_parse_extern_block(parser);
        if(ast->node == NULL)
            return NULL;

        return ast;
    }

    if(is_macro(parser))
    {
        ast->type = FRX_AST_TYPE_MACRO;
        ast->node = parser_parse_macro(parser);
        if(ast->node == NULL)
            return NULL;

        return ast;
    }

    if(is_import_statement(parser))
    {
        ast->type = FRX_AST_TYPE_IMPORT_STATEMENT;
        ast->node = parser_parse_import_statement(parser);
        if(ast->node == NULL)
            return NULL;

        return ast;
    }

    if(is_function_definition(parser))
    {
        ast->type = FRX_AST_TYPE_FUNCTION_DEFINITION;
        ast->node = parser_parse_function_definition(parser);
        if(ast->node == NULL)
            return NULL;

        return ast;
    }

    if(is_variable_declaration(parser))
    {
        ast->type = FRX_AST_TYPE_VARIABLE_DECLARATION;
        ast->node = parser_parse_variable_declaration(parser);
        if(ast->node == NULL)
            return NULL;

        return ast;
    }

    if(is_variable_definition(parser))
    {
        ast->type = FRX_AST_TYPE_VARIABLE_DEFINITION;
        ast->node = parser_parse_variable_definition(parser);
        if(ast->node == NULL)
            return NULL;

        return ast;
    }

    SourceLocation location = parser_current_location(parser);
    FRX_ERROR_FILE("Could not parse token %s, expected include-statement, namespace, struct-definition, extern-block or function-definition!", parser->lexer.filepath, location.line, location.coloumn, token_type_to_str(parser_current_token(parser)->type));

    return NULL;
}

static FRX_NO_DISCARD ASTProgram* parser_parse_program(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    ASTProgram* program = arena_alloc(&parser->arena, sizeof(ASTProgram));

    list_init(&program->top_level_definitions, FRX_MEMORY_CATEGORY_AST);

    while(parser_current_token(parser)->type != FRX_TOKEN_TYPE_EOF)
    {
        AST* top_level_definition = parser_parse_top_level_definition(parser);
        if(top_level_definition == NULL)
            return NULL;

        list_push(&program->top_level_definitions, top_level_definition);
    }

    return program;
}

FRX_NO_DISCARD b8 parser_init(Parser* parser, const char* filepath)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(filepath != NULL);

    parser->current_namespace = namespace_create("");

    //TODO: Remove constant size. This is only a temporary easy solution
    // because we only parse small files for now.
    arena_init(&parser->arena, 1024 * 1024);

    return lexer_init(&parser->lexer, filepath);
}

FRX_NO_DISCARD b8 parser_parse(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    parser->program = parser_parse_program(parser);

    return parser->program == NULL;
}
