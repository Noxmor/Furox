#include "parser.h"

#include <string.h>

#include "core/assert.h"
#include "core/log.h"
#include "core/memory.h"

#define FRX_PARSER_ABORT_ON_ERROR(expr) if(expr) return FRX_TRUE

#define FRX_PARSER_TRY_PARSE(expr, parser, location)\
    if(!(expr))\
        return FRX_FALSE;\
    parser_recover(parser, location)

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
        case FRX_AST_TYPE_LOGICAL_OR: return 0;

        case FRX_AST_TYPE_LOGICAL_AND: return 1;

        case FRX_AST_TYPE_BINARY_OR: return 2;

        case FRX_AST_TYPE_BINARY_XOR: return 3;

        case FRX_AST_TYPE_BINARY_AND: return 4;

        case FRX_AST_TYPE_COMPARISON: return 5;

        case FRX_AST_TYPE_GREATER_THAN:
        case FRX_AST_TYPE_GREATER_THAN_EQUALS:
        case FRX_AST_TYPE_LESS_THAN:
        case FRX_AST_TYPE_LESS_THAN_EQUALS: return 6;

        case FRX_AST_TYPE_BINARY_LEFT_SHIFT:
        case FRX_AST_TYPE_BINARY_RIGHT_SHIFT: return 7;

        case FRX_AST_TYPE_ADDITION:
        case FRX_AST_TYPE_SUBTRACTION: return 8;

        case FRX_AST_TYPE_MULTIPLICATION:
        case FRX_AST_TYPE_DIVISION:
        case FRX_AST_TYPE_MODULO: return 9;

        case FRX_AST_TYPE_ARITHMETIC_NEGATION:
        case FRX_AST_TYPE_LOGICAL_NEGATION:
        case FRX_AST_TYPE_BINARY_NEGATION:

        case FRX_AST_TYPE_DEREFERENCE:
        case FRX_AST_TYPE_ADDRESS_OF: return 10;
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

static FRX_NO_DISCARD b8 parser_parse_expression(Parser* parser, AST* node);

static FRX_NO_DISCARD b8 parser_parse_namespace_resolution(Parser* parser, AST* node, AST** last_namespace_child);

static FRX_NO_DISCARD b8 parser_parse_top_level(Parser* parser, AST* node);

static FRX_NO_DISCARD b8 parser_parse_variable(Parser* parser, AST* node)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

    node->type = FRX_AST_TYPE_VARIABLE;

    VariableData* data = memory_alloc(sizeof(VariableData), FRX_MEMORY_CATEGORY_UNKNOWN);
    node->data = data;

    strcpy(data->name, parser_current_token(parser)->identifier);

    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER));

    AST* index = ast_new_child(node);

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_LEFT_BRACKET)
    {
        FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_BRACKET));

        index->type = FRX_AST_TYPE_VARIABLE_ARRAY_ACCESS;
        FRX_PARSER_ABORT_ON_ERROR(parser_parse_expression(parser, ast_new_child(index)));

        FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_BRACKET));
    }
    else
        index->type = FRX_AST_TYPE_NOOP;

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_DOT || parser_current_token(parser)->type == FRX_TOKEN_TYPE_ARROW)
    {
        data->is_pointer = parser_current_token(parser)->type == FRX_TOKEN_TYPE_ARROW;

        FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, parser_current_token(parser)->type));

        AST* child = ast_new_child(node);

        return parser_parse_variable(parser, child);
    }

    return FRX_FALSE;
}

static FRX_NO_DISCARD b8 parser_parse_number(Parser* parser, AST* node)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

    node->type = FRX_AST_TYPE_NUMBER;

    NumberData* data = memory_alloc(sizeof(NumberData), FRX_MEMORY_CATEGORY_UNKNOWN);
    node->data = data;

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_NULLPTR)
    {
        data->number = 0;

        return parser_eat(parser, FRX_TOKEN_TYPE_KW_NULLPTR);
    }

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_TRUE)
    {
        data->number = 1;

        return parser_eat(parser, FRX_TOKEN_TYPE_KW_TRUE);
    }

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_FALSE)
    {
        data->number = 0;

        return parser_eat(parser, FRX_TOKEN_TYPE_KW_FALSE);
    }

    data->number = parser_current_token(parser)->number;

    return parser_eat(parser, FRX_TOKEN_TYPE_NUMBER);
}

