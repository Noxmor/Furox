#include "parser.h"

#include <string.h>

#include "core/assert.h"
#include "core/log.h"
#include "core/memory.h"

#define FRX_PARSER_ABORT_ON_ERROR(expr) if(expr) return FRX_TRUE

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

static FRX_NO_DISCARD b8 parser_eat(Parser* parser, TokenType type)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(type < FRX_TOKEN_TYPE_COUNT);

    if(parser->current_token->type != type)
    {
        FRX_ERROR_FILE("Could not parse token: expected: %s, but got %s!", parser->lexer.filepath, parser->current_token->line, parser->current_token->coloumn, token_type_to_str(type), token_type_to_str(parser->current_token->type));
        
        return FRX_TRUE;
    }

    ++parser_info.tokens_processed;

    return lexer_next_token(&parser->lexer);
}

static FRX_NO_DISCARD b8 parser_parse_variable(Parser* parser, AST* node)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

    node->type = FRX_AST_TYPE_VARIABLE;

    VariableData* data = memory_alloc(sizeof(VariableData), FRX_MEMORY_CATEGORY_UNKNOWN);
    node->data = data;

    strcpy(data->type, "");
    strcpy(data->name, parser->current_token->identifier);

    return parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER);
}

static FRX_NO_DISCARD b8 parser_parse_number(Parser* parser, AST* node)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

    node->type = FRX_AST_TYPE_NUMBER;

    NumberData* data = memory_alloc(sizeof(NumberData), FRX_MEMORY_CATEGORY_UNKNOWN);
    node->data = data;

    data->number = parser->current_token->number;

    return parser_eat(parser, FRX_TOKEN_TYPE_NUMBER);
}

static FRX_NO_DISCARD b8 parser_parse_char_literal(Parser* parser, AST* node)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

    node->type = FRX_AST_TYPE_CHAR_LITERAL;

    CharLiteralData* data = memory_alloc(sizeof(CharLiteralData), FRX_MEMORY_CATEGORY_UNKNOWN);
    node->data = data;

    data->literal = parser->current_token->identifier[0];

    return parser_eat(parser, FRX_TOKEN_TYPE_CHAR_LITERAL);
}

static FRX_NO_DISCARD b8 parser_parse_string_literal(Parser* parser, AST* node)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

    node->type = FRX_AST_TYPE_STRING_LITERAL;

    StringLiteralData* data = memory_alloc(sizeof(StringLiteralData), FRX_MEMORY_CATEGORY_UNKNOWN);
    node->data = data;

    strcpy(data->literal, parser->current_token->identifier);

    return parser_eat(parser, FRX_TOKEN_TYPE_STRING_LITERAL);
}

static FRX_NO_DISCARD b8 parser_parse_function_call(Parser* parser, AST* node)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

    node->type = FRX_AST_TYPE_FUNCTION_CALL;

    FunctionCallData* data = memory_alloc(sizeof(FunctionCallData), FRX_MEMORY_CATEGORY_UNKNOWN);
    node->data = data;

    strcpy(data->name, parser->current_token->identifier);

    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER));

    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_PARANTHESIS));

    //TODO: Parse function arguments

    return parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_PARANTHESIS);
}

//TODO: Add precedence for missing operators!
static usize parser_get_precedence(ASTType type)
{
    switch(type)
    {
        case FRX_AST_TYPE_ADDITION:
        case FRX_AST_TYPE_SUBTRACTION: return 1;

       case FRX_AST_TYPE_MULTIPLICATION:
       case FRX_AST_TYPE_DIVISION: return 2;
    }

    return 1000000;
}

static b8 parser_next_token_is_unary_operator(const Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    switch(parser->current_token->type)
    {
        case FRX_TOKEN_TYPE_MINUS:
        case FRX_TOKEN_TYPE_LOGICAL_NEGATION:
        case FRX_TOKEN_TYPE_BINARY_NEGATION: return FRX_TRUE;
    }

    return FRX_FALSE;
}

