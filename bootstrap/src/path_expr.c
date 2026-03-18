#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "module.h"
#include "parser.h"
#include "resolution.h"
#include "scope.h"
#include "symbol.h"
#include "token.h"
#include "type_system.h"

static void path_segment_init(ASTPathSegment* path_segment, const char* name)
{
    FRX_ASSERT(path_segment != NULL);

    FRX_ASSERT(name != NULL);

    path_segment->name = name;
    list_init(&path_segment->generic_args);
}

static AST* path_segment_parse(Parser* parser, PathStyle style)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(style < FRX_PATH_STYLE_COUNT);

    AST* ast = ast_create(FRX_AST_TYPE_PATH_SEGMENT);
    ASTPathSegment* path_segment = &ast->path_segment;

    const char* name = parser_current_token(parser)->identifier;
    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    path_segment_init(path_segment, name);

    if (parser_current_type(parser) == FRX_TOKEN_TYPE_RESOLUTION
        && parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_LT)
    {
        parser_eat(parser, FRX_TOKEN_TYPE_RESOLUTION);
    }

    if (style == FRX_PATH_STYLE_TYPE && parser_current_type(parser) == FRX_TOKEN_TYPE_LT)
    {
        parser_eat(parser, FRX_TOKEN_TYPE_LT);

        while (!parser_match(parser, FRX_TOKEN_TYPE_GT))
        {
            if (!list_empty(&path_segment->generic_args))
            {
                parser_eat(parser, FRX_TOKEN_TYPE_COMMA);
            }

            list_add(&path_segment->generic_args, type_specifier_parse(parser));
        }

        parser_eat(parser, FRX_TOKEN_TYPE_GT);
    }

    return ast;
}

static void path_segment_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_PATH_SEGMENT);

    FRX_ASSERT(ctx != NULL);

    ASTPathSegment* path_segment = &ast->path_segment;

    for (usize i = 0; i < list_size(&path_segment->generic_args); ++i)
    {
        AST* type_specifier = list_get(&path_segment->generic_args, i);
        type_specifier_resolve(type_specifier, ctx);
    }
}

static void path_expr_init(ASTPathExpr* path_expr, ASTPathType path_type)
{
    FRX_ASSERT(path_expr != NULL);

    FRX_ASSERT(path_type < FRX_PATH_TYPE_COUNT);

    path_expr->type = path_type;
    list_init(&path_expr->path_segments);
    path_expr->scope = NULL;
    path_expr->mod = NULL;
    path_expr->symbol = NULL;
}

AST* path_expr_parse(Parser* parser, PathStyle style)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(style < FRX_PATH_STYLE_COUNT);

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

    path_expr->scope = parser->current_scope;
    path_expr->mod = parser->src_file->module;

    list_add(&path_expr->path_segments, path_segment_parse(parser, style));

    while (parser_match(parser, FRX_TOKEN_TYPE_RESOLUTION))
    {
        parser_eat(parser, FRX_TOKEN_TYPE_RESOLUTION);
        list_add(&path_expr->path_segments, path_segment_parse(parser, style));
    }

    ast->range.end = parser_current_location(parser);

    compiler_register_expr(ast);

    return ast;
}

void path_expr_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_PATH_EXPR);

    FRX_ASSERT(ctx != NULL);

    ASTPathExpr* path_expr = &ast->path_expr;

    for (usize i = 0; i < list_size(&path_expr->path_segments); ++i)
    {
        AST* path_segment = list_get(&path_expr->path_segments, i);
        path_segment_resolve(path_segment, ctx);
    }

    AST* path_segment = list_get(&path_expr->path_segments, 0);
    Symbol* symbol = scope_lookup_symbol(path_expr->scope, path_segment->path_segment.name);
    Module* mod = module_find_submodule_by_name(path_expr->mod, path_segment->path_segment.name);
    if (mod == NULL)
    {
        mod = module_find_submodule_by_name(ctx->root_mod, path_segment->path_segment.name);
    }

    for (usize i = 1; i < list_size(&path_expr->path_segments) && (symbol != NULL || mod != NULL); ++i)
    {
        path_segment = list_get(&path_expr->path_segments, i);

        if (symbol != NULL)
        {
            switch (symbol->type)
            {
                case FRX_SYMBOL_TYPE_STRUCT: symbol = type_lookup_method(symbol_infer_type(symbol), path_segment->path_segment.name); break;
                case FRX_SYMBOL_TYPE_ENUM: symbol = symbol_create(path_segment->path_segment.name, FRX_SYMBOL_VISIBILITY_PRIVATE, FRX_SYMBOL_TYPE_ENUM_CONSTANT, enum_def_lookup_constant(symbol->data, path_segment->path_segment.name)); break;
                default: break;
            }
        }
        else if (mod != NULL)
        {
            symbol = module_lookup_symbol(mod, path_segment->path_segment.name);
            if (symbol == NULL)
            {
                mod = module_find_submodule_by_name(mod, path_segment->path_segment.name);
            }
        }
    }

    path_segment = list_get(&path_expr->path_segments, list_size(&path_expr->path_segments) - 1);

    if (symbol != NULL)
    {
        path_expr->symbol = symbol;
    }
    else
    {
        path_expr->symbol = symbol_create(path_segment->path_segment.name, FRX_SYMBOL_VISIBILITY_PRIVATE, FRX_SYMBOL_TYPE_MODULE, mod);
    }

    if (path_expr->symbol == NULL)
    {
        resolution_context_fail(ctx);
    }
}
