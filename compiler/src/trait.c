#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"

static Trait* trait_create(b8 error, const char* name)
{
    FRX_ASSERT(name != NULL);

    Trait* trait = compiler_alloc_ast(sizeof(Trait));

    trait->error = error;
    trait->name = name;
    list_init(&trait->methods);

    return trait;
}

Trait* trait_parse(Parser* parser)
{
    b8 error = FRX_FALSE;

    error |= parser_eat(parser, FRX_TOKEN_TYPE_KW_TRAIT);

    const char* name = parser_current_token(parser)->identifier;
    error |= parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    Trait* trait = trait_create(error, name);
    error |= parser_eat(parser, FRX_TOKEN_TYPE_LBRACE);

    while (!parser_match(parser, FRX_TOKEN_TYPE_RBRACE))
    {
        FuncDecl* func_decl = func_decl_parse(parser);
        list_add(&trait->methods, func_decl);
    }

    trait->error |= parser_eat(parser, FRX_TOKEN_TYPE_RBRACE);

    return trait;
}

void trait_resolve(Parser* parser, Trait* trait)
{
    FRX_ASSERT(trait != NULL);

    if (trait->error)
    {
        return;
    }

    for (usize i = 0; i < list_size(&trait->methods); ++i)
    {
        FuncDecl* func_decl = list_get(&trait->methods, i);
        func_decl_resolve(parser, func_decl);
    }
}

void trait_sema(Trait* trait)
{
    FRX_ASSERT(trait != NULL);

    if (trait->error)
    {
        return;
    }

    for (usize i = 0; i < list_size(&trait->methods); ++i)
    {
        FuncDecl* func_decl = list_get(&trait->methods, i);
        func_decl_sema(func_decl);
    }
}