static FRX_NO_DISCARD b8 parser_parse_char_literal(Parser* parser, AST* node)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

    node->type = FRX_AST_TYPE_CHAR_LITERAL;

    CharLiteralData* data = memory_alloc(sizeof(CharLiteralData), FRX_MEMORY_CATEGORY_UNKNOWN);
    node->data = data;

    strcpy(data->literal, parser_current_token(parser)->identifier);

    return parser_eat(parser, FRX_TOKEN_TYPE_CHAR_LITERAL);
}

static FRX_NO_DISCARD b8 parser_parse_string_literal(Parser* parser, AST* node)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

    node->type = FRX_AST_TYPE_STRING_LITERAL;

    StringLiteralData* data = memory_alloc(sizeof(StringLiteralData), FRX_MEMORY_CATEGORY_UNKNOWN);
    node->data = data;

    strcpy(data->literal, parser_current_token(parser)->identifier);

    return parser_eat(parser, FRX_TOKEN_TYPE_STRING_LITERAL);
}

static FRX_NO_DISCARD b8 parser_parse_function_call(Parser* parser, AST* node, b8 is_statement)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

    node->type = FRX_AST_TYPE_FUNCTION_CALL;

    FunctionCallData* data = memory_alloc(sizeof(FunctionCallData), FRX_MEMORY_CATEGORY_UNKNOWN);
    node->data = data;

    data->is_statement = is_statement;

    strcpy(data->name, parser_current_token(parser)->identifier);

    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER));

    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_PARANTHESIS));

    while(parser_current_token(parser)->type != FRX_TOKEN_TYPE_RIGHT_PARANTHESIS)
    {
        AST* parameter = ast_new_child(node);

        FRX_PARSER_ABORT_ON_ERROR(parser_parse_expression(parser, parameter));

        if(parser_current_token(parser)->type != FRX_TOKEN_TYPE_COMMA)
            break;

        FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_COMMA));
    }

    return parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_PARANTHESIS);
}

static FRX_NO_DISCARD b8 parser_parse_expression(Parser* parser, AST* node);

static FRX_NO_DISCARD b8 parser_parse_primary_expression(Parser* parser, AST* node)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_LEFT_PARANTHESIS)
    {
        FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_PARANTHESIS));

        FRX_PARSER_ABORT_ON_ERROR(parser_parse_primary_expression(parser, node));

        return parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_PARANTHESIS);
    }

    if(parser_next_token_is_unary_operator(parser))
    {
        switch(parser_current_token(parser)->type)
        {
            case FRX_TOKEN_TYPE_MINUS: ast_init(node, FRX_AST_TYPE_ARITHMETIC_NEGATION); break;
            case FRX_TOKEN_TYPE_LOGICAL_NEGATION: ast_init(node, FRX_AST_TYPE_LOGICAL_NEGATION); break;
            case FRX_TOKEN_TYPE_BINARY_NEGATION: ast_init(node, FRX_AST_TYPE_BINARY_NEGATION); break;

            case FRX_TOKEN_TYPE_STAR: ast_init(node, FRX_AST_TYPE_DEREFERENCE); break;
            case FRX_TOKEN_TYPE_BINARY_AND: ast_init(node, FRX_AST_TYPE_ADDRESS_OF); break;

            default:
            {
                FRX_ASSERT(FRX_FALSE); //Since all possible unary operators should be handled in a switch case,
                                       //we should never end up in this default case.

                break;
            }
        }

        FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, parser_current_token(parser)->type));

        AST* child = ast_new_child(node);

        if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_LEFT_PARANTHESIS)
            return parser_parse_expression(parser, child);

        return parser_parse_primary_expression(parser, child);
    }

    switch(parser_current_token(parser)->type)
    {
        case FRX_TOKEN_TYPE_IDENTIFIER:
        {
            if(parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_NAMESPACE_RESOLUTION)
            {
                AST* child = NULL;

                FRX_PARSER_ABORT_ON_ERROR(parser_parse_namespace_resolution(parser, node, &child));

                FRX_ASSERT(child != NULL);

                return parser_parse_function_call(parser, child, FRX_FALSE);
            }

            if(parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_LEFT_PARANTHESIS)
                return parser_parse_function_call(parser, node, FRX_FALSE);

            return parser_parse_variable(parser, node);
        }

        case FRX_TOKEN_TYPE_KW_NULLPTR:
        case FRX_TOKEN_TYPE_KW_TRUE:
        case FRX_TOKEN_TYPE_KW_FALSE:
        case FRX_TOKEN_TYPE_NUMBER: return parser_parse_number(parser, node);

        case FRX_TOKEN_TYPE_CHAR_LITERAL: return parser_parse_char_literal(parser, node);
        case FRX_TOKEN_TYPE_STRING_LITERAL: return parser_parse_string_literal(parser, node);
    }

    SourceLocation location = parser_current_location(parser);
    FRX_ERROR_FILE("Could not parse token %s, expected namespace-resolution, function-call, variable, number, char-literal or string-literal!", parser->lexer.filepath, location.line, location.coloumn, token_type_to_str(parser_current_token(parser)->type));

    return FRX_TRUE;
}

