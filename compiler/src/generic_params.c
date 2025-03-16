#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"

static TraitBound* trait_bound_create(TypeSpecifier* type)
{
    FRX_ASSERT(type != NULL);

    TraitBound* trait_bound = compiler_alloc_ast(sizeof(TraitBound));

    trait_bound->type = type;

    return trait_bound;
}

static TraitBound* trait_bound_parse(Parser* parser)
{
    return trait_bound_create(type_specifier_parse(parser));
}

static GenericParam* generic_param_create(const char* name)
{
    GenericParam* generic_param = compiler_alloc_ast(sizeof(GenericParam));

    generic_param->name = name;
    list_init(&generic_param->trait_bounds);

    return generic_param;
}

static void generic_param_add_trait_bound(GenericParam* generic_param,
                                          TraitBound* trait_bound)
{
    FRX_ASSERT(generic_param != NULL);

    FRX_ASSERT(trait_bound != NULL);

    list_add(&generic_param->trait_bounds, trait_bound);
}

static GenericParam* generic_param_parse(Parser* parser)
{
    const char* name = parser_current_token(parser)->identifier;
    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    GenericParam* generic_param = generic_param_create(name);

    if (parser_match(parser, FRX_TOKEN_TYPE_COLON))
    {
        parser_eat(parser, FRX_TOKEN_TYPE_COLON);

        TraitBound* trait_bound = trait_bound_parse(parser);
        generic_param_add_trait_bound(generic_param, trait_bound);

        while (parser_match(parser, FRX_TOKEN_TYPE_PLUS))
        {
            parser_eat(parser, FRX_TOKEN_TYPE_PLUS);
            trait_bound = trait_bound_parse(parser);
            generic_param_add_trait_bound(generic_param, trait_bound);
        }
    }

    return generic_param;
}

static GenericParams* generic_params_create(void)
{
    GenericParams* params = compiler_alloc_ast(sizeof(GenericParams));

    list_init(&params->params);

    return params;
}

static void generic_params_add_param(GenericParams* generic_params,
                                     GenericParam* generic_param)
{
    FRX_ASSERT(generic_params != NULL);

    FRX_ASSERT(generic_param != NULL);

    list_add(&generic_params->params, generic_param);
}

GenericParams* generic_params_parse(Parser* parser)
{
    GenericParams* generic_params = generic_params_create();

    parser_eat(parser, FRX_TOKEN_TYPE_LT);

    while (!parser_match(parser, FRX_TOKEN_TYPE_GT))
    {
        if (!list_empty(&generic_params->params))
        {
            parser_eat(parser, FRX_TOKEN_TYPE_COMMA);
        }

        GenericParam* param = generic_param_parse(parser);
        generic_params_add_param(generic_params, param);
    }

    parser_eat(parser, FRX_TOKEN_TYPE_GT);

    return generic_params;
}
