#include "lexer.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "assert.h"
#include "log.h"
#include "hash.h"
#include "string_table.h"

#define FRX_KEYWORD_TABLE_SIZE 512
#define FRX_LEXER_IDENT_INITIAL_SIZE 4

typedef struct KeywordTableEntry
{
    char* name;
    TokenType type;
} KeywordTableEntry;

typedef struct KeywordTable
{
    KeywordTableEntry entries[FRX_KEYWORD_TABLE_SIZE];
} KeywordTable;

static KeywordTable keyword_table;

static void register_keyword(char* name, TokenType type)
{
    FRX_ASSERT(name != NULL);
    FRX_ASSERT(type < FRX_TOKEN_TYPE_COUNT);

    u64 index = hash_djb2(name) % FRX_KEYWORD_TABLE_SIZE;

    KeywordTableEntry* entry = &keyword_table.entries[index];

    FRX_ASSERT(entry->name == NULL);

    entry->name = name;
    entry->type = type;
}

static KeywordTableEntry* keyword_table_find(const char* name)
{
    FRX_ASSERT(name != NULL);

    u64 index = hash_djb2(name) % FRX_KEYWORD_TABLE_SIZE;
    return &keyword_table.entries[index];
}

static void lexer_grow_identifier_placeholder(Lexer* lexer)
{
    FRX_ASSERT(lexer != NULL);

    lexer->identifier_placeholder_size = lexer->identifier_placeholder_size * 2;
    lexer->identifier_placeholder = realloc(lexer->identifier_placeholder,
            sizeof(char) * lexer->identifier_placeholder_size);
}

void lexer_init_keyword_table(void)
{
    FRX_LOG_INFO("Initializing keyword table...");

    for (TokenType type = FRX_TOKEN_TYPE_FIRST_KEYWORD; type <= FRX_TOKEN_TYPE_LAST_KEYWORD;
        ++type)
    {
        register_keyword(token_type_to_str(type), type);
    }
}

static void lexer_read_token(Lexer* lexer, Token* token);

void lexer_read(Lexer* lexer)
{
    FRX_ASSERT(lexer != NULL);
    FRX_ASSERT(lexer->buffer_index <= FRX_LEXER_BUFFER_CAPACITY);

    usize characters_to_copy = 0;
    if (lexer->buffer_index > 0)
    {
        characters_to_copy = FRX_LEXER_BUFFER_CAPACITY - lexer->buffer_index;
    }

    memcpy(lexer->buffer, &lexer->buffer[lexer->buffer_index], characters_to_copy * sizeof(char));

    usize characters_read = 0;
    if ((characters_read = fread(&lexer->buffer[characters_to_copy],
                    sizeof(char), FRX_LEXER_BUFFER_CAPACITY - characters_to_copy,
                    lexer->file)) < FRX_LEXER_BUFFER_CAPACITY - characters_to_copy)
    {
        lexer->buffer[characters_to_copy + characters_read] = '\0';
    }

    lexer->buffer_index = 0;
}

void lexer_init(Lexer* lexer, const char* filepath)
{
    FRX_ASSERT(lexer != NULL);
    FRX_ASSERT(filepath != NULL);

    lexer->failed = FRX_FALSE;

    lexer->identifier_placeholder = malloc(sizeof(char) *FRX_LEXER_IDENT_INITIAL_SIZE);
    lexer->identifier_placeholder_size = FRX_LEXER_IDENT_INITIAL_SIZE;

    lexer->file = fopen(filepath, "r");

    if(lexer->file == NULL)
    {
        lexer_fail(lexer);
    }

    lexer->filepath = strdup(filepath);

    lexer->location.pos = 0;
    lexer->location.line = 1;
    lexer->location.column = 1;

    //This will automatically trigger the first lexer_read()
    lexer->buffer_index = FRX_LEXER_BUFFER_CAPACITY;

    lexer_read_token(lexer, &lexer->tokens[0]);

    lexer->tokens_count = 1;
}

void lexer_fail(Lexer* lexer)
{
    FRX_ASSERT(lexer != NULL);

    lexer->failed = FRX_TRUE;
}

b8 lexer_failed(const Lexer* lexer)
{
    FRX_ASSERT(lexer != NULL);

    return lexer->failed;
}