static FRX_NO_DISCARD b8 parser_parse_expression(Parser* parser, AST* node)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

   b8 node_had_paranthesis = FRX_FALSE;

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_LEFT_PARANTHESIS)
    {
        node_had_paranthesis = FRX_TRUE;

        FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_PARANTHESIS));

        FRX_PARSER_ABORT_ON_ERROR(parser_parse_expression(parser, node));

        FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_PARANTHESIS));
    }
    else
    {
        FRX_PARSER_ABORT_ON_ERROR(parser_parse_primary_expression(parser, node));
    }

    while(parser_next_token_is_binary_operator(parser))
    {
        AST new_node;

        switch(parser_current_token(parser)->type)
        {
            case FRX_TOKEN_TYPE_PLUS: ast_init(&new_node, FRX_AST_TYPE_ADDITION); break;
            case FRX_TOKEN_TYPE_MINUS: ast_init(&new_node, FRX_AST_TYPE_SUBTRACTION); break;
            case FRX_TOKEN_TYPE_STAR: ast_init(&new_node, FRX_AST_TYPE_MULTIPLICATION); break;
            case FRX_TOKEN_TYPE_SLASH: ast_init(&new_node, FRX_AST_TYPE_DIVISION); break;
            case FRX_TOKEN_TYPE_MODULO: ast_init(&new_node, FRX_AST_TYPE_MODULO); break;

            case FRX_TOKEN_TYPE_LOGICAL_AND: ast_init(&new_node, FRX_AST_TYPE_LOGICAL_AND); break;
            case FRX_TOKEN_TYPE_LOGICAL_OR: ast_init(&new_node, FRX_AST_TYPE_LOGICAL_OR); break;

            case FRX_TOKEN_TYPE_BINARY_AND: ast_init(&new_node, FRX_AST_TYPE_BINARY_AND); break;
            case FRX_TOKEN_TYPE_BINARY_OR: ast_init(&new_node, FRX_AST_TYPE_BINARY_OR); break;
            case FRX_TOKEN_TYPE_BINARY_XOR: ast_init(&new_node, FRX_AST_TYPE_BINARY_XOR); break;
            case FRX_TOKEN_TYPE_BINARY_LEFT_SHIFT: ast_init(&new_node, FRX_AST_TYPE_BINARY_LEFT_SHIFT); break;
            case FRX_TOKEN_TYPE_BINARY_RIGHT_SHIFT: ast_init(&new_node, FRX_AST_TYPE_BINARY_RIGHT_SHIFT); break;

            case FRX_TOKEN_TYPE_COMPARISON: ast_init(&new_node, FRX_AST_TYPE_COMPARISON); break;

            case FRX_TOKEN_TYPE_GREATER_THAN: ast_init(&new_node, FRX_AST_TYPE_GREATER_THAN); break;
            case FRX_TOKEN_TYPE_GREATER_THAN_EQUALS: ast_init(&new_node, FRX_AST_TYPE_GREATER_THAN_EQUALS); break;
            case FRX_TOKEN_TYPE_LESS_THAN: ast_init(&new_node, FRX_AST_TYPE_LESS_THAN); break;
            case FRX_TOKEN_TYPE_LESS_THAN_EQUALS: ast_init(&new_node, FRX_AST_TYPE_LESS_THAN_EQUALS); break;

            default:
            {
                FRX_ASSERT(FRX_FALSE); //Since we only stay in the while loop as long as the next token is a binary operator,
                                       //we should never end up in this default case.

                break;
            }
        }

        FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, parser_current_token(parser)->type));

        if(parser_get_precedence(new_node.type) > parser_get_precedence(node->type) && !node_had_paranthesis)
        {
            AST temp;

            AST* right = &node->children[1];
            memcpy(&temp, right, sizeof(AST));

            memcpy(right, &new_node, sizeof(AST));

            AST* new_left = ast_new_child(right);
            memcpy(new_left, &temp, sizeof(AST));

            AST* new_right = ast_new_child(right);

            if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_LEFT_PARANTHESIS)
            {
                FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_PARANTHESIS));

                FRX_PARSER_ABORT_ON_ERROR(parser_parse_expression(parser, new_right));

                FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_PARANTHESIS));
            }
            else
            {
                FRX_PARSER_ABORT_ON_ERROR(parser_parse_primary_expression(parser, new_right));
            }
        }
        else
        {
            AST temp;
            memcpy(&temp, node, sizeof(AST));

            memcpy(node, &new_node, sizeof(AST));

            AST* left = ast_new_child(node);
            memcpy(left, &temp, sizeof(AST));

            AST* right = ast_new_child(node);

            if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_LEFT_PARANTHESIS)
            {
                FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_PARANTHESIS));

                FRX_PARSER_ABORT_ON_ERROR(parser_parse_expression(parser, right));

                FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_PARANTHESIS));
            }
            else
            {
                FRX_PARSER_ABORT_ON_ERROR(parser_parse_primary_expression(parser, right));
            }
        }
    }

    return FRX_FALSE;
}

