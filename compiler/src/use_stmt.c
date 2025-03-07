#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "diagnostics.h"
#include "module.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "codegen.h"
#include "string_table.h"
#include "symbol_table.h"

#include <string.h>

static UseStmt* use_stmt_create(b8 error, const char* name)
{
    UseStmt* use_stmt = compiler_alloc(sizeof(UseStmt));

    use_stmt->error = error;
    list_init(&use_stmt->path_segments);
    use_stmt->symbol_name = name;
    use_stmt->symbol = NULL;

    return use_stmt;
}

UseStmt* use_stmt_parse(Parser* parser)
{
    b8 error = FRX_FALSE;

    error |= parser_eat(parser, FRX_TOKEN_TYPE_KW_USE);

    const char* symbol_name = parser_current_token(parser)->identifier;

    error |= parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    UseStmt* use_stmt = use_stmt_create(error, symbol_name);

    while (parser_match(parser, FRX_TOKEN_TYPE_RESOLUTION))
    {
        error |= parser_eat(parser, FRX_TOKEN_TYPE_RESOLUTION);

        list_add(&use_stmt->path_segments, (void*)use_stmt->symbol_name);
        use_stmt->symbol_name = parser_current_token(parser)->identifier;

        error |= parser_eat(parser, FRX_TOKEN_TYPE_IDENT);
    }

    use_stmt->error |= parser_eat(parser, FRX_TOKEN_TYPE_SEMI);

    list_add(&parser->use_stmts, use_stmt);

    return use_stmt;
}

void use_stmt_resolve(Parser* parser, UseStmt* use_stmt)
{
    FRX_ASSERT(use_stmt != NULL);

    if (use_stmt->error)
    {
        return;
    }

    SymbolID id = symbol_intern(use_stmt->symbol_name);

    Module* mod = parser_find_module_by_path_segments(parser, &use_stmt->path_segments);
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
        use_stmt->symbol = symbol_table_lookup(&mod->symbol_table, parser, id);
    }
}
