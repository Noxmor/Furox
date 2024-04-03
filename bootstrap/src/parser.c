#include "parser.h"

#include <string.h>

#include "core/assert.h"
#include "core/log.h"
#include "core/memory.h"

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

        case FRX_TOKEN_TYPE_COMPARISON:

        case FRX_TOKEN_TYPE_GREATER_THAN:
        case FRX_TOKEN_TYPE_GREATER_THAN_EQUALS:

        case FRX_TOKEN_TYPE_LESS_THAN:
        case FRX_TOKEN_TYPE_LESS_THAN_EQUALS: return FRX_TRUE;
    }

    return FRX_FALSE;
}

static void parser_skip(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    ++parser_info.tokens_processed;

    lexer_next_token(&parser->lexer);
}

static FRX_NO_DISCARD AST* parser_parse_expression(Parser* parser);

static FRX_NO_DISCARD ASTNamespaceRef* parser_parse_namespace_resolution(Parser* parser);

static FRX_NO_DISCARD AST* parser_parse_top_level_definition(Parser* parser);

static FRX_NO_DISCARD ASTScope* parser_parse_scope(Parser* parser);

static FRX_NO_DISCARD AST* parser_parse_statement(Parser* parser);

static FRX_NO_DISCARD ASTVariable* parser_parse_variable(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    ASTVariable* variable = arena_alloc(&parser->arena, sizeof(ASTVariable));

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

static FRX_NO_DISCARD ASTFunctionCall* parser_parse_function_call(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    ASTFunctionCall* function_call = arena_alloc(&parser->arena, sizeof(ASTFunctionCall));

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

        if(parser_get_precedence(binary_expression->type) > parser_get_precedence(expression->type) && !node_had_paranthesis)
        {
            AST* temp = ((ASTBinaryExpression*)expression->node)->right;

            ((ASTBinaryExpression*)expression->node)->right = new_node;

            ((ASTBinaryExpression*)new_node->node)->left = temp;

            if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_LEFT_PARANTHESIS)
            {
                parser_skip(parser);

                ((ASTBinaryExpression*)new_node)->right = parser_parse_expression(parser);
                if(((ASTBinaryExpression*)new_node)->right == NULL)
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
                ((ASTBinaryExpression*)new_node)->right = parser_parse_primary_expression(parser);
                if(((ASTBinaryExpression*)new_node)->right == NULL)
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

    if(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER))
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

static FRX_NO_DISCARD ASTVariableDeclaration* parser_parse_variable_declaration(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    ASTVariableDeclaration* variable_declaration = arena_alloc(&parser->arena, sizeof(ASTVariableDeclaration));

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

static FRX_NO_DISCARD ASTVariableDefinition* parser_parse_variable_definition(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    ASTVariableDefinition* variable_definition = arena_alloc(&parser->arena, sizeof(ASTVariableDefinition));

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

    variable_definition->value = parser_parse_expression(parser);
    if(variable_definition->value == NULL)
        return NULL;

    if(parser_eat(parser, FRX_TOKEN_TYPE_SEMICOLON))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Missing ';' after variable-definition!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    return variable_definition;
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

    if(parser_eat(parser, FRX_TOKEN_TYPE_SEMICOLON))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Missing ';' after variable-assignment!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    return variable_assignment;
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

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_IF)
    {
        ast->type = FRX_AST_TYPE_IF_STATEMENT;
        ast->node = parser_parse_if_statement(parser);
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

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_IDENTIFIER)
    {
        if(parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_NAMESPACE_RESOLUTION)
        {
            SourceLocation recover_location = parser_current_location(parser);

            if(parser_parse_namespace_resolution(parser) == NULL)
            {
                SourceLocation location = parser_current_location(parser);
                FRX_ERROR_FILE("Expected namespace-reference!", parser->lexer.filepath, location.line, location.coloumn);

                return NULL;
            }

            if(parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_LEFT_PARANTHESIS)
            {
                parser_recover(parser, &recover_location);

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

            if(parser_parse_type(parser) == NULL)
            {
                SourceLocation location = parser_current_location(parser);
                FRX_ERROR_FILE("Expected type-specifier!", parser->lexer.filepath, location.line, location.coloumn);

                return NULL;
            }

            if(parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_EQUALS)
            {
                parser_recover(parser, &recover_location);

                ast->type = FRX_AST_TYPE_VARIABLE_DEFINITION;
                ast->node = parser_parse_variable_definition(parser);
                if(ast->node == NULL)
                    return NULL;

                return ast;
            }

            parser_recover(parser, &recover_location);

            ast->type = FRX_AST_TYPE_VARIABLE_DECLARATION;
            ast->node = parser_parse_variable_declaration(parser);
            if(ast->node == NULL)
                return NULL;

            return ast;
        }

        if(parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_LEFT_PARANTHESIS)
        {
            ast->type = FRX_AST_TYPE_FUNCTION_CALL;
            ast->node = parser_parse_function_call(parser);

            if(parser_eat(parser, FRX_TOKEN_TYPE_SEMICOLON))
            {
                SourceLocation location = parser_current_location(parser);
                FRX_ERROR_FILE("Missing ';' at the end of function-call!", parser->lexer.filepath, location.line, location.coloumn);

                return NULL;
            }

            return ast;
        }

        SourceLocation recover_location = parser_current_location(parser);

        if(parser_parse_type(parser) != NULL && parser_current_token(parser)->type == FRX_TOKEN_TYPE_IDENTIFIER)
        {
            if(parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_EQUALS)
            {
                parser_recover(parser, &recover_location);

                ast->type = FRX_AST_TYPE_VARIABLE_DEFINITION;
                ast->node = parser_parse_variable_definition(parser);
                if(ast->node == NULL)
                    return NULL;

                return ast;
            }

            parser_recover(parser, &recover_location);

            ast->type = FRX_AST_TYPE_VARIABLE_DECLARATION;
            ast->node = parser_parse_variable_declaration(parser);
            if(ast->node == NULL)
                return NULL;

            return ast;
        }

        parser_recover(parser, &recover_location);

        if(parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_EQUALS || parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_DOT || parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_ARROW)
        {
            ast->type = FRX_AST_TYPE_VARIABLE_ASSIGNMENT;
            ast->node = parser_parse_variable_assignment(parser);
            if(ast->node == NULL)
                return NULL;

            return ast;
        }
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

static FRX_NO_DISCARD ASTFunctionDefinition* parser_parse_function_definition(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    ASTFunctionDefinition* function_definition = arena_alloc(&parser->arena, sizeof(ASTFunctionDefinition));
    function_definition->exported = FRX_FALSE;

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

static FRX_NO_DISCARD ASTFunctionDeclaration* parser_parse_function_declaration(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    ASTFunctionDeclaration* function_declaration = arena_alloc(&parser->arena, sizeof(ASTFunctionDeclaration));
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
        AST* constant = parser_parse_expression(parser);
        if(constant == NULL)
            return NULL;

        if(constant->type != FRX_AST_TYPE_VARIABLE
                && constant->type != FRX_AST_TYPE_VARIABLE_ASSIGNMENT)
        {
            SourceLocation location = parser_current_location(parser);
            FRX_ERROR_FILE("Missing '}' at the end of enum-definition!", parser->lexer.filepath, location.line, location.coloumn);

            return NULL;
        }

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

static FRX_NO_DISCARD ASTStructDefinition* parser_parse_struct_definition(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    ASTStructDefinition* struct_definition = arena_alloc(&parser->arena, sizeof(ASTStructDefinition));
    struct_definition->exported = FRX_FALSE;

    list_init(&struct_definition->fields, FRX_MEMORY_CATEGORY_AST);

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

        list_push(&struct_definition->fields, field);
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_BRACE))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Missing '}' at the end of struct-definition!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    return struct_definition;
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

    return namespace;
}

static FRX_NO_DISCARD ASTModuleDefinition* parser_parse_module_definition(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    ASTModuleDefinition* module_definition = arena_alloc(&parser->arena, sizeof(ASTModuleDefinition));
    module_definition->exported = FRX_FALSE;

    list_init(&module_definition->function_declarations, FRX_MEMORY_CATEGORY_AST);

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_EXPORT)
    {
        module_definition->exported = FRX_TRUE;
        parser_skip(parser);
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_KW_MODULE))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected keyword 'module'!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    strcpy(module_definition->name, parser_current_token(parser)->identifier);

    if(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected identifier for the module-definition's name!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_BRACE))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected token '{' to start module-definition!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    while(parser_current_token(parser)->type != FRX_TOKEN_TYPE_RIGHT_BRACE)
    {
        ASTFunctionDeclaration* function_declaration = parser_parse_function_declaration(parser);
        if(function_declaration == NULL)
            return NULL;

        list_push(&module_definition->function_declarations, function_declaration);
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_BRACE))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Missing '}' at the end of module-definition!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    return module_definition;
}

static FRX_NO_DISCARD ASTModuleImplementation* parser_parse_module_implementation(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    ASTModuleImplementation* module_implementation = arena_alloc(&parser->arena, sizeof(ASTModuleImplementation));

    list_init(&module_implementation->function_definitions, FRX_MEMORY_CATEGORY_AST);

    if(parser_eat(parser, FRX_TOKEN_TYPE_KW_IMPL))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected keyword 'impl'!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    strcpy(module_implementation->name, parser_current_token(parser)->identifier);

    if(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected identifier for the module-implementation's name!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_BRACE))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Expected token '{' to start module-implementation!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    while(parser_current_token(parser)->type != FRX_TOKEN_TYPE_RIGHT_BRACE)
    {
        ASTFunctionDefinition* function_definition = parser_parse_function_definition(parser);
        if(function_definition == NULL)
            return NULL;

        list_push(&module_implementation->function_definitions, function_definition);
    }

    if(parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_BRACE))
    {
        SourceLocation location = parser_current_location(parser);
        FRX_ERROR_FILE("Missing '}' at the end of module-implementation!", parser->lexer.filepath, location.line, location.coloumn);

        return NULL;
    }

    return module_implementation;
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
            ASTFunctionDeclaration* function_declaration = parser_parse_function_declaration(parser);
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

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_NAMESPACE)
    {
        ast->type = FRX_AST_TYPE_NAMESPACE;
        ast->node = parser_parse_namespace(parser);

        return ast;
    }

    if((parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_EXPORT && parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_KW_MODULE) || parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_MODULE)
    {
        ast->type = FRX_AST_TYPE_MODULE_DEFINITION;
        ast->node = parser_parse_module_definition(parser);

        return ast;
    }

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_IMPL)
    {
        ast->type = FRX_AST_TYPE_MODULE_IMPLEMENTATION;
        ast->node = parser_parse_module_implementation(parser);

        return ast;
    }

    if((parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_EXPORT && parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_KW_ENUM) || parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_ENUM)
    {
        ast->type = FRX_AST_TYPE_ENUM_DEFINITION;
        ast->node = parser_parse_enum_definition(parser);

        return ast;
    }

    if((parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_EXPORT && parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_KW_STRUCT) || parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_STRUCT)
    {
        ast->type = FRX_AST_TYPE_STRUCT_DEFINITION;
        ast->node = parser_parse_struct_definition(parser);

        return ast;
    }

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_EXTERN)
    {
        ast->type = FRX_AST_TYPE_EXTERN_BLOCK;
        ast->node = parser_parse_extern_block(parser);

        return ast;
    }

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_IMPORT)
    {
        ast->type = FRX_AST_TYPE_IMPORT_STATEMENT;
        ast->node = parser_parse_import_statement(parser);

        return ast;
    }

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_EXPORT || parser_current_token(parser)->type == FRX_TOKEN_TYPE_IDENTIFIER)
    {
        ast->type = FRX_AST_TYPE_FUNCTION_DEFINITION;
        ast->node = parser_parse_function_definition(parser);

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