static FRX_NO_DISCARD b8 parser_parse_type(Parser* parser, AST* node)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_IDENTIFIER && parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_NAMESPACE_RESOLUTION)
    {
        AST* child = NULL;

        FRX_PARSER_ABORT_ON_ERROR(parser_parse_namespace_resolution(parser, node, &child));

        FRX_ASSERT(child != NULL);

        return parser_parse_type(parser, child);
    }

    node->type = FRX_AST_TYPE_TYPE;

    TypeData* data = memory_alloc(sizeof(TypeData), FRX_MEMORY_CATEGORY_UNKNOWN);
    node->data = data;

    strcpy(data->name, parser_current_token(parser)->identifier);

    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER));

    data->pointer_level = 0;

    while(parser_current_token(parser)->type == FRX_TOKEN_TYPE_STAR)
    {
        FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_STAR));

        ++data->pointer_level;
    }

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_LEFT_BRACKET)
    {
        FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_BRACKET));

        AST* expr = ast_new_child(node);

        FRX_PARSER_ABORT_ON_ERROR(parser_parse_expression(parser, expr));

        FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_BRACKET));
    }

    return FRX_FALSE;
}

static FRX_NO_DISCARD b8 parser_parse_variable_declaration(Parser* parser, AST* node)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

    node->type = FRX_AST_TYPE_VARIABLE_DECLARATION;

    AST* type = ast_new_child(node);

    FRX_PARSER_ABORT_ON_ERROR(parser_parse_type(parser, type));

    VariableData* data = memory_alloc(sizeof(VariableData), FRX_MEMORY_CATEGORY_UNKNOWN);
    node->data = data;

    strcpy(data->name, parser_current_token(parser)->identifier);

    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER));

    return parser_eat(parser, FRX_TOKEN_TYPE_SEMICOLON);
}

static FRX_NO_DISCARD b8 parser_parse_variable_definition(Parser* parser, AST* node)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

    node->type = FRX_AST_TYPE_VARIABLE_DEFINITION;

    VariableData* data = memory_alloc(sizeof(VariableData), FRX_MEMORY_CATEGORY_UNKNOWN);
    node->data = data;

    AST* type = ast_new_child(node);

    FRX_PARSER_ABORT_ON_ERROR(parser_parse_type(parser, type));

    strcpy(data->name, parser_current_token(parser)->identifier);

    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER));

    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_EQUALS));

    AST* child = ast_new_child(node);

    FRX_PARSER_ABORT_ON_ERROR(parser_parse_expression(parser, child));

    return parser_eat(parser, FRX_TOKEN_TYPE_SEMICOLON);
}

