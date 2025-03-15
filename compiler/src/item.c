#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "diagnostics.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "token.h"

typedef void (*ItemResolveFunc)(Parser*, void*);

static const ItemResolveFunc item_type_to_resolve[FRX_ITEM_TYPE_COUNT] = {
    [FRX_ITEM_TYPE_ERROR] = (ItemResolveFunc)NULL,
    [FRX_ITEM_TYPE_USE_STMT] = (ItemResolveFunc)use_stmt_resolve,
    [FRX_ITEM_TYPE_FUNC_DECL] = (ItemResolveFunc)func_decl_resolve,
    [FRX_ITEM_TYPE_FUNC_DEF] = (ItemResolveFunc)func_def_resolve,
    [FRX_ITEM_TYPE_STRUCT_DEF] = (ItemResolveFunc)struct_def_resolve,
    [FRX_ITEM_TYPE_TRAIT] = (ItemResolveFunc)trait_resolve,
    [FRX_ITEM_TYPE_IMPL_BLOCK] = (ItemResolveFunc)impl_block_resolve,
};

typedef void (*ItemSemaFunc)(void*);

static const ItemSemaFunc item_type_to_sema[FRX_ITEM_TYPE_COUNT] = {
    [FRX_ITEM_TYPE_ERROR] = (ItemSemaFunc)NULL,
    [FRX_ITEM_TYPE_USE_STMT] = (ItemSemaFunc)NULL,
    [FRX_ITEM_TYPE_FUNC_DECL] = (ItemSemaFunc)func_decl_sema,
    [FRX_ITEM_TYPE_FUNC_DEF] = (ItemSemaFunc)func_def_sema,
    [FRX_ITEM_TYPE_STRUCT_DEF] = (ItemSemaFunc)struct_def_sema,
    [FRX_ITEM_TYPE_TRAIT] = (ItemSemaFunc)trait_sema,
    [FRX_ITEM_TYPE_IMPL_BLOCK] = (ItemSemaFunc)impl_block_sema,
};

static Item* item_create(ItemType type, void* node)
{
    FRX_ASSERT(type < FRX_ITEM_TYPE_COUNT);

    FRX_ASSERT((type == FRX_ITEM_TYPE_ERROR && node == NULL)
               || (type != FRX_ITEM_TYPE_ERROR && node != NULL));

    Item* item = compiler_alloc_ast(sizeof(Item));

    item->type = type;
    item->node = node;

    return item;
}

Item* item_parse(Parser* parser)
{
    if (parser_match(parser, FRX_TOKEN_TYPE_KW_PUB))
    {
        parser_eat(parser, FRX_TOKEN_TYPE_KW_PUB);

        parser->visibility = FRX_SYMBOL_VISIBILITY_PUBLIC;

        switch (parser_current_type(parser))
        {
            case FRX_TOKEN_TYPE_KW_FN: return item_create(FRX_ITEM_TYPE_FUNC_DEF, func_def_parse(parser));
            case FRX_TOKEN_TYPE_KW_STRUCT: return item_create(FRX_ITEM_TYPE_STRUCT_DEF, struct_def_parse(parser));
            case FRX_TOKEN_TYPE_KW_ENUM: return item_create(FRX_ITEM_TYPE_ENUM_DEF, enum_def_parse(parser));
            case FRX_TOKEN_TYPE_KW_TRAIT: return item_create(FRX_ITEM_TYPE_TRAIT, trait_parse(parser));
            default:
            {
                FRX_PARSER_ADD_DIAGNOSTIC(parser, FRX_DIAGNOSTIC_ID_EXPECTED_ITEM,
                                          FRX_DIAGNOSTIC_LVL_ERROR,
                                          parser_current_token(parser)->range,
                                          token_type_to_str(parser_current_type(parser)));
                parser_recover(parser);

                return item_create(FRX_ITEM_TYPE_ERROR, NULL);
            }
        }
    }

    parser->visibility = FRX_SYMBOL_VISIBILITY_PRIVATE;

    switch (parser_current_type(parser))
    {
        case FRX_TOKEN_TYPE_KW_USE: return item_create(FRX_ITEM_TYPE_USE_STMT, use_stmt_parse(parser));
        case FRX_TOKEN_TYPE_KW_EXTERN: parser_eat(parser, FRX_TOKEN_TYPE_KW_EXTERN); return item_create(FRX_ITEM_TYPE_FUNC_DECL, func_decl_parse(parser));
        case FRX_TOKEN_TYPE_KW_FN: return item_create(FRX_ITEM_TYPE_FUNC_DEF, func_def_parse(parser));
        case FRX_TOKEN_TYPE_KW_STRUCT: return item_create(FRX_ITEM_TYPE_STRUCT_DEF, struct_def_parse(parser));
        case FRX_TOKEN_TYPE_KW_ENUM: return item_create(FRX_ITEM_TYPE_ENUM_DEF, enum_def_parse(parser));
        case FRX_TOKEN_TYPE_KW_TRAIT: return item_create(FRX_ITEM_TYPE_TRAIT, trait_parse(parser));
        case FRX_TOKEN_TYPE_KW_IMPL: return item_create(FRX_ITEM_TYPE_IMPL_BLOCK, impl_block_parse(parser));
        default:
        {
            FRX_PARSER_ADD_DIAGNOSTIC(parser, FRX_DIAGNOSTIC_ID_EXPECTED_ITEM,
                                      FRX_DIAGNOSTIC_LVL_ERROR,
                                      parser_current_token(parser)->range,
                                      token_type_to_str(parser_current_type(parser)));
            parser_recover(parser);

            return item_create(FRX_ITEM_TYPE_ERROR, NULL);
        }
    }
}

void item_resolve(Parser* parser, Item* item)
{
    FRX_ASSERT(item != NULL);

    ItemResolveFunc func = item_type_to_resolve[item->type];
    if (func != NULL)
    {
        func(parser, item->node);
    }
}

void item_sema(Item* item)
{
    FRX_ASSERT(item != NULL);

    ItemSemaFunc func = item_type_to_sema[item->type];
    if (func != NULL)
    {
        func(item->node);
    }
}
