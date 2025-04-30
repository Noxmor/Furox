#include "assert.h"
#include "ast.h"
#include "parser.h"

static AST* trait_bound_create(AST* type)
{
    FRX_ASSERT(type != NULL);

    AST* ast = ast_create(FRX_AST_TYPE_TRAIT_BOUND);
    TraitBound* trait_bound = &ast->trait_bound;

    trait_bound->type = type;

    return ast;
}

static AST* trait_bound_parse(Parser* parser)
{
    return trait_bound_create(type_specifier_parse(parser));
}

static void generic_param_init(GenericParam* generic_param, const char* name)
{
    generic_param->name = name;
    list_init(&generic_param->trait_bounds);
}

static void generic_param_add_trait_bound(GenericParam* generic_param,
                                          AST* trait_bound)
{
    FRX_ASSERT(generic_param != NULL);

    FRX_ASSERT(trait_bound != NULL);

    list_add(&generic_param->trait_bounds, trait_bound);
}

static AST* generic_param_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_GENERIC_PARAM);
    GenericParam* generic_param = &ast->generic_param;

    ast->range.start = parser_current_location(parser);

    const char* name = parser_current_token(parser)->identifier;
    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    generic_param_init(generic_param, name);

    if (parser_match(parser, FRX_TOKEN_TYPE_COLON))
    {
        parser_eat(parser, FRX_TOKEN_TYPE_COLON);

        AST* trait_bound = trait_bound_parse(parser);
        generic_param_add_trait_bound(generic_param, trait_bound);

        while (parser_match(parser, FRX_TOKEN_TYPE_PLUS))
        {
            parser_eat(parser, FRX_TOKEN_TYPE_PLUS);
            trait_bound = trait_bound_parse(parser);
            generic_param_add_trait_bound(generic_param, trait_bound);
        }
    }

    ast->range.end = parser_current_location(parser);

    return ast;
}

static void generic_params_init(GenericParams* params)
{
    list_init(&params->params);
}

static void generic_params_add_param(GenericParams* generic_params,
                                     AST* generic_param)
{
    FRX_ASSERT(generic_params != NULL);

    FRX_ASSERT(generic_param != NULL);

    list_add(&generic_params->params, generic_param);
}

AST* generic_params_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_GENERIC_PARAMS);
    GenericParams* generic_params = &ast->generic_params;

    ast->range.start = parser_current_location(parser);

    generic_params_init(generic_params);

    parser_eat(parser, FRX_TOKEN_TYPE_LT);

    while (!parser_match(parser, FRX_TOKEN_TYPE_GT))
    {
        if (!list_empty(&generic_params->params))
        {
            parser_eat(parser, FRX_TOKEN_TYPE_COMMA);
        }

        AST* param = generic_param_parse(parser);
        generic_params_add_param(generic_params, param);
    }

    parser_eat(parser, FRX_TOKEN_TYPE_GT);

    ast->range.end = parser_current_location(parser);

    return ast;
}