static FRX_NO_DISCARD b8 parser_parse_variable_assignment(Parser* parser, AST* node)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

    node->type = FRX_AST_TYPE_VARIABLE_ASSIGNMENT;

    AST* variable = ast_new_child(node);
    FRX_PARSER_ABORT_ON_ERROR(parser_parse_variable(parser, variable));

    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_EQUALS));

    AST* expr = ast_new_child(node);
    FRX_PARSER_ABORT_ON_ERROR(parser_parse_expression(parser, expr));

    return parser_eat(parser, FRX_TOKEN_TYPE_SEMICOLON);
}

static FRX_NO_DISCARD b8 parser_parse_return_statement(Parser* parser, AST* node)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

    node->type = FRX_AST_TYPE_RETURN_STATEMENT;

    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_KW_RETURN));

    if(parser_current_token(parser)->type != FRX_TOKEN_TYPE_SEMICOLON)
    {
        AST* child = ast_new_child(node);
        
        FRX_PARSER_ABORT_ON_ERROR(parser_parse_expression(parser, child));
    }

    return parser_eat(parser, FRX_TOKEN_TYPE_SEMICOLON);
}

static FRX_NO_DISCARD b8 parser_parse_statement(Parser* parser, AST* node)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_RETURN)
        return parser_parse_return_statement(parser, node);

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_IDENTIFIER)
    {
        if(parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_NAMESPACE_RESOLUTION)
        {
            AST* child = NULL;

            SourceLocation recover_location = parser_current_location(parser);

            FRX_PARSER_ABORT_ON_ERROR(parser_parse_namespace_resolution(parser, node, &child));

            FRX_ASSERT(child != NULL);

            if(parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_LEFT_PARANTHESIS)
            {
                FRX_PARSER_ABORT_ON_ERROR(parser_parse_function_call(parser, child, FRX_TRUE));

                return parser_eat(parser, FRX_TOKEN_TYPE_SEMICOLON);
            }

            AST type;
            ast_init(&type, FRX_AST_TYPE_NOOP);
            FRX_PARSER_ABORT_ON_ERROR(parser_parse_type(parser, &type));

            //NOTE: The following lines are leaking memory, since we never delete any AST nodes from the namespace resolution parse.
            if(parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_EQUALS)
            {
                parser_recover(parser, &recover_location);
                node->children_size = 0;

                return parser_parse_variable_definition(parser, node);
            }

            parser_recover(parser, &recover_location);
            node->children_size = 0;

            return parser_parse_variable_declaration(parser, node);
        }

        if(parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_LEFT_PARANTHESIS)
        {
            FRX_PARSER_ABORT_ON_ERROR(parser_parse_function_call(parser, node, FRX_TRUE));

            return parser_eat(parser, FRX_TOKEN_TYPE_SEMICOLON);
        }

        SourceLocation recover_location = parser_current_location(parser);

        AST type;
        ast_init(&type, FRX_AST_TYPE_NOOP);
        if(!parser_parse_type(parser, &type) && parser_current_token(parser)->type == FRX_TOKEN_TYPE_IDENTIFIER)
        {
            if(parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_EQUALS)
            {
                parser_recover(parser, &recover_location);
                return parser_parse_variable_definition(parser, node);
            }

            parser_recover(parser, &recover_location);
            return parser_parse_variable_declaration(parser, node);
        }

        parser_recover(parser, &recover_location);

        if(parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_EQUALS || parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_DOT || parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_ARROW)
            return parser_parse_variable_assignment(parser, node);
    }

    //TODO: If no rule applied so far, parse this as an assignment, where the lhs of the assignment is a complex expression.
    //Ensure that the lhs is a valid lvalue and not a rvalue afterwards.

    FRX_ERROR_FILE("Could not parse token %s!", parser->lexer.filepath, parser_current_location(parser).line, parser_current_location(parser).coloumn, token_type_to_str(parser_current_token(parser)->type));

    return FRX_TRUE;
}

