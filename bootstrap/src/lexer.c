#include "lexer.h"

#include <string.h>
#include <ctype.h>

#include "core/assert.h"
#include "core/log.h"

#include "symbols/hash.h"

typedef struct LexerInfo
{
    usize lines_processed;
} LexerInfo;

static LexerInfo lexer_info;

typedef struct KeywordTableEntry
{
    char name[FRX_TOKEN_IDENTIFIER_CAPACITY];

    TokenType type;
} KeywordTableEntry;

#define FRX_KEYWORD_TABLE_SIZE 512

typedef struct KeywordTable
{
    KeywordTableEntry entries[FRX_KEYWORD_TABLE_SIZE];
} KeywordTable;

static KeywordTable keyword_table;

static void register_keyword(const char* name, TokenType type)
{
    FRX_ASSERT(name != NULL);

    u64 index = hash_djb2(name) % FRX_KEYWORD_TABLE_SIZE;

    KeywordTableEntry* entry = &keyword_table.entries[index];

    FRX_ASSERT(strlen(entry->name) == 0);

    strcpy(entry->name, name);
    entry->type = type;
}

static KeywordTableEntry* keyword_table_find(const char* name)
{
    FRX_ASSERT(name != NULL);

    u64 index = hash_djb2(name) % FRX_KEYWORD_TABLE_SIZE;

    return &keyword_table.entries[index];
}

