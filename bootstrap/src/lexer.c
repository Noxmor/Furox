#include "lexer.h"

#include <string.h>
#include <ctype.h>

#include "core/assert.h"
#include "core/log.h"

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

static FRX_NO_DISCARD b8 lexer_read_token(Lexer* lexer, Token* token);

FRX_NO_DISCARD b8 lexer_init(Lexer* lexer, const char* filepath)
{
    FRX_ASSERT(lexer != NULL);

    FRX_ASSERT(filepath != NULL);

    lexer->file = fopen(filepath, "r");

    if(lexer->file == NULL)
    {
        FRX_ERROR("Failed to open file \"%s\"!", filepath);

        return FRX_TRUE;
    }

    lexer->filepath = filepath;

    lexer->line = 1;
    lexer->coloumn = 0;

    //This will automatically trigger the first lexer_read().
    lexer->buffer_index = FRX_LEXER_BUFFER_CAPACITY;

    if(lexer_read_token(lexer, &lexer->tokens[0]))
        return FRX_TRUE;

    lexer->tokens_count = 1;

    return FRX_FALSE;
}

Token* lexer_peek(Lexer* lexer, usize offset)
{
    FRX_ASSERT(offset < FRX_LEXER_TOKEN_CAPACITY);

    for(usize i = lexer->tokens_count; i <= offset; ++i)
    {
        if(lexer_read_token(lexer, &lexer->tokens[i]))
            return NULL;

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

static void lexer_skip_comment(Lexer* lexer)
{
    FRX_ASSERT(lexer != NULL);

    while(lexer_peek_char(lexer, 0) != '\n')
        lexer_advance(lexer);

    lexer_advance(lexer);
}

static b8 lexer_skip_comment_block(Lexer* lexer)
{
    FRX_ASSERT(lexer != NULL);

    usize line_start = lexer->line;
    usize coloumn_start = lexer->coloumn;

    usize comment_blocks_to_skip = 1;

    while(comment_blocks_to_skip > 0)
    {
        lexer_advance(lexer);

        char current = lexer_peek_char(lexer, 0);
        char next = lexer_peek_char(lexer, 1);

        if(current == '/' && next == '*')
        {
            line_start = lexer->line;
            coloumn_start = lexer->coloumn;

            ++comment_blocks_to_skip;

            lexer_advance(lexer);
            lexer_advance(lexer);
        }
        else if(current == '*' && next == '/')
        {
            --comment_blocks_to_skip;

            lexer_advance(lexer);
            lexer_advance(lexer);
        }

        if(lexer_peek_char(lexer, 0) == '\0')
        {
            FRX_ERROR_FILE("Reached end of file while skipping a comment block!", lexer->filepath, line_start, coloumn_start);

            return FRX_TRUE;
        }
    }

    return FRX_FALSE;
}

static void lexer_parse_identifier(Lexer* lexer, Token* token)
{
    FRX_ASSERT(lexer != NULL);
    
    FRX_ASSERT(token != NULL);

    token->type = FRX_TOKEN_TYPE_IDENTIFIER;

    usize identifier_index = 0;
    char current = lexer_peek_char(lexer, 0);
    
    while(isalnum(current) || current == '_')
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

    lexer_advance(lexer);
    lexer_advance(lexer);

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

    lexer_advance(lexer);
    lexer_advance(lexer);

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

static FRX_NO_DISCARD b8 lexer_parse_char_literal(Lexer* lexer, Token* token)
{
    FRX_ASSERT(lexer != NULL);

    FRX_ASSERT(token != NULL);

    token->type = FRX_TOKEN_TYPE_CHAR_LITERAL;

    lexer_advance(lexer);

    token->identifier[0] = lexer_peek_char(lexer, 0);
    token->identifier[1] = '\0';

    lexer_advance(lexer);

    if(token->identifier[0] == '\\')
    {
        token->identifier[2] = '\0';

        switch(lexer_peek_char(lexer, 0))
        {
            case '0':
            case 'n':
            case 't':
            case 'v':
            case 'b':
            case 'r':
            case 'f':
            case 'a':
            case '\\':
            case '\'':
            case '"': token->identifier[1] = lexer_peek_char(lexer, 0); break;

            default:
            {
                FRX_ERROR_FILE("'\\%c' is not a valid escaped character!", lexer->filepath, token->line, token->coloumn, token->identifier[0]);

                return FRX_TRUE;
            }
        }

        lexer_advance(lexer);
    }

    if(lexer_peek_char(lexer, 0) != '\'')
    {
        FRX_ERROR_FILE("Missing ' after char literal!", lexer->filepath, token->line, token->coloumn);

        return FRX_TRUE;
    }

    lexer_advance(lexer);

    return FRX_FALSE;
}

static FRX_NO_DISCARD b8 lexer_parse_string_literal(Lexer* lexer, Token* token)
{
    FRX_ASSERT(lexer != NULL);

    FRX_ASSERT(token != NULL);

    token->type = FRX_TOKEN_TYPE_STRING_LITERAL;

    lexer_advance(lexer);

    usize identifier_index = 0;
    char current = '\0';
    while((current = lexer_peek_char(lexer, 0)) != '"')
    {
        if(current == '\0')
        {
            FRX_ERROR_FILE("Reached end of file while parsing string literal!", lexer->filepath, token->line, token->coloumn);

            return FRX_TRUE;
        }

        token->identifier[identifier_index++] = current;

        lexer_advance(lexer);
    }

    token->identifier[identifier_index] = '\0';

    lexer_advance(lexer);

    return FRX_FALSE;
}

static FRX_NO_DISCARD b8 lexer_read_token(Lexer* lexer, Token* token)
{
    FRX_ASSERT(lexer != NULL);

    FRX_ASSERT(token != NULL);
    
    lexer_skip_whitespaces(lexer);

    token->line = lexer->line;
    token->coloumn = lexer->coloumn;

    char current = lexer_peek_char(lexer, 0);

    if(isalpha(current))
    {
        lexer_parse_identifier(lexer, token);
        return FRX_FALSE;
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
        
        return FRX_FALSE;
    }

    switch(current)
    {
        case '\'': return lexer_parse_char_literal(lexer, token);
        case '"': return lexer_parse_string_literal(lexer, token);

        case '+': token->type = FRX_TOKEN_TYPE_PLUS; break;
        case '-': token->type = FRX_TOKEN_TYPE_MINUS; break;
        case '*': token->type = FRX_TOKEN_TYPE_STAR; break;
        case '/':
        {
            if(lexer_peek_char(lexer, 1) == '/')
            {
                lexer_skip_comment(lexer);

                return lexer_read_token(lexer, token);
            }

            if(lexer_peek_char(lexer, 1) == '*')
            {
                lexer_skip_comment_block(lexer);

                return lexer_read_token(lexer, token);
            }

            token->type = FRX_TOKEN_TYPE_SLASH;

            break;
        }

        case '%': token->type = FRX_TOKEN_TYPE_MODULO; break;

        case '!': token->type = FRX_TOKEN_TYPE_LOGICAL_NEGATION; break;

        case '&':
        {
            if(lexer_peek_char(lexer, 1) == '&')
            {
                token->type = FRX_TOKEN_TYPE_LOGICAL_AND;
                strcpy(token->identifier, "&&");

                lexer_advance(lexer);
                lexer_advance(lexer);

                return FRX_FALSE;
            }

            token->type = FRX_TOKEN_TYPE_BINARY_AND;

            break;
        }

        case '|':
        {
            if(lexer_peek_char(lexer, 1) == '|')
            {
                token->type = FRX_TOKEN_TYPE_LOGICAL_OR;
                strcpy(token->identifier, "||");

                lexer_advance(lexer);
                lexer_advance(lexer);

                return FRX_FALSE;
            }

            token->type = FRX_TOKEN_TYPE_BINARY_OR;

            break;
        }

        case '^': token->type = FRX_TOKEN_TYPE_BINARY_XOR; break;
        case '~': token->type = FRX_TOKEN_TYPE_BINARY_NEGATION; break;

        case '<':
        {
            if(lexer_peek_char(lexer, 1) == '<')
            {
                token->type = FRX_TOKEN_TYPE_BINARY_LEFT_SHIFT;
                strcpy(token->identifier, "<<");

                lexer_advance(lexer);
                lexer_advance(lexer);

                return FRX_FALSE;
            }

            if(lexer_peek_char(lexer, 1) == '=')
            {
                token->type = FRX_TOKEN_TYPE_LESS_THAN_EQUALS;
                strcpy(token->identifier, "<=");

                lexer_advance(lexer);
                lexer_advance(lexer);

                return FRX_FALSE;
            }

            token->type = FRX_TOKEN_TYPE_LESS_THAN;

            break;
        }

        case '>':
        {
            if(lexer_peek_char(lexer, 1) == '>')
            {
                token->type = FRX_TOKEN_TYPE_BINARY_RIGHT_SHIFT;
                strcpy(token->identifier, ">>");

                lexer_advance(lexer);
                lexer_advance(lexer);

                return FRX_FALSE;
            }

            if(lexer_peek_char(lexer, 1) == '=')
            {
                token->type = FRX_TOKEN_TYPE_GREATER_THAN_EQUALS;
                strcpy(token->identifier, ">=");

                lexer_advance(lexer);
                lexer_advance(lexer);

                return FRX_FALSE;
            }

            token->type = FRX_TOKEN_TYPE_GREATER_THAN;

            break;
        }

        case '=':
        {
            if(lexer_peek_char(lexer, 1) == '=')
            {
                token->type = FRX_TOKEN_TYPE_COMPARISON;
                strcpy(token->identifier, "==");

                lexer_advance(lexer);
                lexer_advance(lexer);

                return FRX_FALSE;
            }

            token->type = FRX_TOKEN_TYPE_EQUALS;

            break;
        }

        case '(': token->type = FRX_TOKEN_TYPE_LEFT_PARANTHESIS; break;   
        case ')': token->type = FRX_TOKEN_TYPE_RIGHT_PARANTHESIS; break;   
        
        case '[': token->type = FRX_TOKEN_TYPE_LEFT_BRACKET; break;   
        case ']': token->type = FRX_TOKEN_TYPE_RIGHT_BRACKET; break;   
        
        case '{': token->type = FRX_TOKEN_TYPE_LEFT_BRACE; break;   
        case '}': token->type = FRX_TOKEN_TYPE_RIGHT_BRACE; break;

        case ',': token->type = FRX_TOKEN_TYPE_COMMA; break;
        case '.': token->type = FRX_TOKEN_TYPE_DOT; break;

        case ':':
        {
            if(lexer_peek_char(lexer, 1) == ':')
            {
                token->type = FRX_TOKEN_TYPE_NAMESPACE_RESOLUTION;
                strcpy(token->identifier, "::");

                lexer_advance(lexer);
                lexer_advance(lexer);

                return FRX_FALSE;
            }

            token->type = FRX_TOKEN_TYPE_COLON;

            break;
        }

        case ';': token->type = FRX_TOKEN_TYPE_SEMICOLON; break;

        case '\0': token->type = FRX_TOKEN_TYPE_EOF; break;

        default:
        {
            FRX_ERROR_FILE("Could not process '%c'!", lexer->filepath, lexer->line, lexer->coloumn, current);

            return FRX_TRUE;
        }
    }

    token->identifier[0] = current;
    token->identifier[1] = '\0';

    lexer_advance(lexer);

    return FRX_FALSE;
}

FRX_NO_DISCARD b8 lexer_next_token(Lexer* lexer)
{
    FRX_ASSERT(lexer != NULL);

    for(usize i = 1; i < lexer->tokens_count; ++i)
        memcpy(&lexer->tokens[i - 1], &lexer->tokens[i], sizeof(Token));

    if(lexer->tokens_count == 1)
        return lexer_read_token(lexer, &lexer->tokens[0]);

    --lexer->tokens_count;

    return FRX_FALSE;
}

void lexer_destroy(Lexer* lexer)
{
    FRX_ASSERT(lexer != NULL);

    fclose(lexer->file);
}