static b8 parser_next_token_is_binary_operator(const Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    switch(parser->current_token->type)
    {
        case FRX_TOKEN_TYPE_PLUS:
        case FRX_TOKEN_TYPE_MINUS:
        case FRX_TOKEN_TYPE_STAR:
        case FRX_TOKEN_TYPE_SLASH:

        case FRX_TOKEN_TYPE_LOGICAL_AND:
        case FRX_TOKEN_TYPE_LOGICAL_OR:

        case FRX_TOKEN_TYPE_BINARY_AND:
        case FRX_TOKEN_TYPE_BINARY_OR:
        case FRX_TOKEN_TYPE_BINARY_XOR:
        case FRX_TOKEN_TYPE_BINARY_LEFT_SHIFT:
        case FRX_TOKEN_TYPE_BINARY_RIGHT_SHIFT:
        case FRX_TOKEN_TYPE_COMPARISON: return FRX_TRUE;
    }

    return FRX_FALSE;
}

static FRX_NO_DISCARD b8 parser_parse_expression(Parser* parser, AST* node);

static FRX_NO_DISCARD b8 parser_parse_primary_expression(Parser* parser, AST* node)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

    if(parser->current_token->type == FRX_TOKEN_TYPE_LEFT_PARANTHESIS)
    {
        FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_PARANTHESIS));

        FRX_PARSER_ABORT_ON_ERROR(parser_parse_primary_expression(parser, node));

        return parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_PARANTHESIS);
    }

    if(parser_next_token_is_unary_operator(parser))
    {
        switch(parser->current_token->type)
        {
            case FRX_TOKEN_TYPE_MINUS: ast_init(node, FRX_AST_TYPE_ARITHMETIC_NEGATION); break;
            case FRX_TOKEN_TYPE_LOGICAL_NEGATION: ast_init(node, FRX_AST_TYPE_LOGICAL_NEGATION); break;
            case FRX_TOKEN_TYPE_BINARY_NEGATION: ast_init(node, FRX_AST_TYPE_BINARY_NEGATION); break;

            default:
            {
                FRX_ASSERT(FRX_FALSE); //Since all possible unary operators should be handled in a switch case,
                                       //we should never end up in this default case.

                break;
            }
        }

        FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, parser->current_token->type));

        AST* child = ast_new_child(node, FRX_AST_TYPE_NOOP);

        if(parser->current_token->type == FRX_TOKEN_TYPE_LEFT_PARANTHESIS)
            return parser_parse_expression(parser, child);

        return parser_parse_primary_expression(parser, child);
    }

    switch(parser->current_token->type)
    {
        case FRX_TOKEN_TYPE_IDENTIFIER:
        {
            if(parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_LEFT_PARANTHESIS)
                return parser_parse_function_call(parser, node);

            return parser_parse_variable(parser, node);
        }

        case FRX_TOKEN_TYPE_NUMBER: return parser_parse_number(parser, node);

        case FRX_TOKEN_TYPE_CHAR_LITERAL: return parser_parse_char_literal(parser, node);
        case FRX_TOKEN_TYPE_STRING_LITERAL: return parser_parse_string_literal(parser, node);
    }

    FRX_ERROR_FILE("Could not parse token %s!", parser->lexer.filepath, parser->current_token->line, parser->current_token->coloumn, token_type_to_str(parser->current_token->type));

    return FRX_TRUE;
}

