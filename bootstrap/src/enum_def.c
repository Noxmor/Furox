#include "assert.h"
#include "ast.h"
#include "parser.h"
#include "early_resolution.h"
#include "sema.h"
#include "symbol.h"

static void enum_variant_init(ASTEnumVariant* variant, const char* name)
{
    FRX_ASSERT(variant != NULL);

    FRX_ASSERT(name != NULL);

    variant->name = name;
    variant->value = NULL;
    variant->symbol = NULL;
}

static AST* enum_variant_parse(Parser* parser)
{
    AST* ast = parser_create_ast(parser, FRX_AST_TYPE_ENUM_VARIANT);
    ASTEnumVariant* variant = &ast->enum_variant;

    ast->span.lo = parser_current_span(parser).lo;

    const char* name = parser_current_token(parser)->identifier;
    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    enum_variant_init(variant, name);

    if (parser_match(parser, FRX_TOKEN_TYPE_EQ))
    {
        parser_eat(parser, FRX_TOKEN_TYPE_EQ);
        variant->value = expr_parse(parser);
    }

    ast->span.hi = parser_current_span(parser).hi;
    parser_eat(parser, FRX_TOKEN_TYPE_SEMI);

    return ast;
}

static void enum_def_init(ASTEnumDef* enum_def, const char* name, AST* type)
{
    FRX_ASSERT(name != NULL);

    FRX_ASSERT(type != NULL);

    enum_def->name = name;
    enum_def->type = type;
    list_init(&enum_def->variants);
}

static void enum_def_add_variant(ASTEnumDef* enum_def, AST* variant)
{
    FRX_ASSERT(enum_def != NULL);

    FRX_ASSERT(variant != NULL);

    FRX_ASSERT(variant->type == FRX_AST_TYPE_ENUM_VARIANT);

    list_add(&enum_def->variants, variant);
}

AST* enum_def_parse(Parser* parser, SymbolVisibility visibility)
{
    AST* ast = parser_create_ast(parser, FRX_AST_TYPE_ENUM_DEF);
    ASTEnumDef* enum_def = &ast->enum_def;

    ast->span.lo = parser_current_span(parser).lo;

    parser_eat(parser, FRX_TOKEN_TYPE_KW_ENUM);

    const char* name = parser_current_token(parser)->identifier;
    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    parser_eat(parser, FRX_TOKEN_TYPE_COLON);

    AST* type = type_specifier_parse(parser);

    enum_def_init(enum_def, name, type);

    parser_eat(parser, FRX_TOKEN_TYPE_LBRACE);
    while (!parser_match(parser, FRX_TOKEN_TYPE_RBRACE))
    {
        AST* variant = enum_variant_parse(parser);
        enum_def_add_variant(enum_def, variant);
    }

    ast->span.hi = parser_current_span(parser).hi;
    parser_eat(parser, FRX_TOKEN_TYPE_RBRACE);

    Symbol* symbol = parser_insert_symbol(parser, visibility, FRX_SYMBOL_TYPE_ENUM,
                         name, enum_def);

    for (usize i = 0; i < list_size(&enum_def->variants); ++i)
    {
        AST* variant = list_get(&enum_def->variants, i);
        variant->enum_variant.symbol = symbol;
    }

    return ast;
}

void enum_def_resolve_early(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_ENUM_DEF);

    ASTEnumDef* enum_def = &ast->enum_def;

    ast_resolve(enum_def->type, ctx);
}

void enum_def_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_ENUM_DEF);

    FRX_ASSERT(ctx != NULL);
}
