#include "assert.h"
#include "ast.h"
#include "diagnostics.h"
#include "parser.h"
#include "resolution.h"
#include "token.h"
#include "type_system.h"

static void type_specifier_init(ASTTypeSpecifier* type, ASTTypeSpecifierKind kind)
{
    FRX_ASSERT(type != NULL);

    FRX_ASSERT(kind < FRX_TYPE_SPECIFIER_KIND_COUNT);

    type->kind = kind;
    type->resolved_type = NULL;
    type->path = NULL;
    list_init(&type->func_params);
    type->func_return_type = NULL;
    type->is_variadic = FRX_FALSE;
}

static void type_specifier_init_path(ASTTypeSpecifier* type, AST* path)
{
    FRX_ASSERT(type != NULL);

    FRX_ASSERT(path != NULL);

    type_specifier_init(type, FRX_TYPE_SPECIFIER_KIND_PATH);

    type->path = path;
}

static void type_specifier_init_func(ASTTypeSpecifier* type)
{
    FRX_ASSERT(type != NULL);

    type_specifier_init(type, FRX_TYPE_SPECIFIER_KIND_FUNC);
}


static void type_specifier_init_pointer(ASTTypeSpecifier* type, AST* base, b8 mutable)
{
    FRX_ASSERT(type != NULL);

    FRX_ASSERT(base != NULL);

    FRX_ASSERT(base->type == FRX_AST_TYPE_TYPE_SPECIFIER);

    type_specifier_init(type, FRX_TYPE_SPECIFIER_KIND_PTR);

    type->base = base;
    type->mutable = mutable;
}

static void type_specifier_init_array(ASTTypeSpecifier* type, AST* base, AST* size)
{
    FRX_ASSERT(type != NULL);

    FRX_ASSERT(base != NULL);

    FRX_ASSERT(base->type == FRX_AST_TYPE_TYPE_SPECIFIER);

    type_specifier_init(type, FRX_TYPE_SPECIFIER_KIND_ARRAY);

    type->base = base;
    type->size = size;
}

AST* type_specifier_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_TYPE_SPECIFIER);
    ASTTypeSpecifier* type = &ast->type_specifier;

    if (token_type_is_primitive(parser_current_type(parser))
        || parser_current_type(parser) == FRX_TOKEN_TYPE_IDENT
        || parser_current_type(parser) == FRX_TOKEN_TYPE_KW_SELF_UPPER)
    {
        AST* path = path_parse(parser, FRX_PATH_STYLE_TYPE);
        type_specifier_init_path(type, path);
    }
    else if (parser_current_type(parser) == FRX_TOKEN_TYPE_KW_FN)
    {
        type_specifier_init_func(type);
        parser_eat(parser, FRX_TOKEN_TYPE_KW_FN);
        parser_eat(parser, FRX_TOKEN_TYPE_LPAREN);

        while (!parser_match(parser, FRX_TOKEN_TYPE_RPAREN))
        {
            if (parser_current_type(parser) == FRX_TOKEN_TYPE_ELLIPSIS)
            {
                type->is_variadic = FRX_TRUE;
                parser_eat(parser, FRX_TOKEN_TYPE_ELLIPSIS);
                break;
            }

            AST* param_type = type_specifier_parse(parser);
            list_add(&type->func_params, param_type);

            if (parser_current_type(parser) == FRX_TOKEN_TYPE_COMMA)
            {
                parser_eat(parser, FRX_TOKEN_TYPE_COMMA);
            }
            else
            {
                break;
            }
        }

        parser_eat(parser, FRX_TOKEN_TYPE_RPAREN);
        parser_eat(parser, FRX_TOKEN_TYPE_ARROW);

        AST* return_type = type_specifier_parse(parser);
        type->func_return_type = return_type;
    }
    else if (parser_current_type(parser) == FRX_TOKEN_TYPE_LBRACKET)
    {
        parser_eat(parser, FRX_TOKEN_TYPE_LBRACKET);
        AST* base = type_specifier_parse(parser);
        parser_eat(parser, FRX_TOKEN_TYPE_SEMI);
        AST* size = expr_parse(parser);
        parser_eat(parser, FRX_TOKEN_TYPE_RBRACKET);

        type_specifier_init_array(type, base, size);
    }
    else
    {
        SourceRange range = parser_current_token(parser)->range;
        Diagnostic* d = diagnostic_create(FRX_DIAGNOSTIC_ID_EXPECTED_TYPE_SPECIFIER,
                                          FRX_DIAGNOSTIC_LVL_ERROR, range,
                                          token_type_to_str(parser_current_type(parser)));
        parser_add_diagnostic(parser, d);

        return ast_create(FRX_AST_TYPE_ERROR);
    }

    while (parser_match(parser, FRX_TOKEN_TYPE_STAR) ||
        parser_match(parser, FRX_TOKEN_TYPE_BIT_AND))
    {
        b8 mutable = parser_match(parser, FRX_TOKEN_TYPE_STAR);
        AST* pointer_type = ast_create(FRX_AST_TYPE_TYPE_SPECIFIER);
        type_specifier_init_pointer(&pointer_type->type_specifier, ast, mutable);
        ast = pointer_type;
        type = &pointer_type->type_specifier;

        parser_eat(parser, parser_current_type(parser));
    }

    return ast;
}

void type_specifier_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_TYPE_SPECIFIER);

    FRX_ASSERT(ctx != NULL);

    ASTTypeSpecifier* type_specifier = &ast->type_specifier;
    switch(type_specifier->kind)
    {
        case FRX_TYPE_SPECIFIER_KIND_PATH:
        {
            path_resolve(type_specifier->path, ctx);

            ASTPath* path = &type_specifier->path->path;
            switch (path->symbol->type)
            {
                case FRX_SYMBOL_TYPE_STRUCT: type_specifier->resolved_type = type_intern_struct(path->symbol, &((AST*)list_get(&path->path_segments, list_size(&path->path_segments) - 1))->path_segment.generic_args); break;
                case FRX_SYMBOL_TYPE_ENUM: type_specifier->resolved_type = type_intern_enum(path->symbol); break;

                default: type_specifier->resolved_type = symbol_infer_type(path->symbol); break;
            }

            break;
        }
        case FRX_TYPE_SPECIFIER_KIND_FUNC:
        {
            for (usize i = 0; i < list_size(&type_specifier->func_params); ++i)
            {
                AST* param = list_get(&type_specifier->func_params, i);
                type_specifier_resolve(param, ctx);
            }

            type_specifier_resolve(type_specifier->func_return_type, ctx);

            type_specifier->resolved_type = type_intern_func(&type_specifier->func_params, type_specifier->func_return_type->type_specifier.resolved_type, type_specifier->is_variadic);

            break;
        }
        case FRX_TYPE_SPECIFIER_KIND_PTR:
        {
            AST* base = type_specifier->base;
            type_specifier_resolve(base, ctx);
            type_specifier->resolved_type = type_intern_ptr(base->type_specifier.resolved_type, base->type_specifier.mutable);

            break;
        }
        case FRX_TYPE_SPECIFIER_KIND_ARRAY:
        {
            AST* base = type_specifier->base;
            type_specifier_resolve(base, ctx);
            type_specifier->resolved_type = type_intern_array(base->type_specifier.resolved_type, type_specifier->size);

            break;
        }

        default: FRX_ASSERT(FRX_FALSE); break;
    }
}