static FRX_NO_DISCARD b8 parser_parse_expression(Parser* parser, AST* node)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

   b8 node_had_paranthesis = FRX_FALSE;

    if(parser->current_token->type == FRX_TOKEN_TYPE_LEFT_PARANTHESIS)
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

        switch(parser->current_token->type)
        {
            case FRX_TOKEN_TYPE_PLUS: ast_init(&new_node, FRX_AST_TYPE_ADDITION); break;
            case FRX_TOKEN_TYPE_MINUS: ast_init(&new_node, FRX_AST_TYPE_SUBTRACTION); break;
            case FRX_TOKEN_TYPE_STAR: ast_init(&new_node, FRX_AST_TYPE_MULTIPLICATION); break;
            case FRX_TOKEN_TYPE_SLASH: ast_init(&new_node, FRX_AST_TYPE_DIVISION); break;

            case FRX_TOKEN_TYPE_LOGICAL_AND: ast_init(&new_node, FRX_AST_TYPE_LOGICAL_AND); break;
            case FRX_TOKEN_TYPE_LOGICAL_OR: ast_init(&new_node, FRX_AST_TYPE_LOGICAL_OR); break;

            case FRX_TOKEN_TYPE_BINARY_AND: ast_init(&new_node, FRX_AST_TYPE_BINARY_AND); break;
            case FRX_TOKEN_TYPE_BINARY_OR: ast_init(&new_node, FRX_AST_TYPE_BINARY_OR); break;
            case FRX_TOKEN_TYPE_BINARY_XOR: ast_init(&new_node, FRX_AST_TYPE_BINARY_XOR); break;
            case FRX_TOKEN_TYPE_BINARY_LEFT_SHIFT: ast_init(&new_node, FRX_AST_TYPE_BINARY_LEFT_SHIFT); break;
            case FRX_TOKEN_TYPE_BINARY_RIGHT_SHIFT: ast_init(&new_node, FRX_AST_TYPE_BINARY_RIGHT_SHIFT); break;

            case FRX_TOKEN_TYPE_COMPARISON: ast_init(&new_node, FRX_AST_TYPE_COMPARISON); break;

            default:
            {
                FRX_ASSERT(FRX_FALSE); //Since we only stay in the while loop as long as the next token is a binary operator,
                                       //we should never end up in this default case.

                break;
            }
        }

        FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, parser->current_token->type));

        if(parser_get_precedence(new_node.type) > parser_get_precedence(node->type) && !node_had_paranthesis)
        {
            AST temp;

            AST* right = &node->children[1];
            memcpy(&temp, right, sizeof(AST));

            memcpy(right, &new_node, sizeof(AST));

            AST* new_left = ast_new_child(right, FRX_AST_TYPE_NOOP);
            memcpy(new_left, &temp, sizeof(AST));

            AST* new_right = ast_new_child(right, FRX_AST_TYPE_NOOP);

            if(parser->current_token->type == FRX_TOKEN_TYPE_LEFT_PARANTHESIS)
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

            AST* left = ast_new_child(node, FRX_AST_TYPE_NOOP);
            memcpy(left, &temp, sizeof(AST));

            AST* right = ast_new_child(node, FRX_AST_TYPE_NOOP);

            if(parser->current_token->type == FRX_TOKEN_TYPE_LEFT_PARANTHESIS)
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

static FRX_NO_DISCARD b8 parser_parse_variable_declaration(Parser* parser, AST* node)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

    node->type = FRX_AST_TYPE_VARIABLE_DECLARATION;

    VariableData* data = memory_alloc(sizeof(VariableData), FRX_MEMORY_CATEGORY_UNKNOWN);
    node->data = data;

    strcpy(data->type, parser->current_token->identifier);

    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER));

    strcpy(data->name, parser->current_token->identifier);

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

    strcpy(data->type, parser->current_token->identifier);

    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER));

    strcpy(data->name, parser->current_token->identifier);

    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER));

    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_EQUALS));

    AST* child = ast_new_child(node, FRX_AST_TYPE_NOOP);

    FRX_PARSER_ABORT_ON_ERROR(parser_parse_expression(parser, child));

    return parser_eat(parser, FRX_TOKEN_TYPE_SEMICOLON);
}

static FRX_NO_DISCARD b8 parser_parse_return_statement(Parser* parser, AST* node)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

    node->type = FRX_AST_TYPE_RETURN_STATEMENT;

    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER));

    if(parser->current_token->type != FRX_TOKEN_TYPE_SEMICOLON)
    {
        AST* child = ast_new_child(node, FRX_AST_TYPE_NOOP);
        
        FRX_PARSER_ABORT_ON_ERROR(parser_parse_expression(parser, child));
    }

    return parser_eat(parser, FRX_TOKEN_TYPE_SEMICOLON);
}

