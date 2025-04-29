#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "diagnostics.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "codegen.h"

typedef void (*StmtResolveFunc)(Parser*, void*);

static const StmtResolveFunc stmt_type_to_resolve[FRX_STMT_TYPE_COUNT] = {
    [FRX_STMT_TYPE_ERROR] = (StmtResolveFunc)NULL,
    [FRX_STMT_TYPE_EXPR_STMT] = (StmtResolveFunc)expr_stmt_resolve,
    [FRX_STMT_TYPE_BREAK_STMT] = (StmtResolveFunc)NULL,
    [FRX_STMT_TYPE_CONTINUE_STMT] = (StmtResolveFunc)NULL,
    [FRX_STMT_TYPE_RETURN_STMT] = (StmtResolveFunc)return_stmt_resolve,
    [FRX_STMT_TYPE_LET_STMT] = (StmtResolveFunc)let_stmt_resolve,
    [FRX_STMT_TYPE_IF_STMT] = (StmtResolveFunc)if_stmt_resolve
};

typedef void (*StmtSemaFunc)(void*);

static const StmtSemaFunc stmt_type_to_sema[FRX_STMT_TYPE_COUNT] = {
    [FRX_STMT_TYPE_ERROR] = (StmtSemaFunc)NULL,
    [FRX_STMT_TYPE_EXPR_STMT] = (StmtSemaFunc)expr_stmt_sema,
    [FRX_STMT_TYPE_BREAK_STMT] = (StmtSemaFunc)break_stmt_sema,
    [FRX_STMT_TYPE_CONTINUE_STMT] = (StmtSemaFunc)continue_stmt_sema,
    [FRX_STMT_TYPE_RETURN_STMT] = (StmtSemaFunc)return_stmt_sema,
    [FRX_STMT_TYPE_LET_STMT] = (StmtSemaFunc)let_stmt_sema,
    [FRX_STMT_TYPE_IF_STMT] = (StmtSemaFunc)if_stmt_sema
};

typedef void (*StmtCodegenFunc)(void*, CodegenContext* ctx);

static const StmtCodegenFunc stmt_type_to_codegen[FRX_STMT_TYPE_COUNT] = {
    [FRX_STMT_TYPE_ERROR] = (StmtCodegenFunc)NULL,
    [FRX_STMT_TYPE_EXPR_STMT] = (StmtCodegenFunc)NULL,
    [FRX_STMT_TYPE_BREAK_STMT] = (StmtCodegenFunc)NULL,
    [FRX_STMT_TYPE_CONTINUE_STMT] = (StmtCodegenFunc)NULL,
    [FRX_STMT_TYPE_RETURN_STMT] = (StmtCodegenFunc)return_stmt_codegen,
    [FRX_STMT_TYPE_LET_STMT] = (StmtCodegenFunc)let_stmt_codegen,
    [FRX_STMT_TYPE_IF_STMT] = (StmtCodegenFunc)NULL,
};

static Stmt* stmt_create(StmtType type, void* node)
{
    FRX_ASSERT(type < FRX_STMT_TYPE_COUNT);

    FRX_ASSERT((type == FRX_STMT_TYPE_ERROR && node == NULL)
               || (type != FRX_STMT_TYPE_ERROR && node != NULL));

    Stmt* stmt = compiler_alloc_ast(sizeof(Stmt));

    stmt->type = type;
    stmt->node = node;

    return stmt;
}

Stmt* stmt_parse(Parser* parser)
{
    switch (parser_current_type(parser))
    {
        case FRX_TOKEN_TYPE_IDENT:
        case FRX_TOKEN_TYPE_LPAREN:
        case FRX_TOKEN_TYPE_KW_EXTERN:
        case FRX_TOKEN_TYPE_INT_LIT: return stmt_create(FRX_STMT_TYPE_EXPR_STMT, expr_stmt_parse(parser));
        case FRX_TOKEN_TYPE_KW_BREAK: return stmt_create(FRX_STMT_TYPE_BREAK_STMT, break_stmt_parse(parser));
        case FRX_TOKEN_TYPE_KW_CONTINUE: return stmt_create(FRX_STMT_TYPE_CONTINUE_STMT, continue_stmt_parse(parser));
        case FRX_TOKEN_TYPE_KW_RETURN: return stmt_create(FRX_STMT_TYPE_RETURN_STMT, return_stmt_parse(parser));
        case FRX_TOKEN_TYPE_KW_LET: return stmt_create(FRX_STMT_TYPE_LET_STMT, let_stmt_parse(parser));
        case FRX_TOKEN_TYPE_KW_IF: return stmt_create(FRX_STMT_TYPE_IF_STMT, if_stmt_parse(parser));
        default:
        {
            FRX_PARSER_ADD_DIAGNOSTIC(parser, FRX_DIAGNOSTIC_ID_EXPECTED_STMT,
                                      FRX_DIAGNOSTIC_LVL_ERROR,
                                      parser_current_token(parser)->range,
                                      token_type_to_str(parser_current_type(parser)));

            parser_recover(parser);

            return stmt_create(FRX_STMT_TYPE_ERROR, NULL);
        }
    }
}

void stmt_resolve(Parser* parser, Stmt* stmt)
{
    FRX_ASSERT(stmt != NULL);

    StmtResolveFunc func = stmt_type_to_resolve[stmt->type];
    if (func != NULL)
    {
        func(parser, stmt->node);
    }
}

void stmt_sema(Stmt* stmt)
{
    FRX_ASSERT(stmt != NULL);

    StmtSemaFunc func = stmt_type_to_sema[stmt->type];
    if (func != NULL)
    {
        func(stmt->node);
    }
}

void stmt_codegen(Stmt* stmt, CodegenContext* ctx)
{
    FRX_ASSERT(stmt != NULL);

    FRX_ASSERT(ctx != NULL);

    StmtCodegenFunc func = stmt_type_to_codegen[stmt->type];
    if (func != NULL)
    {
        func(stmt->node, ctx);
    }
}