static FRX_NO_DISCARD b8 parser_parse_scope(Parser* parser, AST* node)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

    node->type = FRX_AST_TYPE_SCOPE;

    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_BRACE));
    
    while(parser_current_token(parser)->type != FRX_TOKEN_TYPE_RIGHT_BRACE)
    {
        AST* child = ast_new_child(node);
        FRX_PARSER_ABORT_ON_ERROR(parser_parse_statement(parser, child));
    }

    return parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_BRACE);
}

static FRX_NO_DISCARD b8 parser_parse_function_parameter_list(Parser* parser, AST* node, b8* is_variadic)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

    node->type = FRX_AST_TYPE_PARAMETER_LIST;

    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_PARANTHESIS));

    while(parser_current_token(parser)->type != FRX_TOKEN_TYPE_RIGHT_PARANTHESIS)
    {
        if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_ELLIPSIS)
        {
            if(node->children_size == 0)
            {
                FRX_ERROR_FILE("Variadic function arguments requiere at least one named argument!", parser->lexer.filepath, parser_current_location(parser).line, parser_current_location(parser).coloumn);

               return FRX_TRUE;
            }

            FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_ELLIPSIS));

            *is_variadic = FRX_TRUE;

            if(parser_current_token(parser)->type != FRX_TOKEN_TYPE_RIGHT_PARANTHESIS)
            {
                FRX_ERROR_FILE("Variadic function arguments must come last in function signature!", parser->lexer.filepath, parser_current_location(parser).line, parser_current_location(parser).coloumn);

                return FRX_TRUE;
            }

            break;
        }

        AST* parameter = ast_new_child(node);

        AST* parameter_type = ast_new_child(parameter);

        FRX_PARSER_ABORT_ON_ERROR(parser_parse_type(parser, parameter_type));

        VariableData* data = memory_alloc(sizeof(VariableData), FRX_MEMORY_CATEGORY_UNKNOWN);
        parameter->data = data;

        strcpy(data->name, parser_current_token(parser)->identifier);

        FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER));
    
        if(parser_current_token(parser)->type != FRX_TOKEN_TYPE_RIGHT_PARANTHESIS)
        {
            FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_COMMA));
        }
    }
    
    return parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_PARANTHESIS);
}

static FRX_NO_DISCARD b8 parser_parse_function_definition(Parser* parser, AST* node)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

    node->type = FRX_AST_TYPE_FUNCTION_DEFINITION;

    AST* type = ast_new_child(node);

    FRX_PARSER_ABORT_ON_ERROR(parser_parse_type(parser, type));

    FunctionDefinitionData* data = memory_alloc(sizeof(FunctionDefinitionData), FRX_MEMORY_CATEGORY_UNKNOWN);
    node->data = data;

    data->is_variadic = FRX_FALSE;

    strcpy(data->name, parser_current_token(parser)->identifier);

    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER));

    AST* parameter_list = ast_new_child(node);

    FRX_PARSER_ABORT_ON_ERROR(parser_parse_function_parameter_list(parser, parameter_list, &data->is_variadic));    

    AST* body = ast_new_child(node);

    return parser_parse_scope(parser, body);
}

static FRX_NO_DISCARD b8 parser_parse_function_declaration(Parser* parser, AST* node)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

    node->type = FRX_AST_TYPE_FUNCTION_DECLARATION;

    AST* type = ast_new_child(node);

    FRX_PARSER_ABORT_ON_ERROR(parser_parse_type(parser, type));

    FunctionDeclarationData* data = memory_alloc(sizeof(FunctionDeclarationData), FRX_MEMORY_CATEGORY_UNKNOWN);
    node->data = data;

    data->is_variadic = FRX_FALSE;

    strcpy(data->name, parser_current_token(parser)->identifier);

    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER));

    AST* parameter_list = ast_new_child(node);

    FRX_PARSER_ABORT_ON_ERROR(parser_parse_function_parameter_list(parser, parameter_list, &data->is_variadic));

    return parser_eat(parser, FRX_TOKEN_TYPE_SEMICOLON);
}

