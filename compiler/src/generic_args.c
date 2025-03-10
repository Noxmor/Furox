#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "codegen.h"

static GenericArgs* generic_args_create(void)
{
    GenericArgs* args = compiler_alloc(sizeof(GenericArgs));

    args->error = FRX_FALSE;
    list_init(&args->args);

    return args;
}

GenericArgs* generic_args_parse(Parser* parser)
{
    GenericArgs* generic_args = generic_args_create();

    generic_args->error |= parser_eat(parser, FRX_TOKEN_TYPE_LT);

    while (!parser_match(parser, FRX_TOKEN_TYPE_GT))
    {
        if (!list_empty(&generic_args->args))
        {
            generic_args->error |= parser_eat(parser, FRX_TOKEN_TYPE_COMMA);
        }

        const char* name = parser_current_token(parser)->identifier;
        generic_args->error |= parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

        GenericArg* arg = compiler_alloc(sizeof(GenericArg));
        arg->name = name;

        list_add(&generic_args->args, arg);
    }

    generic_args->error |= parser_eat(parser, FRX_TOKEN_TYPE_GT);

    return generic_args;
}
