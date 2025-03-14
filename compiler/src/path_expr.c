#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "token.h"

static PathSegment* path_segment_create(const char* name)
{
    FRX_ASSERT(name != NULL);

    PathSegment* path_segment = compiler_alloc_ast(sizeof(PathSegment));

    path_segment->name = name;

    return path_segment;
}

static PathExpr* path_expr_create(void)
{
    PathExpr* path_expr = compiler_alloc_ast(sizeof(ExprStmt));

    list_init(&path_expr->path_segments);

    return path_expr;
}

static void path_expr_add_segment(PathExpr* path_expr, PathSegment* path_segment)
{
    FRX_ASSERT(path_expr != NULL);

    list_add(&path_expr->path_segments, path_segment);
}

PathExpr* path_expr_parse(Parser* parser)
{
    const char* name = parser_current_token(parser)->identifier;
    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    PathExpr* path_expr = path_expr_create();
    PathSegment* path_segment = path_segment_create(name);
    path_expr_add_segment(path_expr, path_segment);

    while (parser_match(parser, FRX_TOKEN_TYPE_RESOLUTION))
    {
        parser_eat(parser, FRX_TOKEN_TYPE_RESOLUTION);
        name = parser_current_token(parser)->identifier;
        parser_eat(parser, FRX_TOKEN_TYPE_IDENT);
        path_segment = path_segment_create(name);
        path_expr_add_segment(path_expr, path_segment);
    }

    return path_expr;
}
