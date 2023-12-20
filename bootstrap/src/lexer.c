#include "lexer.h"

#include <string.h>
#include <ctype.h>

#include "core/assert.h"

typedef struct LexerInfo
{
    usize lines_processed;
} LexerInfo;

static LexerInfo lexer_info;

static void lexer_read(Lexer* lexer)
{
    FRX_ASSERT(lexer != NULL);

    FRX_ASSERT(lexer->buffer_index <= FRX_LEXER_BUFFER_CAPACITY);

    usize characters_to_copy = lexer->buffer_index == 0 ? 0 : FRX_LEXER_BUFFER_CAPACITY - lexer->buffer_index;

    memcpy(lexer->buffer, &lexer->buffer[lexer->buffer_index], characters_to_copy * sizeof(char));

    usize characters_read = 0;
    if((characters_read = fread(&lexer->buffer[characters_to_copy], sizeof(char), FRX_LEXER_BUFFER_CAPACITY - characters_to_copy, lexer->file)) < FRX_LEXER_BUFFER_CAPACITY - characters_to_copy)
    {
        lexer->buffer[characters_to_copy + characters_read] = '\0';

        //Since the last line of a file does not contain a '\n' at the end, we need to manually count this line.
        ++lexer_info.lines_processed;
    }

    lexer->buffer_index = 0;
}

static void lexer_read_token(Lexer* lexer, Token* token);

void lexer_init(Lexer* lexer, const char* filepath)
{
    FRX_ASSERT(lexer != NULL);

    FRX_ASSERT(filepath != NULL);

    lexer->file = fopen(filepath, "r");

    lexer->line = 1;
    lexer->coloumn = 0;

    //This will automatically trigger the first lexer_read().
    lexer->buffer_index = FRX_LEXER_BUFFER_CAPACITY;

    lexer_read_token(lexer, &lexer->tokens[0]);

    lexer->tokens_count = 1;
}

Token* lexer_peek(Lexer* lexer, usize offset)
{
    FRX_ASSERT(offset < FRX_LEXER_TOKEN_CAPACITY);

    for(usize i = lexer->tokens_count; i <= offset; ++i)
    {
        lexer_read_token(lexer, &lexer->tokens[i]);
        ++lexer->tokens_count;
    }

    return &lexer->tokens[offset];
}

static char lexer_peek_char(Lexer* lexer, usize offset)
{
    FRX_ASSERT(lexer != NULL);

    FRX_ASSERT(offset < FRX_LEXER_BUFFER_CAPACITY);

    if(lexer->buffer_index + offset >= FRX_LEXER_BUFFER_CAPACITY)
        lexer_read(lexer);

    for(usize i = lexer->buffer_index; i < lexer->buffer_index + offset; ++i)
    {
        if(lexer->buffer[i] == '\0')
            return '\0';
    }

    return lexer->buffer[lexer->buffer_index + offset];
}

static void lexer_advance(Lexer* lexer)
{
    FRX_ASSERT(lexer != NULL);

    char current = lexer_peek_char(lexer, 0);

    if(current == '\0')
        return;

    if(current == '\n')
    {
        ++lexer->line;
        lexer->coloumn = 0;

        ++lexer_info.lines_processed;
    }
    else
        ++lexer->coloumn;

    ++lexer->buffer_index;

    if(lexer->buffer_index >= FRX_LEXER_BUFFER_CAPACITY)
        lexer_read(lexer);
}

static void lexer_skip_whitespaces(Lexer* lexer)
{
    FRX_ASSERT(lexer != NULL);

    char current = lexer_peek_char(lexer, 0);
    while(current == ' ' || current == '\t' || current == '\n' || current == '\r')
    {
        lexer_advance(lexer);
        current = lexer_peek_char(lexer, 0);
    }
}

static void lexer_parse_identifier(Lexer* lexer, Token* token)
{
    FRX_ASSERT(lexer != NULL);
    
    FRX_ASSERT(token != NULL);

    token->type = FRX_TOKEN_TYPE_IDENTIFIER;

    usize identifier_index = 0;
    char current = lexer_peek_char(lexer, 0);
    
    while(isalpha(current))
    {
        token->identifier[identifier_index++] = current;
        lexer_advance(lexer);
        current = lexer_peek_char(lexer, 0);
    }

    token->identifier[identifier_index] = '\0';
}

