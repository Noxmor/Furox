#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "diagnostics.h"
#include "module.h"
#include "parser.h"
#include "resolution.h"
#include "string_table.h"
#include "symbol_table.h"

#include <string.h>

static void use_stmt_init(UseStmt* use_stmt, const char* name)
{
    FRX_ASSERT(use_stmt != NULL);

    list_init(&use_stmt->path_segments);
    use_stmt->symbol_name = name;
    use_stmt->module = NULL;
}

AST* use_stmt_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_USE_STMT);
    UseStmt* use_stmt = &ast->use_stmt;

    ast->range.start = parser_current_location(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_KW_USE);

    const char* symbol_name = parser_current_token(parser)->identifier;

    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    use_stmt_init(use_stmt, symbol_name);

    while (parser_match(parser, FRX_TOKEN_TYPE_RESOLUTION))
    {
        parser_eat(parser, FRX_TOKEN_TYPE_RESOLUTION);

        list_add(&use_stmt->path_segments, (void*)use_stmt->symbol_name);
        use_stmt->symbol_name = parser_current_token(parser)->identifier;

        parser_eat(parser, FRX_TOKEN_TYPE_IDENT);
    }

    parser_eat(parser, FRX_TOKEN_TYPE_SEMI);

    list_add(&parser->use_stmts, use_stmt);

    ast->range.end = parser_current_location(parser);

    return ast;
}

void use_stmt_resolve(AST* ast, Parser* parser)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_USE_STMT);

    UseStmt* use_stmt = &ast->use_stmt;

    Module* mod = parser_find_module_by_path_segments(parser, &use_stmt->path_segments);
    if (mod == NULL)
    {
        mod = compiler_find_module_by_path_segments(&use_stmt->path_segments);
    }

    if (mod == NULL)
    {
        usize path_segments_count = list_size(&use_stmt->path_segments);
        usize path_len = path_segments_count == 0 ? 0 : path_segments_count * 2;
        for (usize i = 0; i < path_segments_count; ++i)
        {
            const char* path_segment = list_get(&use_stmt->path_segments, i);
            path_len += strlen(path_segment);
        }

        char path_buffer[path_len + 1];
        path_buffer[0] = '\0';
        for (usize i = 0; i < path_segments_count; ++i)
        {
            const char* path_segment = list_get(&use_stmt->path_segments, i);
            strcat(path_buffer, path_segment);

            if (i + 1 < path_segments_count)
            {
                strcat(path_buffer, token_type_to_str(FRX_TOKEN_TYPE_RESOLUTION));
            }
        }

        const char* path = string_table_intern(path_buffer);

        SourceRange range; //TODO: Replace with correct range
        range.start.line = 0;
        range.start.column = 0;
        FRX_PARSER_ADD_DIAGNOSTIC(parser, FRX_DIAGNOSTIC_ID_INVALID_MODULE_PATH, FRX_DIAGNOSTIC_LVL_ERROR, range, path);
    }
    else
    {
        use_stmt->module = mod;
    }
}
