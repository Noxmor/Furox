#include "assert.h"
#include "ast.h"
#include "parser.h"
#include "early_resolution.h"
#include "sema.h"
#include "symbol.h"

static void trait_init(ASTTrait* trait, const char* name)
{
    FRX_ASSERT(name != NULL);

    trait->name = name;
    list_init(&trait->methods);
}

AST* trait_parse(Parser* parser, SymbolVisibility visibility)
{
    parser_push_scope(parser);

    AST* ast = parser_create_ast(parser, FRX_AST_TYPE_TRAIT);
    ASTTrait* trait = &ast->trait;

    ast->span.lo = parser_current_span(parser).lo;

    parser_eat(parser, FRX_TOKEN_TYPE_KW_TRAIT);

    const char* name = parser_current_token(parser)->identifier;
    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    trait_init(trait, name);
    parser_eat(parser, FRX_TOKEN_TYPE_LBRACE);

    while (!parser_match(parser, FRX_TOKEN_TYPE_RBRACE))
    {
        AST* func_decl = func_decl_parse(parser, FRX_SYMBOL_VISIBILITY_PRIVATE);
        list_add(&trait->methods, func_decl);
    }

    ast->span.hi = parser_current_span(parser).hi;
    parser_eat(parser, FRX_TOKEN_TYPE_RBRACE);

    parser_pop_scope(parser);

    parser_insert_symbol(parser, visibility, FRX_SYMBOL_TYPE_TRAIT,
                         trait->name, ast);

    return ast;
}

void trait_resolve_early(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_TRAIT);

    ASTTrait* trait = &ast->trait;

    for (usize i = 0; i < list_size(&trait->methods); ++i)
    {
        AST* func_decl = list_get(&trait->methods, i);
        func_decl_resolve_early(func_decl, ctx);
    }
}

void trait_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_TRAIT);

    FRX_ASSERT(ctx != NULL);

    ASTTrait* trait = &ast->trait;

    for (usize i = 0; i < list_size(&trait->methods); ++i)
    {
        AST* func_decl = list_get(&trait->methods, i);
        func_decl_sema(func_decl, ctx);
    }
}