static void lexer_parse_binary_number(Lexer* lexer, Token* token)
{
    FRX_ASSERT(lexer != NULL);

    FRX_ASSERT(token != NULL);

    token->type = FRX_TOKEN_TYPE_NUMBER;

    token->number = 0;

    char current = lexer_peek_char(lexer, 0);
    while(current == '0' || current == '1')
    {
        token->number = token->number * 2 + (current - '0');
        lexer_advance(lexer);
        current = lexer_peek_char(lexer, 0);
    }
}

static void lexer_parse_hex_number(Lexer* lexer, Token* token)
{
    FRX_ASSERT(lexer != NULL);

    FRX_ASSERT(token != NULL);

    token->type = FRX_TOKEN_TYPE_NUMBER;

    token->number = 0;

    char current = toupper(lexer_peek_char(lexer, 0));
    while(isdigit(current) || (current >= 'A' && current <= 'F'))
    {
        if(isdigit(current))
            token->number = token->number * 16 + (current - '0');
        else
            token->number = token->number * 16 + (current - 'A' + 10);

        lexer_advance(lexer);
        current = lexer_peek_char(lexer, 0);
    }
}

static void lexer_parse_number(Lexer* lexer, Token* token)
{
    FRX_ASSERT(lexer != NULL);

    FRX_ASSERT(token != NULL);

    token->type = FRX_TOKEN_TYPE_NUMBER;

    token->number = 0;

    char current = lexer_peek_char(lexer, 0);
    while(isdigit(current))
    {
        token->number = token->number * 10 + (current - '0');

        lexer_advance(lexer);
        current = lexer_peek_char(lexer, 0);
    }
}

static void lexer_read_token(Lexer* lexer, Token* token)
{
    FRX_ASSERT(lexer != NULL);

    FRX_ASSERT(token != NULL);
    
    lexer_skip_whitespaces(lexer);

    char current = lexer_peek_char(lexer, 0);

    if(isalpha(current))
    {
        lexer_parse_identifier(lexer, token);
        return;
    }

    if(isdigit(current))
    {
        char next = lexer_peek_char(lexer, 1);
        
        if(current == '0' && next == 'b')
            lexer_parse_binary_number(lexer, token);
        else if(current == '0' && next == 'x')
            lexer_parse_hex_number(lexer, token);
        else
            lexer_parse_number(lexer, token);
        
        return;
    }

    switch(current)
    {
        case '+': token->type = FRX_TOKEN_TYPE_PLUS; break;
        case '-': token->type = FRX_TOKEN_TYPE_MINUS; break;
        case '*': token->type = FRX_TOKEN_TYPE_STAR; break;
        case '/': token->type = FRX_TOKEN_TYPE_SLASH; break;

        case '(': token->type = FRX_TOKEN_TYPE_LEFT_PARANTHESIS; break;   
        case ')': token->type = FRX_TOKEN_TYPE_RIGHT_PARANTHESIS; break;   
        
        case '[': token->type = FRX_TOKEN_TYPE_LEFT_BRACKET; break;   
        case ']': token->type = FRX_TOKEN_TYPE_RIGHT_BRACKET; break;   
        
        case '{': token->type = FRX_TOKEN_TYPE_LEFT_BRACE; break;   
        case '}': token->type = FRX_TOKEN_TYPE_RIGHT_BRACE; break;

        case ',': token->type = FRX_TOKEN_TYPE_COMMA; break;
        case ':': token->type = FRX_TOKEN_TYPE_COLON; break;
        case ';': token->type = FRX_TOKEN_TYPE_SEMICOLON; break;

        case '\0': token->type = FRX_TOKEN_TYPE_EOF; break;

        default: break; //TODO: Throw error that the current character could not be processed!   
    }

    token->identifier[0] = current;
    token->identifier[1] = '\0';

    lexer_advance(lexer);
}

void lexer_next_token(Lexer* lexer)
{
    FRX_ASSERT(lexer != NULL);

    for(usize i = 1; i < lexer->tokens_count; ++i)
        memcpy(&lexer->tokens[i - 1], &lexer->tokens[i], sizeof(Token));

    if(lexer->tokens_count == 1)
        lexer_read_token(lexer, &lexer->tokens[0]);
    else
        --lexer->tokens_count;
}

void lexer_destroy(Lexer* lexer)
{
    FRX_ASSERT(lexer != NULL);

    fclose(lexer->file);
}