Token* lexer_peek(Lexer* lexer, usize offset)
{
    FRX_ASSERT(lexer != NULL);
    FRX_ASSERT(offset < FRX_LEXER_TOKEN_CAPACITY);

    for (usize i = lexer->tokens_count; i <= offset; ++i)
    {
        lexer_read_token(lexer, &lexer->tokens[i]);

        lexer->tokens_count = lexer->tokens_count + 1;
    }

    return &lexer->tokens[offset];
}

Token* lexer_current_token(Lexer* lexer)
{
    return lexer_peek(lexer, 0);
}

char lexer_peek_char(Lexer* lexer, usize offset)
{
    FRX_ASSERT(lexer != NULL);
    FRX_ASSERT(offset < FRX_LEXER_BUFFER_CAPACITY);

    if(lexer->buffer_index + offset >= FRX_LEXER_BUFFER_CAPACITY)
    {
        lexer_read(lexer);
    }

    for (usize i = lexer->buffer_index; i < lexer->buffer_index + offset; ++i)
    {
        if(lexer->buffer[i] == '\0')
        {
            return '\0';
        }
    }

    return lexer->buffer[lexer->buffer_index + offset];
}

static char lexer_current_char(Lexer* lexer)
{
    return lexer_peek_char(lexer, 0);
}

static void lexer_advance_n(Lexer* lexer, usize count)
{
    FRX_ASSERT(lexer != NULL);

    for (usize i = 0; i < count; ++i)
    {
        char current = lexer_current_char(lexer);

        if (current == '\0')
        {
            return;
        }

        if (current == '\n')
        {
            lexer->location.line = lexer->location.line + 1;
            lexer->location.column = 1;
        }
        else
        {
            lexer->location.column = lexer->location.column + 1;
        }

        ++lexer->buffer_index;
        ++lexer->location.pos;

        if (lexer->buffer_index >= FRX_LEXER_BUFFER_CAPACITY)
        {
            lexer_read(lexer);
        }
    }
}

static void lexer_advance(Lexer* lexer)
{
    lexer_advance_n(lexer, 1);
}

static void lexer_skip_whitespaces(Lexer* lexer)
{
    char current = lexer_current_char(lexer);
    while (current == ' ' || current == '\t' || current == '\n' || current == '\r')
    {
        lexer_advance(lexer);
        current = lexer_current_char(lexer);
    }
}

static void lexer_skip_comment(Lexer* lexer)
{
    while (lexer_current_char(lexer) != '\n')
    {
        lexer_advance(lexer);
    }

    lexer_advance(lexer);
}

static void lexer_skip_comment_block(Lexer* lexer)
{
    FRX_ASSERT(lexer != NULL);

    usize comment_blocks_to_skip = 1;

    while (comment_blocks_to_skip > 0)
    {
        lexer_advance(lexer);

        char current = lexer_current_char(lexer);
        char next = lexer_peek_char(lexer, 1);

        if (current == '/' && next == '*')
        {
            ++comment_blocks_to_skip;

            lexer_advance_n(lexer, 2);
        }
        else if (current == '*' && next == '/')
        {
            --comment_blocks_to_skip;

            lexer_advance_n(lexer, 2);
        }

        if (lexer_current_char(lexer) == '\0')
        {
            lexer_fail(lexer);
        }
    }
}

static void lexer_parse_identifier(Lexer* lexer, Token* token)
{
    FRX_ASSERT(lexer != NULL);
    FRX_ASSERT(token != NULL);

    token->type = FRX_TOKEN_TYPE_IDENT;

    usize identifier_index = 0;
    char current = lexer_current_char(lexer);

    while (isalnum(current) || current == '_')
    {
        lexer->identifier_placeholder[identifier_index++] = current;

        if(identifier_index >= lexer->identifier_placeholder_size)
        {
            lexer_grow_identifier_placeholder(lexer);
        }

        token->range.end = lexer->location;

        lexer_advance(lexer);
        current = lexer_current_char(lexer);
    }

    lexer->identifier_placeholder[identifier_index] = '\0';

    token->identifier = string_table_intern(lexer->identifier_placeholder);

    KeywordTableEntry* entry = keyword_table_find(token->identifier);

    if(entry->name != NULL && strcmp(entry->name, token->identifier) == 0)
    {
        token->type = entry->type;
    }
}