static FRX_NO_DISCARD b8 parser_parse_statement(Parser* parser, AST* node)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

    if(parser->current_token->type == FRX_TOKEN_TYPE_IDENTIFIER)
    {
        if(strcmp(parser->current_token->identifier, "return") == 0)
            return parser_parse_return_statement(parser, node);

        if(parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_IDENTIFIER)
        {
            if(parser_peek(parser, 2)->type == FRX_TOKEN_TYPE_EQUALS)
                return parser_parse_variable_definition(parser, node);

            return parser_parse_variable_declaration(parser, node);
        }
    }

    FRX_ERROR_FILE("Could not parse token %s!", parser->lexer.filepath, parser->current_token->line, parser->current_token->coloumn, token_type_to_str(parser->current_token->type));

    return FRX_TRUE;
}

static FRX_NO_DISCARD b8 parser_parse_scope(Parser* parser, AST* node)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_BRACE));
    
    while(parser->current_token->type != FRX_TOKEN_TYPE_RIGHT_BRACE)
    {
        AST* child = ast_new_child(node, FRX_AST_TYPE_NOOP);
        FRX_PARSER_ABORT_ON_ERROR(parser_parse_statement(parser, child));
    }

    return parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_BRACE);
}

static FRX_NO_DISCARD b8 parser_parse_function_definition(Parser* parser, AST* node)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

    node->type = FRX_AST_TYPE_FUNCTION_DEFINITION;

    FunctionDefinitionData* data = memory_alloc(sizeof(FunctionDefinitionData), FRX_MEMORY_CATEGORY_UNKNOWN);
    node->data = data;

    strcpy(data->return_type, parser->current_token->identifier);

    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER));

    strcpy(data->name, parser->current_token->identifier);

    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER));

    AST* parameter_list = ast_new_child(node, FRX_AST_TYPE_PARAMETER_LIST);
    
    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_LEFT_PARANTHESIS));

    while(parser->current_token->type != FRX_TOKEN_TYPE_RIGHT_PARANTHESIS)
    {
        AST* parameter = ast_new_child(parameter_list, FRX_AST_TYPE_VARIABLE_DECLARATION);

        VariableData* data = memory_alloc(sizeof(VariableData), FRX_MEMORY_CATEGORY_UNKNOWN);
        parameter->data = data;

        strcpy(data->type, parser->current_token->identifier);

        FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER));

        strcpy(data->name, parser->current_token->identifier);

        FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_IDENTIFIER));
    
        if(parser->current_token->type != FRX_TOKEN_TYPE_RIGHT_PARANTHESIS)
        {
            FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_COMMA));
        }
    }
    
    FRX_PARSER_ABORT_ON_ERROR(parser_eat(parser, FRX_TOKEN_TYPE_RIGHT_PARANTHESIS));

    AST* body = ast_new_child(node, FRX_AST_TYPE_SCOPE);

    return parser_parse_scope(parser, body);
}

static FRX_NO_DISCARD b8 parser_parse_top_level(Parser* parser, AST* node)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(node != NULL);

    if(parser->current_token->type == FRX_TOKEN_TYPE_IDENTIFIER)
    {
        //TODO: Handle namespaces

        //TODO: Handle includes/imports

        //TODO: Handle struct definitions
    
        return parser_parse_function_definition(parser, node);
    }


    FRX_ERROR_FILE("Could not process token %s!", parser->lexer.filepath, parser->current_token->line, parser->current_token->coloumn, token_type_to_str(parser->current_token->type));

    return FRX_TRUE;
}

FRX_NO_DISCARD b8 parser_init(Parser* parser, const char* filepath)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(filepath != NULL);

    FRX_PARSER_ABORT_ON_ERROR(lexer_init(&parser->lexer, filepath));

    parser->current_token = lexer_peek(&parser->lexer, 0);

    return parser->current_token == NULL;
}

FRX_NO_DISCARD b8 parser_parse(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    ast_init(&parser->root, FRX_AST_TYPE_COMPOUND);

    while(parser->current_token->type != FRX_TOKEN_TYPE_EOF)
    {
        AST* child = ast_new_child(&parser->root, FRX_AST_TYPE_NOOP);

        FRX_PARSER_ABORT_ON_ERROR(parser_parse_top_level(parser, child));
    }

    return FRX_FALSE;
}
