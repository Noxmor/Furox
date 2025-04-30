#include "assert.h"
#include "ast.h"
#include "codegen.h"
#include "parser.h"
#include "token.h"

static void path_segment_init(PathSegment* path_segment, const char* name,
                              b8 external)
{
    FRX_ASSERT(name != NULL || external == FRX_TRUE);

    path_segment->name = name;
    path_segment->external = external;
}

static AST* path_segment_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_PATH_SEGMENT);
    PathSegment* path_segment = &ast->path_segment;

    ast->range.start = parser_current_location(parser);

    b8 external = FRX_FALSE;
    const char* name = NULL;

    if (parser_match(parser, FRX_TOKEN_TYPE_KW_EXTERN))
    {
        external = FRX_TRUE;
        parser_eat(parser, FRX_TOKEN_TYPE_KW_EXTERN);
    }
    else
    {
        name = parser_current_token(parser)->identifier;
        parser_eat(parser, FRX_TOKEN_TYPE_IDENT);
    }

    path_segment_init(path_segment, name, external);

    ast->range.end = parser_current_location(parser);

    return ast;
}

static void path_expr_init(PathExpr* path_expr)
{
    FRX_ASSERT(path_expr != NULL);

    list_init(&path_expr->path_segments);
}

static void path_expr_add_segment(PathExpr* path_expr, AST* path_segment)
{
    FRX_ASSERT(path_expr != NULL);

    FRX_ASSERT(path_segment->type == FRX_AST_TYPE_PATH_SEGMENT);

    list_add(&path_expr->path_segments, path_segment);
}

AST* path_expr_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_PATH_EXPR);
    PathExpr* path_expr = &ast->path_expr;

    ast->range.start = parser_current_location(parser);

    path_expr_init(path_expr);

    AST* path_segment = path_segment_parse(parser);
    path_expr_add_segment(path_expr, path_segment);

    while (parser_match(parser, FRX_TOKEN_TYPE_RESOLUTION))
    {
        parser_eat(parser, FRX_TOKEN_TYPE_RESOLUTION);
        path_segment = path_segment_parse(parser);
        path_expr_add_segment(path_expr, path_segment);
    }

    ast->range.end = parser_current_location(parser);

    return ast;
}

void path_expr_resolve(AST* ast, Parser* parser)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_PATH_EXPR);

    // TODO: Implement
    (void)parser;
}

void path_expr_codegen(AST* ast, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_PATH_EXPR);

    FRX_ASSERT(ctx != NULL);

    PathExpr* path_expr = &ast->path_expr;

    for (usize i = 0; i < list_size(&path_expr->path_segments); ++i)
    {
        if (i > 0)
        {
            fprintf(ctx->source, "_");
        }

        AST* segment = list_get(&path_expr->path_segments, i);
        PathSegment* path_segment = &segment->path_segment;

        if (!path_segment->external)
        {
            fprintf(ctx->source, "%s", path_segment->name);
        }
    }
}
