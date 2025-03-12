#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "sema.h"
#include "codegen.h"
#include "source_location.h"
#include "source_range.h"

static BreakStmt* break_stmt_create(b8 error, SourceRange range)
{
    BreakStmt* break_stmt = compiler_alloc_ast(sizeof(BreakStmt));

    break_stmt->error = error;
    break_stmt->range = range;

    return break_stmt;
}

BreakStmt* break_stmt_parse(Parser* parser)
{
    b8 error = FRX_FALSE;
    SourceLocation start = parser_current_location(parser);

    error |= parser_eat(parser, FRX_TOKEN_TYPE_KW_BREAK);

    SourceLocation end = parser_current_location(parser);

    error |= parser_eat(parser, FRX_TOKEN_TYPE_SEMI);

    SourceRange range = {
        .start = start,
        .end = end
    };

    return break_stmt_create(error, range);
}

void break_stmt_sema(BreakStmt* break_stmt)
{
    FRX_ASSERT(break_stmt != NULL);
}

void break_stmt_codegen(BreakStmt* break_stmt)
{
    FRX_ASSERT(break_stmt != NULL);
    FRX_ASSERT(!break_stmt->error);

    codegen_write("break;\n");
}
