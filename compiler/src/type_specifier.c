#include "assert.h"
#include "ast.h"
#include "codegen.h"
#include "diagnostics.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "symbol_registry.h"
#include "symbol_table.h"
#include "token.h"

static void generic_args_init(GenericArgs* generic_args)
{
    FRX_ASSERT(generic_args != NULL);

    list_init(&generic_args->args);
}

static AST* generic_args_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_GENERIC_ARGS);
    GenericArgs* generic_args = &ast->generic_args;

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

static void type_specifier_init(TypeSpecifier* type, TypeKind kind)
{
    FRX_ASSERT(type != NULL);

    FRX_ASSERT(kind < FRX_TYPE_KIND_COUNT);

    type->kind = kind;
    type->generic_args = NULL;
}

static void type_specifier_init_unresolved(TypeSpecifier* type, const char* name)
{
    FRX_ASSERT(type != NULL);

    FRX_ASSERT(name != NULL);

    type_specifier_init(type, FRX_TYPE_KIND_UNRESOLVED);

    type->name = name;
}

static void type_specifier_init_primitive(TypeSpecifier* type, TokenType primitive)
{
    FRX_ASSERT(type != NULL);

    FRX_ASSERT(token_type_is_primitive(primitive));

    type_specifier_init(type, FRX_TYPE_KIND_PRIMITIVE);

    type->primitive = primitive;
}

static void type_specifier_init_pointer(TypeSpecifier* type, AST* base, b8 mutable)
{
    FRX_ASSERT(type != NULL);

    FRX_ASSERT(base != NULL);

    FRX_ASSERT(base->type == FRX_AST_TYPE_TYPE_SPECIFIER);

    type_specifier_init(type, FRX_TYPE_KIND_POINTER);

    type->ptr.base = base;
    type->ptr.mutable = mutable;
}

AST* type_specifier_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_TYPE_SPECIFIER);
    TypeSpecifier* type = &ast->type_specifier;

    if (token_type_is_primitive(parser_current_type(parser)))
    {
        TokenType primitive = parser_current_type(parser);
        parser_eat(parser, primitive);

        type_specifier_init_primitive(type, primitive);
    }
    else if (parser_current_type(parser) == FRX_TOKEN_TYPE_IDENT)
    {
        const char* name = parser_current_token(parser)->identifier;
        parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

        type_specifier_init_unresolved(type, name);
    }
    else
    {
        SourceRange range = parser_current_token(parser)->range;
        FRX_PARSER_ADD_DIAGNOSTIC(parser, FRX_DIAGNOSTIC_ID_EXPECTED_TYPE_SPECIFIER,
                                  FRX_DIAGNOSTIC_LVL_ERROR, range, token_type_to_str(parser_current_type(parser)));

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

void type_specifier_resolve(AST* ast, Parser* parser)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_TYPE_SPECIFIER);

    FRX_ASSERT(parser != NULL);

    TypeSpecifier* type = &ast->type_specifier;

    switch (type->kind)
    {
        case FRX_TYPE_KIND_UNRESOLVED:
        {
            void* symbol = parser_lookup_symbol(parser, FRX_SYMBOL_TYPE_STRUCT, type->name);
            if (symbol != NULL)
            {
                type->kind = FRX_TYPE_KIND_STRUCT;
            }

            break;
        }
        case FRX_TYPE_KIND_PRIMITIVE: break;
        case FRX_TYPE_KIND_ENUM: break;
        case FRX_TYPE_KIND_STRUCT: break;
        case FRX_TYPE_KIND_UNION: break;
        case FRX_TYPE_KIND_POINTER:
        case FRX_TYPE_KIND_ARRAY: type_specifier_resolve(type->ptr.base, parser); break;
        default: FRX_ASSERT(FRX_FALSE); break;
    }
}

void type_specifier_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_TYPE_SPECIFIER);

    FRX_ASSERT(ctx != NULL);

    TypeSpecifier* type = &ast->type_specifier;
    (void)type;

    // TODO: Implement
}

void type_specifier_codegen(AST* ast, FILE* f)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_TYPE_SPECIFIER);

    FRX_ASSERT(f != NULL);

    TypeSpecifier* type = &ast->type_specifier;

    switch (type->kind)
    {
        case FRX_TYPE_KIND_PRIMITIVE: fprintf(f, "%s", token_type_to_str(type->primitive)); break;
        case FRX_TYPE_KIND_ENUM: break;
        case FRX_TYPE_KIND_STRUCT: break;
        case FRX_TYPE_KIND_UNION: break;
        case FRX_TYPE_KIND_POINTER: type_specifier_codegen(type->ptr.base, f); fprintf(f, "*"); break;
        case FRX_TYPE_KIND_ARRAY: break;
        default: FRX_ASSERT(FRX_FALSE); break;
    }
}