static void lexer_parse_binary_number(Lexer* lexer, Token* token)
{
    FRX_ASSERT(lexer != NULL);
    FRX_ASSERT(token != NULL);

    token->type = FRX_TOKEN_TYPE_INT_LIT;

    lexer_advance_n(lexer, 2);

    token->int_literal = 0;

    char current = lexer_current_char(lexer);
    while (current == '0' || current == '1')
    {
        token->int_literal = token->int_literal * 2 + (current - '0');

        token->range.end = lexer->location;

        lexer_advance(lexer);
        current = lexer_current_char(lexer);
    }
}

static void lexer_parse_hex_number(Lexer* lexer, Token* token)
{
    FRX_ASSERT(lexer != NULL);
    FRX_ASSERT(token != NULL);

    token->type = FRX_TOKEN_TYPE_INT_LIT;

    lexer_advance_n(lexer, 2);

    token->int_literal = 0;

    char current = lexer_current_char(lexer);
    char lower = current | 0x20;
    while (isdigit(current) || (lower >= 'a' && lower <= 'f'))
    {
        if (isdigit(current))
        {
            token->int_literal = token->int_literal * 16 + (current - '0');
        }
        else
        {
            static char bit_to_letter[2] = { 'a', 'A' };
            token->int_literal = token->int_literal * 16 + (current - bit_to_letter[current & 0x20] + 10);
        }

        token->range.end = lexer->location;

        lexer_advance(lexer);
        current = lexer_current_char(lexer);
    }
}

static void lexer_parse_number(Lexer* lexer, Token* token)
{
    FRX_ASSERT(lexer != NULL);
    FRX_ASSERT(token != NULL);

    token->type = FRX_TOKEN_TYPE_INT_LIT;

    token->int_literal = 0;

    char current = lexer_current_char(lexer);
    while (isdigit(current))
    {
        token->int_literal = token->int_literal * 10 + (current - '0');

        token->range.end = lexer->location;

        lexer_advance(lexer);
        current = lexer_current_char(lexer);
    }

    if (current == '.')
    {
        lexer_advance(lexer);
        current = lexer_current_char(lexer);

        token->type = FRX_TOKEN_TYPE_FLOAT_LIT;

        u64 integer = token->int_literal;
        u64 decimal = 0;

        while (isdigit(current))
        {
            decimal = decimal * 10 + (current - '0');

            token->range.end = lexer->location;

            lexer_advance(lexer);
            current = lexer_current_char(lexer);
        }

        token->float_literal = (integer << 32) | decimal;

        if (current == 'f')
        {
            lexer_advance(lexer);
        }
    }
    else if (current == 'f')
    {
        token->type = FRX_TOKEN_TYPE_FLOAT_LIT;
        token->float_literal = token->int_literal << 32;

        lexer_advance(lexer);
    }
}

static void lexer_parse_char_literal(Lexer* lexer, Token* token)
{
    FRX_ASSERT(lexer != NULL);
    FRX_ASSERT(token != NULL);

    token->type = FRX_TOKEN_TYPE_CHAR_LIT;

    lexer_advance(lexer);

    lexer->identifier_placeholder[0] = lexer_current_char(lexer);
    lexer->identifier_placeholder[1] = '\0';

    lexer_advance(lexer);

    if(lexer->identifier_placeholder[0] == '\\')
    {
        lexer->identifier_placeholder[2] = '\0';

        switch(lexer_current_char(lexer))
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
            case '"':
            {
                lexer->identifier_placeholder[1] = lexer_current_char(lexer);
                break;
            }
            default:
            {
                lexer_fail(lexer);
            }
        }

        lexer_advance(lexer);
    }

    token->identifier = string_table_intern(lexer->identifier_placeholder);

    if (lexer_current_char(lexer) != '\'')
    {
        lexer_fail(lexer);
    }

    token->range.end = lexer->location;

    lexer_advance(lexer);
}

static void lexer_parse_string_literal(Lexer* lexer, Token* token)
{
    FRX_ASSERT(lexer != NULL);
    FRX_ASSERT(token != NULL);

    token->type = FRX_TOKEN_TYPE_STR_LIT;

    lexer_advance(lexer);

    usize identifier_index = 0;
    char current = '\0';
    while ((current = lexer_current_char(lexer)) != '"')
    {
        if (current == '\0')
        {
            lexer_fail(lexer);
        }

        lexer->identifier_placeholder[identifier_index++] = current;

        if (identifier_index >= lexer->identifier_placeholder_size)
        {
            lexer_grow_identifier_placeholder(lexer);
        }

        lexer_advance(lexer);
    }

    lexer->identifier_placeholder[identifier_index] = '\0';

    token->identifier = string_table_intern(lexer->identifier_placeholder);

    token->range.end = lexer->location;

    lexer_advance(lexer);
}

