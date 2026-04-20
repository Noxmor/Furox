#include "assert.h"
#include "ast.h"
#include "attributes_table.h"
#include "parser.h"
#include "resolution.h"
#include "symbol.h"
#include "token.h"
#include "type_system.h"

static void path_segment_init(ASTPathSegment* path_segment, PathSegmentType type,
                              const char* name, TokenType primitive)
{
    FRX_ASSERT(path_segment != NULL);

    FRX_ASSERT(type < FRX_PATH_SEGMENT_TYPE_COUNT);

    path_segment->type = type;
    path_segment->name = name;
    path_segment->primitive = primitive;
    list_init(&path_segment->generic_args);
}

static AST* path_segment_parse(Parser* parser, PathStyle style)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(style < FRX_PATH_STYLE_COUNT);

    AST* ast = parser_create_ast(parser, FRX_AST_TYPE_PATH_SEGMENT);
    ASTPathSegment* path_segment = &ast->path_segment;

    ast->span.lo = parser_current_span(parser).lo;

    PathSegmentType type = FRX_PATH_SEGMENT_TYPE_COUNT;
    const char* name = NULL;
    TokenType primitive = FRX_TOKEN_TYPE_EOF;

    if (parser_current_type(parser) == FRX_TOKEN_TYPE_IDENT)
    {
        type = FRX_PATH_SEGMENT_TYPE_IDENT;
        ast->span.hi = parser_current_span(parser).hi;
        name = parse_ident(parser);
    }
    else if (token_type_is_primitive(parser_current_type(parser)))
    {
        type = FRX_PATH_SEGMENT_TYPE_PRIMITIVE;
        primitive = parser_current_type(parser);
        ast->span.hi = parser_current_span(parser).hi;
        parser_eat(parser, primitive);
    }
    else if (parser_current_type(parser) == FRX_TOKEN_TYPE_KW_SELF_UPPER)
    {
        type = FRX_PATH_SEGMENT_TYPE_SELF_UPPER;
        ast->span.hi = parser_current_span(parser).hi;
        parser_eat(parser, FRX_TOKEN_TYPE_KW_SELF_UPPER);
    }

    path_segment_init(path_segment, type, name, primitive);

    if ((style == FRX_PATH_STYLE_TYPE && parser_current_type(parser) == FRX_TOKEN_TYPE_LT)
        || (style == FRX_PATH_STYLE_EXPR && parser_current_type(parser) == FRX_TOKEN_TYPE_RESOLUTION
        && parser_peek(parser, 1)->type == FRX_TOKEN_TYPE_LT))
    {

        if (parser_current_type(parser) == FRX_TOKEN_TYPE_RESOLUTION)
        {
            parser_eat(parser, FRX_TOKEN_TYPE_RESOLUTION);
        }

        parser_eat(parser, FRX_TOKEN_TYPE_LT);

        while (!parser_match(parser, FRX_TOKEN_TYPE_GT))
        {
            if (!list_empty(&path_segment->generic_args))
            {
                parser_eat(parser, FRX_TOKEN_TYPE_COMMA);
            }

            list_add(&path_segment->generic_args, type_specifier_parse(parser));
        }

        ast->span.hi = parser_current_span(parser).hi;
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

static void path_init(ASTPath* path, ASTPathType path_type)
{
    FRX_ASSERT(path != NULL);

    FRX_ASSERT(path_type < FRX_PATH_TYPE_COUNT);

    path->type = path_type;
    list_init(&path->path_segments);
    path->mod = NULL;
    path->symbol = NULL;
}

AST* path_parse(Parser* parser, PathStyle style)
{
    FRX_ASSERT(parser != NULL);

    FRX_ASSERT(style < FRX_PATH_STYLE_COUNT);

    AST* ast = parser_create_ast(parser, FRX_AST_TYPE_PATH);
    ASTPath* path = &ast->path;

    ast->span.lo = parser_current_span(parser).lo;

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

    path_init(path, path_type);

    attributes_table_insert_scope(ast->id, parser->current_scope);
    path->mod = parser->src_file->module;

    list_add(&path->path_segments, path_segment_parse(parser, style));

    while (parser_match(parser, FRX_TOKEN_TYPE_RESOLUTION))
    {
        parser_eat(parser, FRX_TOKEN_TYPE_RESOLUTION);
        list_add(&path->path_segments, path_segment_parse(parser, style));
    }

    ast->span.hi = ((AST*)list_get(&path->path_segments, list_size(&path->path_segments) - 1))->span.hi;

    return ast;
}

void path_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_PATH);

    FRX_ASSERT(ctx != NULL);

    ASTPath* path = &ast->path;

    for (usize i = 0; i < list_size(&path->path_segments); ++i)
    {
        AST* path_segment = list_get(&path->path_segments, i);
        path_segment_resolve(path_segment, ctx);
    }

    AST* path_segment = list_get(&path->path_segments, 0);
    const Symbol* symbol = NULL;
    Module* mod = NULL;

    switch (path_segment->path_segment.type)
    {
        case FRX_PATH_SEGMENT_TYPE_PRIMITIVE: symbol = symbol_create(NULL, FRX_SYMBOL_VISIBILITY_PRIVATE, FRX_SYMBOL_TYPE_PRIMITIVE, path_segment); break;
        case FRX_PATH_SEGMENT_TYPE_IDENT:
        {
            symbol = scope_lookup_symbol(attributes_table_lookup_scope(ast->id), path_segment->path_segment.name);
            mod = module_find_submodule_by_name(path->mod, path_segment->path_segment.name);
            if (mod == NULL)
            {
                mod = module_find_submodule_by_name(ctx->root_mod, path_segment->path_segment.name);
            }

            break;
        }
        case FRX_PATH_SEGMENT_TYPE_SELF_UPPER: symbol = ctx->current_impl_block->impl_block.type_path->path.symbol; break;

        default: FRX_ASSERT(FRX_FALSE); break;
    }

    for (usize i = 1; i < list_size(&path->path_segments) && (symbol != NULL || mod != NULL); ++i)
    {
        path_segment = list_get(&path->path_segments, i);

        if (symbol != NULL)
        {
            switch (symbol->type)
            {
                case FRX_SYMBOL_TYPE_STRUCT: symbol = type_lookup_method(symbol_infer_type(symbol), path_segment->path_segment.name); break;
                case FRX_SYMBOL_TYPE_ENUM: symbol = symbol_create(path_segment->path_segment.name, FRX_SYMBOL_VISIBILITY_PRIVATE, FRX_SYMBOL_TYPE_ENUM_VARIANT, enum_def_lookup_variant(&symbol->data->enum_def, path_segment->path_segment.name)); break;
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

    path_segment = list_get(&path->path_segments, list_size(&path->path_segments) - 1);

    if (symbol != NULL)
    {
        path->symbol = symbol;
    }

    if (path->symbol == NULL)
    {
        resolution_context_fail(ctx);
    }
}
