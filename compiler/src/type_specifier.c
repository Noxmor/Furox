#include "assert.h"
#include "ast.h"
#include "diagnostics.h"
#include "parser.h"
#include "resolution.h"
#include "token.h"

static void generic_args_init(ASTGenericArgs* generic_args)
{
    FRX_ASSERT(generic_args != NULL);

    list_init(&generic_args->args);
}

static AST* generic_args_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_GENERIC_ARGS);
    ASTGenericArgs* generic_args = &ast->generic_args;

    ast->range.start = parser_current_location(parser);

    generic_args_init(generic_args);

    parser_eat(parser, FRX_TOKEN_TYPE_LT);

    while (!parser_match(parser, FRX_TOKEN_TYPE_GT))
    {
        if (!list_empty(&generic_args->args))
        {
            parser_eat(parser, FRX_TOKEN_TYPE_COMMA);
        }

        AST* arg = ast_create(FRX_AST_TYPE_GENERIC_ARG);
        arg->generic_arg.type = type_specifier_parse(parser);
        list_add(&generic_args->args, arg);
    }

    parser_eat(parser, FRX_TOKEN_TYPE_GT);

    ast->range.end = parser_current_location(parser);

    return ast;
}

static void type_specifier_init(ASTTypeSpecifier* type, ASTTypeSpecifierKind kind)
{
    FRX_ASSERT(type != NULL);

    FRX_ASSERT(kind < FRX_TYPE_SPECIFIER_KIND_COUNT);

    type->kind = kind;
    type->generic_args = NULL;
    type->resolved_type = NULL;
    type->path_expr = NULL;
}

static void type_specifier_init_primitive(ASTTypeSpecifier* type, TokenType primitive)
{
    FRX_ASSERT(type != NULL);

    FRX_ASSERT(token_type_is_primitive(primitive));

    type_specifier_init(type, FRX_TYPE_SPECIFIER_KIND_PRIMITIVE);

    type->primitive = primitive;
}

static void type_specifier_init_path_expr(ASTTypeSpecifier* type, AST* path_expr)
{
    FRX_ASSERT(type != NULL);

    FRX_ASSERT(path_expr != NULL);

    type_specifier_init(type, FRX_TYPE_SPECIFIER_KIND_PATH_EXPR);

    type->path_expr = path_expr;
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

AST* type_specifier_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_TYPE_SPECIFIER);
    ASTTypeSpecifier* type = &ast->type_specifier;

    if (token_type_is_primitive(parser_current_type(parser)))
    {
        TokenType primitive = parser_current_type(parser);
        parser_eat(parser, primitive);

        type_specifier_init_primitive(type, primitive);
    }
    else if (parser_current_type(parser) == FRX_TOKEN_TYPE_IDENT)
    {
        AST* path_expr = path_expr_parse(parser);
        type_specifier_init_path_expr(type, path_expr);
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

    if (parser_match(parser, FRX_TOKEN_TYPE_LT))
    {
        type->generic_args = generic_args_parse(parser);
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
        case FRX_TYPE_SPECIFIER_KIND_PRIMITIVE: type_specifier->resolved_type = type_create_primitive(type_specifier->primitive); break;
        case FRX_TYPE_SPECIFIER_KIND_PATH_EXPR:
        {
            path_expr_resolve(type_specifier->path_expr, ctx);
            type_specifier->resolved_type = symbol_infer_type(type_specifier->path_expr->path_expr.symbol);

            break;
        }
        case FRX_TYPE_SPECIFIER_KIND_PTR:
        {
            AST* base = type_specifier->base;
            type_specifier_resolve(base, ctx);
            type_specifier->resolved_type = type_create_ptr(base->type_specifier.resolved_type, base->type_specifier.mutable);

            break;
        }
        case FRX_TYPE_SPECIFIER_KIND_ARRAY:
        {
            AST* base = type_specifier->base;
            type_specifier_resolve(base, ctx);
            type_specifier->resolved_type = type_create_array(base->type_specifier.resolved_type, base->type_specifier.size);

            break;
        }

        default: FRX_ASSERT(FRX_FALSE); break;
    }
}
