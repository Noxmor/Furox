#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "sema.h"
#include "codegen.h"

static ContinueStmt* continue_stmt_create(b8 error)
{
    ContinueStmt* continue_stmt = compiler_alloc_ast(sizeof(ContinueStmt));

    continue_stmt->error = error;

    return continue_stmt;
}

ContinueStmt* continue_stmt_parse(Parser* parser)
{
    b8 error = FRX_FALSE;

    error |= parser_eat(parser, FRX_TOKEN_TYPE_KW_CONTINUE);
    error |= parser_eat(parser, FRX_TOKEN_TYPE_SEMI);

    return continue_stmt_create(error);
}

void continue_stmt_sema(ContinueStmt* continue_stmt)
{
    FRX_ASSERT(continue_stmt != NULL);
}

void continue_stmt_codegen(ContinueStmt* continue_stmt)
{
    FRX_ASSERT(continue_stmt != NULL);
    FRX_ASSERT(!continue_stmt->error);

    codegen_write("continue;\n");
}
