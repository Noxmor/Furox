#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "codegen.h"

static GenericParams* generic_params_create(void)
{
    GenericParams* params = compiler_alloc_ast(sizeof(GenericParams));

    params->error = FRX_FALSE;
    list_init(&params->params);

    return params;
}

GenericParams* generic_params_parse(Parser* parser)
{
    GenericParams* generic_params = generic_params_create();

    generic_params->error |= parser_eat(parser, FRX_TOKEN_TYPE_LT);

    while (!parser_match(parser, FRX_TOKEN_TYPE_GT))
    {
        if (!list_empty(&generic_params->params))
        {
            generic_params->error |= parser_eat(parser, FRX_TOKEN_TYPE_COMMA);
        }

        const char* name = parser_current_token(parser)->identifier;
        generic_params->error |= parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

        GenericParam* param = compiler_alloc_ast(sizeof(GenericParam));
        param->name = name;

        list_add(&generic_params->params, param);
    }

    generic_params->error |= parser_eat(parser, FRX_TOKEN_TYPE_GT);

    return generic_params;
}
