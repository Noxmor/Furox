#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "sema.h"
#include "codegen.h"

static ContinueStmt* continue_stmt_create(void)
{
    ContinueStmt* continue_stmt = compiler_alloc(sizeof(ContinueStmt));

    return continue_stmt;
}

ContinueStmt* continue_stmt_parse(Parser* parser)
{
    if (parser_eat(parser, FRX_TOKEN_TYPE_KW_CONTINUE))
    {
        return NULL;
    }

    if (parser_eat(parser, FRX_TOKEN_TYPE_SEMI))
    {
        return NULL;
    }

    return continue_stmt_create();
}

void continue_stmt_sema(ContinueStmt* continue_stmt)
{
    FRX_ASSERT(continue_stmt != NULL);
}

void continue_stmt_codegen(ContinueStmt* continue_stmt)
{
    FRX_ASSERT(continue_stmt != NULL);

    codegen_write("continue;\n");
}
