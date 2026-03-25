#include "assert.h"
#include "ast.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"

static void if_stmt_init(ASTIfStmt* if_stmt, AST* condition, AST* if_block,
                         AST* else_block)
{
    if_stmt->condition = condition;
    if_stmt->if_block = if_block;
    if_stmt->else_block = else_block;
}

AST* if_stmt_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_IF_STMT);
    ASTIfStmt* if_stmt = &ast->if_stmt;

    ast->range.start = parser_current_location(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_KW_IF);

    parser_eat(parser, FRX_TOKEN_TYPE_LPAREN);

    AST* condition = expr_parse(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_RPAREN);

    AST* if_block = block_parse(parser);
    AST* else_block = NULL;

    if (parser_match(parser, FRX_TOKEN_TYPE_KW_ELSE))
    {
        parser_eat(parser, FRX_TOKEN_TYPE_KW_ELSE);

        if (parser_match(parser, FRX_TOKEN_TYPE_KW_IF))
        {
            parser_push_scope(parser);
            else_block = block_from_stmt(stmt_parse(parser), parser->current_scope);
            parser_pop_scope(parser);
        }
        else
        {
            else_block = block_parse(parser);
        }
    }

    if_stmt_init(if_stmt, condition, if_block, else_block);

    ast->range.end = parser_current_location(parser);

    return ast;
}

void if_stmt_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_IF_STMT);

    ASTIfStmt* if_stmt = &ast->if_stmt;

    if (if_stmt->condition != NULL)
    {
        ast_resolve(if_stmt->condition, ctx);
    }

    if (if_stmt->if_block != NULL)
    {
        block_resolve(if_stmt->if_block, ctx);
    }

    if (if_stmt->else_block != NULL)
    {
        block_resolve(if_stmt->else_block, ctx);
    }
}

void if_stmt_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_IF_STMT);

    FRX_ASSERT(ctx != NULL);

    ASTIfStmt* if_stmt = &ast->if_stmt;

    if (if_stmt->condition != NULL)
    {
        ast_sema(if_stmt->condition, ctx);
    }

    if (if_stmt->if_block != NULL)
    {
        block_sema(if_stmt->if_block, ctx);
    }

    if (if_stmt->else_block != NULL)
    {
        block_sema(if_stmt->else_block, ctx);
    }
}