static void lexer_read_token(Lexer* lexer, Token* token)
{
    FRX_ASSERT(lexer != NULL);
    FRX_ASSERT(token != NULL);

    if (lexer_failed(lexer))
    {
        token->type = FRX_TOKEN_TYPE_EOF;
        return;
    }

    lexer_skip_whitespaces(lexer);

    token->range.start = lexer->location;

    char current = lexer_current_char(lexer);

    if (isalpha(current))
    {
        lexer_parse_identifier(lexer, token);
        return;
    }

    if (isdigit(current))
    {
        char next = lexer_peek_char(lexer, 1);

        if (current == '0' && next == 'b')
        {
            lexer_parse_binary_number(lexer, token);
        }
        else if (current == '0' && next == 'x')
        {
            lexer_parse_hex_number(lexer, token);
        }
        else
        {
            lexer_parse_number(lexer, token);
        }

        return;
    }

    switch (current)
    {
        case '\'': lexer_parse_char_literal(lexer, token); return;
        case '"': lexer_parse_string_literal(lexer, token); return;
        case '+':
        {
            token->type = FRX_TOKEN_TYPE_PLUS;

            char next = lexer_peek_char(lexer, 1);

            if (next == '=')
            {
                token->type = FRX_TOKEN_TYPE_PLUS_EQ;
                lexer_advance(lexer);
            }
            else if (next == '+')
            {
                token->type = FRX_TOKEN_TYPE_PLUS_PLUS;
                lexer_advance(lexer);
            }

            break;
        }
        case '-':
        {
            token->type = FRX_TOKEN_TYPE_MINUS;

            char next = lexer_peek_char(lexer, 1);

            if (next == '>')
            {
                token->type = FRX_TOKEN_TYPE_ARROW;
                lexer_advance(lexer);
            }
            else if (next == '-')
            {
                token->type = FRX_TOKEN_TYPE_MINUS_MINUS;
                lexer_advance(lexer);
            }

            break;
        }
        case '*':
        {
            token->type = FRX_TOKEN_TYPE_STAR;

            if (lexer_peek_char(lexer, 1) == '=')
            {
                token->type = FRX_TOKEN_TYPE_STAR_EQ;
                lexer_advance(lexer);
            }

            break;
        }
        case '/':
        {
            token->type = FRX_TOKEN_TYPE_SLASH;

            char next = lexer_peek_char(lexer, 1);

            if (next == '=')
            {
                token->type = FRX_TOKEN_TYPE_SLASH_EQ;
                lexer_advance(lexer);
            }
            else if (next == '/')
            {
                lexer_skip_comment(lexer);
                lexer_read_token(lexer, token);

                return;
            }
            else if (next == '*')
            {
                lexer_skip_comment_block(lexer);
                lexer_read_token(lexer, token);

                return;
            }

            break;
        }
        case '%':
        {
            token->type = FRX_TOKEN_TYPE_MODULO;

            if(lexer_peek_char(lexer, 1) == '=')
            {
                token->type = FRX_TOKEN_TYPE_MODULO_EQ;
                lexer_advance(lexer);
            }

            break;
        }
        case '!':
        {
            token->type = FRX_TOKEN_TYPE_LOG_NOT;

            if (lexer_peek_char(lexer, 1) == '=')
            {
                token->type = FRX_TOKEN_TYPE_LOG_NEQ;
                lexer_advance(lexer);
            }

            break;
        }
        case '&':
        {
            token->type = FRX_TOKEN_TYPE_BIT_AND;

            char next = lexer_peek_char(lexer, 1);

            if (next == '&')
            {
                token->type = FRX_TOKEN_TYPE_LOG_AND;
                lexer_advance(lexer);
            }
            else if (next == '=')
            {
                token->type = FRX_TOKEN_TYPE_BIT_AND_EQ;
                lexer_advance(lexer);
            }

            break;
        }
        case '|':
        {
            token->type = FRX_TOKEN_TYPE_BIT_OR;

            char next = lexer_peek_char(lexer, 1);

            if (next == '|')
            {
                token->type = FRX_TOKEN_TYPE_LOG_OR;
                lexer_advance(lexer);
            }
            else if (next == '=')
            {
                token->type = FRX_TOKEN_TYPE_BIT_OR_EQ;
                lexer_advance(lexer);
            }

            break;
        }
        case '^':
        {
            token->type = FRX_TOKEN_TYPE_BIT_XOR;

            if (lexer_peek_char(lexer, 1) == '=')
            {
                token->type = FRX_TOKEN_TYPE_BIT_XOR_EQ;
                lexer_advance(lexer);
            }

            break;
        }
        case '~':
        {
            token->type = FRX_TOKEN_TYPE_BIT_NOT;

            break;
        }
        case '<':
        {
            token->type = FRX_TOKEN_TYPE_LT;

            char next = lexer_peek_char(lexer, 1);

            if (next == '<')
            {
                token->type = FRX_TOKEN_TYPE_BIT_LSHIFT;
                lexer_advance(lexer);

                if (lexer_peek_char(lexer, 1) == '=')
                {
                    token->type = FRX_TOKEN_TYPE_BIT_LSHIFT_EQ;
                    lexer_advance(lexer);
                }
            }
            else if (next == '=')
            {
                token->type = FRX_TOKEN_TYPE_LEQ;
                lexer_advance(lexer);
            }

            break;
        }
        case '>':
        {
            token->type = FRX_TOKEN_TYPE_GT;

            char next = lexer_peek_char(lexer, 1);

            if (next == '>')
            {
                token->type = FRX_TOKEN_TYPE_BIT_RSHIFT;
                lexer_advance(lexer);

                if (lexer_peek_char(lexer, 1) == '=')
                {
                    token->type = FRX_TOKEN_TYPE_BIT_RSHIFT_EQ;
                    lexer_advance(lexer);
                }
            }
            else if (next == '=')
            {
                token->type = FRX_TOKEN_TYPE_GEQ;
                lexer_advance(lexer);
            }

            break;
        }
        case '=':
        {
            token->type = FRX_TOKEN_TYPE_EQ;

            if(lexer_peek_char(lexer, 1) == '=')
            {
                token->type = FRX_TOKEN_TYPE_LOG_EQ;
                lexer_advance(lexer);
            }

            break;
        }
        case '(': token->type = FRX_TOKEN_TYPE_LPAREN; break;
        case ')': token->type = FRX_TOKEN_TYPE_RPAREN; break;
        case '[': token->type = FRX_TOKEN_TYPE_LBRACKET; break;
        case ']': token->type = FRX_TOKEN_TYPE_RBRACKET; break;
        case '{': token->type = FRX_TOKEN_TYPE_LBRACE; break;
        case '}': token->type = FRX_TOKEN_TYPE_RBRACE; break;
        case ',': token->type = FRX_TOKEN_TYPE_COMMA; break;
        case '.':
        {
            token->type = FRX_TOKEN_TYPE_DOT;

            if (lexer_peek_char(lexer, 1) == '.' && lexer_peek_char(lexer, 2) == '.')
            {
                token->type = FRX_TOKEN_TYPE_ELLIPSIS;
                lexer_advance_n(lexer, 2);
            }

            break;
        }
        case ':':
        {
            token->type = FRX_TOKEN_TYPE_COLON;

            if (lexer_peek_char(lexer, 1) == ':')
            {
                token->type = FRX_TOKEN_TYPE_RESOLUTION;
                lexer_advance(lexer);
            }

            break;
        }
        case ';': token->type = FRX_TOKEN_TYPE_SEMI; break;
        case '\0': token->type = FRX_TOKEN_TYPE_EOF; break;
        default: lexer_fail(lexer); break;
    }

    token->identifier = token_type_to_str(token->type);

    token->range.end = lexer->location;

    lexer_advance(lexer);
}

void lexer_next_token(Lexer* lexer)
{
    FRX_ASSERT(lexer != NULL);

    usize i;
    for(i = 1; i < lexer->tokens_count; i = (i + 1))
    {
        memcpy(&lexer->tokens[i - 1], &lexer->tokens[i], sizeof(Token));
    }

    if(lexer->tokens_count == 1)
    {
        lexer_read_token(lexer, &lexer->tokens[0]);
    }
    else
    {
        lexer->tokens_count = lexer->tokens_count - 1;
    }
}

const char* lexer_source_file(const Lexer* lexer)
{
    FRX_ASSERT(lexer != NULL);

    return lexer->filepath;
}

void lexer_destroy(Lexer* lexer)
{
    FRX_ASSERT(lexer != NULL);

    fclose(lexer->file);
    free(lexer->filepath);
    free(lexer->identifier_placeholder);
}