void lexer_init_keyword_table(void)
{
    memset(&keyword_table, 0, sizeof(KeywordTable));

    register_keyword("u8", FRX_TOKEN_TYPE_KW_U8);
    register_keyword("u16", FRX_TOKEN_TYPE_KW_U16);
    register_keyword("u32", FRX_TOKEN_TYPE_KW_U32);
    register_keyword("u64", FRX_TOKEN_TYPE_KW_U64);
    register_keyword("usize", FRX_TOKEN_TYPE_KW_USIZE);
    register_keyword("i8", FRX_TOKEN_TYPE_KW_I8);
    register_keyword("i16", FRX_TOKEN_TYPE_KW_I16);
    register_keyword("i32", FRX_TOKEN_TYPE_KW_I32);
    register_keyword("i64", FRX_TOKEN_TYPE_KW_I64);
    register_keyword("isize", FRX_TOKEN_TYPE_KW_ISIZE);
    register_keyword("b8", FRX_TOKEN_TYPE_KW_B8);
    register_keyword("b16", FRX_TOKEN_TYPE_KW_B16);
    register_keyword("b32", FRX_TOKEN_TYPE_KW_B32);
    register_keyword("b64", FRX_TOKEN_TYPE_KW_B64);
    register_keyword("char", FRX_TOKEN_TYPE_KW_CHAR);
    register_keyword("f32", FRX_TOKEN_TYPE_KW_F32);
    register_keyword("f64", FRX_TOKEN_TYPE_KW_F64);
    register_keyword("void", FRX_TOKEN_TYPE_KW_VOID);

    register_keyword("nullptr", FRX_TOKEN_TYPE_KW_NULLPTR);
    register_keyword("true", FRX_TOKEN_TYPE_KW_TRUE);
    register_keyword("false", FRX_TOKEN_TYPE_KW_FALSE);
    register_keyword("import", FRX_TOKEN_TYPE_KW_IMPORT);
    register_keyword("export", FRX_TOKEN_TYPE_KW_EXPORT);
    register_keyword("namespace", FRX_TOKEN_TYPE_KW_NAMESPACE);
    register_keyword("enum", FRX_TOKEN_TYPE_KW_ENUM);
    register_keyword("struct", FRX_TOKEN_TYPE_KW_STRUCT);
    register_keyword("extern", FRX_TOKEN_TYPE_KW_EXTERN);
    register_keyword("return", FRX_TOKEN_TYPE_KW_RETURN);
    register_keyword("if", FRX_TOKEN_TYPE_KW_IF);
    register_keyword("else", FRX_TOKEN_TYPE_KW_ELSE);
    register_keyword("switch", FRX_TOKEN_TYPE_KW_SWITCH);
    register_keyword("case", FRX_TOKEN_TYPE_KW_CASE);
    register_keyword("default", FRX_TOKEN_TYPE_KW_DEFAULT);
    register_keyword("break", FRX_TOKEN_TYPE_KW_BREAK);
    register_keyword("for", FRX_TOKEN_TYPE_KW_FOR);
    register_keyword("while", FRX_TOKEN_TYPE_KW_WHILE);
    register_keyword("do", FRX_TOKEN_TYPE_KW_DO);
    register_keyword("macro", FRX_TOKEN_TYPE_KW_MACRO);
    register_keyword("sizeof", FRX_TOKEN_TYPE_KW_SIZEOF);
}

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

    lexer->location.pos = 0;
    lexer->location.line = 1;
    lexer->location.coloumn = 0;

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
        ++lexer->location.line;
        lexer->location.coloumn = 0;

        ++lexer_info.lines_processed;
    }
    else
        ++lexer->location.coloumn;

    ++lexer->buffer_index;

    ++lexer->location.pos;

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

    usize line_start = lexer->location.line;
    usize coloumn_start = lexer->location.coloumn;

    usize comment_blocks_to_skip = 1;

    while(comment_blocks_to_skip > 0)
    {
        lexer_advance(lexer);

        char current = lexer_peek_char(lexer, 0);
        char next = lexer_peek_char(lexer, 1);

        if(current == '/' && next == '*')
        {
            line_start = lexer->location.line;
            coloumn_start = lexer->location.coloumn;

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

    KeywordTableEntry* entry = keyword_table_find(token->identifier);

    if(strcmp(entry->name, token->identifier) == 0)
        token->type = entry->type;
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
                FRX_ERROR_FILE("'\\%c' is not a valid escaped character!", lexer->filepath, token->location.line, token->location.coloumn, token->identifier[0]);

                return FRX_TRUE;
            }
        }

        lexer_advance(lexer);
    }

    if(lexer_peek_char(lexer, 0) != '\'')
    {
        FRX_ERROR_FILE("Missing ' after char literal!", lexer->filepath, token->location.line, token->location.coloumn);

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
            FRX_ERROR_FILE("Reached end of file while parsing string literal!", lexer->filepath, token->location.line, token->location.coloumn);

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

    memcpy(&token->location, &lexer->location, sizeof(SourceLocation));

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

        case '-':
        {
            if(lexer_peek_char(lexer, 1) == '>')
            {
                token->type = FRX_TOKEN_TYPE_ARROW;
                strcpy(token->identifier, "->");

                lexer_advance(lexer);
                lexer_advance(lexer);

                return FRX_FALSE;
            }

            token->type = FRX_TOKEN_TYPE_MINUS;

            break;
        }

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
        case '.':
        {
            if(lexer_peek_char(lexer, 1) == '.' && lexer_peek_char(lexer, 2) == '.')
            {
                token->type = FRX_TOKEN_TYPE_ELLIPSIS;
                strcpy(token->identifier, "...");

                lexer_advance(lexer);
                lexer_advance(lexer);
                lexer_advance(lexer);

                return FRX_FALSE;
            }

            token->type = FRX_TOKEN_TYPE_DOT;

            break;
        }

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
            FRX_ERROR_FILE("Could not process '%c'!", lexer->filepath, lexer->location.line, lexer->location.coloumn, current);

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

FRX_NO_DISCARD b8 lexer_recover(Lexer* lexer, SourceLocation* location)
{
    FRX_ASSERT(lexer != NULL);

    FRX_ASSERT(location != NULL);

    FRX_ASSERT(location->pos <= lexer->location.pos);

    if(fseek(lexer->file, location->pos, SEEK_SET) != 0)
        return FRX_TRUE;

    memcpy(&lexer->location, location, sizeof(SourceLocation));

    lexer->buffer_index = FRX_LEXER_BUFFER_CAPACITY;

    if(lexer_read_token(lexer, &lexer->tokens[0]))
        return FRX_TRUE;

    lexer->tokens_count = 1;

    return FRX_FALSE;
}

void lexer_destroy(Lexer* lexer)
{
    FRX_ASSERT(lexer != NULL);

    fclose(lexer->file);
}
