#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"

static ImplBlock* impl_block_create(const char* type_name)
{
    FRX_ASSERT(type_name != NULL);

    ImplBlock* impl_block = compiler_alloc_ast(sizeof(Trait));

    impl_block->type_name = type_name;
    list_init(&impl_block->methods);

    return impl_block;
}

ImplBlock* impl_block_parse(Parser* parser)
{
    parser_eat(parser, FRX_TOKEN_TYPE_KW_IMPL);

    const char* type_name = parser_current_token(parser)->identifier;
    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    ImplBlock* impl_block = impl_block_create(type_name);
    parser_eat(parser, FRX_TOKEN_TYPE_LBRACE);

    while (!parser_match(parser, FRX_TOKEN_TYPE_RBRACE))
    {
        FuncDef* func_def = func_def_parse(parser);
        list_add(&impl_block->methods, func_def);
    }

    parser_eat(parser, FRX_TOKEN_TYPE_RBRACE);

    return impl_block;
}

void impl_block_resolve(Parser* parser, ImplBlock* impl_block)
{
    FRX_ASSERT(impl_block != NULL);

    for (usize i = 0; i < list_size(&impl_block->methods); ++i)
    {
        FuncDef* func_def = list_get(&impl_block->methods, i);
        func_def_resolve(parser, func_def);
    }
}

void impl_block_sema(ImplBlock* impl_block)
{
    FRX_ASSERT(impl_block != NULL);

    for (usize i = 0; i < list_size(&impl_block->methods); ++i)
    {
        FuncDef* func_def = list_get(&impl_block->methods, i);
        func_def_sema(func_def);
    }
}
