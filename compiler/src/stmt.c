#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "diagnostics.h"
#include "parser.h"
#include "sema.h"
#include "codegen.h"

typedef void (*StmtSemaFunc)(void*);

static const StmtSemaFunc stmt_type_to_sema[FRX_STMT_TYPE_COUNT] = {
    [FRX_STMT_TYPE_EXPR_STMT] = (StmtSemaFunc)expr_stmt_sema,
    [FRX_STMT_TYPE_BREAK_STMT] = (StmtSemaFunc)break_stmt_sema,
    [FRX_STMT_TYPE_CONTINUE_STMT] = (StmtSemaFunc)continue_stmt_sema,
    [FRX_STMT_TYPE_RETURN_STMT] = (StmtSemaFunc)return_stmt_sema
};

typedef void (*StmtCodegenFunc)(void*);

static const StmtCodegenFunc stmt_type_to_codegen[FRX_STMT_TYPE_COUNT] = {
    [FRX_STMT_TYPE_EXPR_STMT] = (StmtCodegenFunc)expr_stmt_codegen,
    [FRX_STMT_TYPE_BREAK_STMT] = (StmtCodegenFunc)break_stmt_codegen,
    [FRX_STMT_TYPE_CONTINUE_STMT] = (StmtCodegenFunc)continue_stmt_codegen,
    [FRX_STMT_TYPE_RETURN_STMT] = (StmtCodegenFunc)return_stmt_codegen
};
static Stmt* stmt_create(StmtType type, void* node)
{
    FRX_ASSERT(type < FRX_STMT_TYPE_COUNT);

    Stmt* stmt = compiler_alloc(sizeof(Stmt));

    stmt->type = type;
    stmt->node = node;

    return stmt;
}

Stmt* stmt_parse(Parser* parser)
{
    switch (parser_current_type(parser))
    {
        case FRX_TOKEN_TYPE_INT_LIT: return stmt_create(FRX_STMT_TYPE_EXPR_STMT, expr_stmt_parse(parser));
        case FRX_TOKEN_TYPE_KW_BREAK: return stmt_create(FRX_STMT_TYPE_BREAK_STMT, break_stmt_parse(parser));
        case FRX_TOKEN_TYPE_KW_CONTINUE: return stmt_create(FRX_STMT_TYPE_CONTINUE_STMT, continue_stmt_parse(parser));
        case FRX_TOKEN_TYPE_KW_RETURN: return stmt_create(FRX_STMT_TYPE_RETURN_STMT, return_stmt_parse(parser));
        default:
        {
            FRX_PARSER_ADD_DIAGNOSTIC(parser, FRX_DIAGNOSTIC_ID_EXPECTED_STMT,
                                      FRX_DIAGNOSTIC_LVL_ERROR,
                                      parser_current_token(parser)->range,
                                      token_type_to_str(parser_current_type(parser)));

            parser_recover(parser);

            return NULL;
        }
    }
}

void stmt_sema(Stmt* stmt)
{
    FRX_ASSERT(stmt != NULL);

    stmt_type_to_sema[stmt->type](stmt->node);
}

void stmt_codegen(Stmt* stmt)
{
    FRX_ASSERT(stmt != NULL);

    stmt_type_to_codegen[stmt->type](stmt->node);
}
