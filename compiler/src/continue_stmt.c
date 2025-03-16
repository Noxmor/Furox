#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "sema.h"

static ContinueStmt* continue_stmt_create(void)
{
    ContinueStmt* continue_stmt = compiler_alloc_ast(sizeof(ContinueStmt));

    return continue_stmt;
}

ContinueStmt* continue_stmt_parse(Parser* parser)
{
    parser_eat(parser, FRX_TOKEN_TYPE_KW_CONTINUE);
    parser_eat(parser, FRX_TOKEN_TYPE_SEMI);

    return continue_stmt_create();
}

void continue_stmt_sema(ContinueStmt* continue_stmt)
{
    FRX_ASSERT(continue_stmt != NULL);
}