static FRX_NO_DISCARD b8 parser_parse_struct_definition(Parser* parser, AST* node)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

    node->type = FRX_AST_TYPE_STRUCT_DEFINITION;

    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_KW_STRUCT));

    StructDefinitionData* data = memory_alloc(sizeof(StructDefinitionData), FRX_MEMORY_CATEGORY_UNKNOWN);
    node->data = data;

    strcpy(data->name, parser_current_token(parser)->identifier);

    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER));

    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_BRACE));

    while(parser_current_token(parser)->type != FRX_TOKEN_TYPE_RIGHT_BRACE)
    {
        AST* field = ast_new_child(node);

        FRX_PARSER_ABORT_ON_ERROR(parser_parse_variable_declaration(parser, field));
    }

    return parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_BRACE);
}

static FRX_NO_DISCARD b8 parser_parse_namespace_resolution(Parser* parser, AST* node, AST** last_namespace_child)
{
    //TODO: Handle case where we just have a namespace resolution operator for referencing the global namespace.

    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

    node->type = FRX_AST_TYPE_NAMESPACE_REF;

    NamespaceData* data = memory_alloc(sizeof(NamespaceData), FRX_MEMORY_CATEGORY_UNKNOWN);
    node->data = data;

    strcpy(data->namespace, parser_current_token(parser)->identifier);

    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER));
    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_NAMESPACE_RESOLUTION));

    AST* child = ast_new_child(node);

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_IDENTIFIER && parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_NAMESPACE_RESOLUTION)
        return parser_parse_namespace_resolution(parser, child, last_namespace_child);

    *last_namespace_child = child;

    return FRX_FALSE;
}

static FRX_NO_DISCARD b8 parser_parse_namespace(Parser* parser, AST* node)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_KW_NAMESPACE));

    node->type = FRX_AST_TYPE_NAMESPACE;

    NamespaceData* data = memory_alloc(sizeof(NamespaceData), FRX_MEMORY_CATEGORY_UNKNOWN);
    node->data = data;

    strcpy(data->namespace, parser_current_token(parser)->identifier);

    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER));

    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_BRACE));

    while(parser_current_token(parser)->type != FRX_TOKEN_TYPE_RIGHT_BRACE)
    {
        AST* child = ast_new_child(node);

        FRX_PARSER_ABORT_ON_ERROR(parser_parse_top_level(parser, child));
    }

    return parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_BRACE);
}

static FRX_NO_DISCARD b8 parser_parse_extern_block(Parser* parser, AST* node)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

    node->type = FRX_AST_TYPE_EXTERN_BLOCK;

    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_KW_EXTERN));

    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_BRACE));

    while(parser_current_token(parser)->type != FRX_TOKEN_TYPE_RIGHT_BRACE)
    {
        AST* child = ast_new_child(node);

        if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_STRUCT)
        {
            FRX_PARSER_ABORT_ON_ERROR(parser_parse_struct_definition(parser, child));
        }
        else
            FRX_PARSER_ABORT_ON_ERROR(parser_parse_function_declaration(parser, child));
    }

    return parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_BRACE);
}

static FRX_NO_DISCARD b8 parser_parse_top_level(Parser* parser, AST* node)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_NAMESPACE)
        return parser_parse_namespace(parser, node);

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_STRUCT)
        return parser_parse_struct_definition(parser, node);

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_KW_EXTERN)
        return parser_parse_extern_block(parser, node);

    //TODO: Handle includes

    if(parser_current_token(parser)->type == FRX_TOKEN_TYPE_IDENTIFIER)
        return parser_parse_function_definition(parser, node);

    SourceLocation location = parser_current_location(parser);
    FRX_ERROR_FILE("Could not parse token %s, expected namespace, struct-definition, extern-block or function-definition!", parser->lexer.filepath, location.line, location.coloumn, token_type_to_str(parser_current_token(parser)->type));

    return FRX_TRUE;
}

FRX_NO_DISCARD b8 parser_init(Parser* parser, const char* filepath)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(filepath != NULL);

    return lexer_init(&parser->lexer, filepath);
}

FRX_NO_DISCARD b8 parser_parse(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    ast_init(&parser->root, FRX_AST_TYPE_COMPOUND);

    while(parser_current_token(parser)->type != FRX_TOKEN_TYPE_EOF)
    {
        AST* child = ast_new_child(&parser->root);

        FRX_PARSER_ABORT_ON_ERROR(parser_parse_top_level(parser, child));
    }

    return FRX_FALSE;
}
