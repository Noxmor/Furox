#include "assert.h"
#include "ast.h"
#include "parser.h"
#include "resolution.h"
#include "symbol_table.h"
#include "token.h"

static void path_expr_init(ASTPathExpr* path_expr, ASTPathType path_type)
{
    FRX_ASSERT(path_expr != NULL);

    FRX_ASSERT(path_type < FRX_PATH_TYPE_COUNT);

    path_expr->type = path_type;
    list_init(&path_expr->path_segments);
}

AST* path_expr_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_PATH_EXPR);
    ASTPathExpr* path_expr = &ast->path_expr;

    ast->range.start = parser_current_location(parser);

    ASTPathType path_type = FRX_PATH_TYPE_ABSOLUTE;
    if (parser_current_type(parser) == FRX_TOKEN_TYPE_KW_EXTERN)
    {
        path_type = FRX_PATH_TYPE_EXTERN;
        parser_eat(parser, FRX_TOKEN_TYPE_KW_EXTERN);
        parser_eat(parser, FRX_TOKEN_TYPE_RESOLUTION);
    }
    else if (parser_current_type(parser) == FRX_TOKEN_TYPE_KW_MOD)
    {
        path_type = FRX_PATH_TYPE_MOD;
        parser_eat(parser, FRX_TOKEN_TYPE_KW_MOD);
        parser_eat(parser, FRX_TOKEN_TYPE_RESOLUTION);
    }

    path_expr_init(path_expr, path_type);

    list_add(&path_expr->path_segments, (char*)parser_current_token(parser)->identifier);
    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    while (parser_match(parser, FRX_TOKEN_TYPE_RESOLUTION))
    {
        parser_eat(parser, FRX_TOKEN_TYPE_RESOLUTION);
        list_add(&path_expr->path_segments, (char*)parser_current_token(parser)->identifier);
        parser_eat(parser, FRX_TOKEN_TYPE_IDENT);
    }

    ast->range.end = parser_current_location(parser);

    return ast;
}

void path_expr_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_PATH_EXPR);

    ASTPathExpr* path_expr = &ast->path_expr;
    const char* name = list_get(&path_expr->path_segments, list_size(&path_expr->path_segments) - 1);

    path_expr->symbol = symbol_table_lookup(&ctx->src_file->symbol_table, FRX_SYMBOL_TYPE_FUNC, name);
    if (path_expr->symbol == NULL)
    {
        path_expr->symbol = symbol_table_lookup(&ctx->src_file->symbol_table, FRX_SYMBOL_TYPE_EXTERN_FUNC, name);
    }

    if (path_expr->symbol == NULL)
    {
        path_expr->symbol = symbol_table_lookup(&ctx->src_file->symbol_table, FRX_SYMBOL_TYPE_PARAM, name);
    }

    if (path_expr->symbol == NULL)
    {
        path_expr->symbol = symbol_table_lookup(&ctx->src_file->symbol_table, FRX_SYMBOL_TYPE_VAR, name);
    }

    if (path_expr->symbol == NULL)
    {
        path_expr->symbol = symbol_table_lookup_type(&ctx->src_file->symbol_table, name);
    }

    if (path_expr->symbol == NULL)
    {
        resolution_context_fail(ctx);
    }
}
