#include "assert.h"
#include "ast.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "codegen.h"

static void if_stmt_init(IfStmt* if_stmt, AST* condition, AST* if_block,
                         AST* else_block)
{
    if_stmt->condition = condition;
    if_stmt->if_block = if_block;
    if_stmt->else_block = else_block;
}

AST* if_stmt_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_IF_STMT);
    IfStmt* if_stmt = &ast->if_stmt;

    ast->range.start = parser_current_location(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_KW_IF);

    AST* condition = expr_parse(parser);
    AST* if_block = scope_parse(parser);
    AST* else_block = NULL;

    if (parser_match(parser, FRX_TOKEN_TYPE_KW_ELSE))
    {
        parser_eat(parser, FRX_TOKEN_TYPE_KW_ELSE);

        if (parser_match(parser, FRX_TOKEN_TYPE_KW_IF))
        {
            else_block = scope_from_stmt(stmt_parse(parser));
        }
        else
        {
            else_block = scope_parse(parser);
        }
    }

    if_stmt_init(if_stmt, condition, if_block, else_block);

    ast->range.end = parser_current_location(parser);

    return ast;
}

void if_stmt_resolve(AST* ast, Parser* parser)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_IF_STMT);

    IfStmt* if_stmt = &ast->if_stmt;

    if (if_stmt->condition != NULL)
    {
        ast_resolve(if_stmt->condition, parser);
    }

    if (if_stmt->if_block != NULL)
    {
        scope_resolve(if_stmt->if_block, parser);
    }

    if (if_stmt->else_block != NULL)
    {
        scope_resolve(if_stmt->else_block, parser);
    }
}

void if_stmt_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_IF_STMT);

    FRX_ASSERT(ctx != NULL);

    IfStmt* if_stmt = &ast->if_stmt;

    if (if_stmt->condition != NULL)
    {
        ast_sema(if_stmt->condition, ctx);
    }

    if (if_stmt->if_block != NULL)
    {
        scope_sema(if_stmt->if_block, ctx);
    }

    if (if_stmt->else_block != NULL)
    {
        scope_sema(if_stmt->else_block, ctx);
    }
}

void if_stmt_codegen(AST* ast, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_IF_STMT);

    FRX_ASSERT(ctx != NULL);

    IfStmt* if_stmt = &ast->if_stmt;

    // TODO: Implement
    (void)if_stmt;
}
