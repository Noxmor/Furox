#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "diagnostics.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "codegen.h"
#include "token.h"

typedef void (*ItemResolveFunc)(Parser*, void*);

static const ItemResolveFunc item_type_to_resolve[FRX_ITEM_TYPE_COUNT] = {
    [FRX_ITEM_TYPE_ERROR] = (ItemResolveFunc)NULL,
    [FRX_ITEM_TYPE_USE_STMT] = (ItemResolveFunc)use_stmt_resolve,
    [FRX_ITEM_TYPE_FUNC_DEF] = (ItemResolveFunc)func_def_resolve,
    [FRX_ITEM_TYPE_STRUCT_DEF] = (ItemResolveFunc)struct_def_resolve,
};

typedef void (*ItemSemaFunc)(void*);

static const ItemSemaFunc item_type_to_sema[FRX_ITEM_TYPE_COUNT] = {
    [FRX_ITEM_TYPE_ERROR] = (ItemSemaFunc)NULL,
    [FRX_ITEM_TYPE_USE_STMT] = (ItemSemaFunc)NULL,
    [FRX_ITEM_TYPE_FUNC_DEF] = (ItemSemaFunc)func_def_sema,
    [FRX_ITEM_TYPE_STRUCT_DEF] = (ItemSemaFunc)struct_def_sema,
};

typedef void (*ItemCodegenFunc)(void*);

static const ItemSemaFunc item_type_to_codegen[FRX_ITEM_TYPE_COUNT] = {
    [FRX_ITEM_TYPE_ERROR] = (ItemCodegenFunc)NULL,
    [FRX_ITEM_TYPE_USE_STMT] = (ItemCodegenFunc)NULL,
    [FRX_ITEM_TYPE_FUNC_DEF] = (ItemCodegenFunc)func_def_codegen,
    [FRX_ITEM_TYPE_STRUCT_DEF] = (ItemCodegenFunc)struct_def_codegen,
};

static Item* item_create(ItemType type, void* node)
{
    FRX_ASSERT(type < FRX_ITEM_TYPE_COUNT);

    Item* item = compiler_alloc(sizeof(Item));

    item->type = type;
    item->node = node;

    return item;
}

Item* item_parse(Parser* parser)
{
    switch (parser_current_type(parser))
    {
        case FRX_TOKEN_TYPE_KW_USE: return item_create(FRX_ITEM_TYPE_USE_STMT, use_stmt_parse(parser));
        case FRX_TOKEN_TYPE_KW_FN: return item_create(FRX_ITEM_TYPE_FUNC_DEF, func_def_parse(parser));
        case FRX_TOKEN_TYPE_KW_STRUCT: return item_create(FRX_ITEM_TYPE_STRUCT_DEF, struct_def_parse(parser));
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

void item_codegen(Item* item)
{
    FRX_ASSERT(item != NULL);
    FRX_ASSERT(item->type != FRX_ITEM_TYPE_ERROR);

    ItemCodegenFunc func = item_type_to_codegen[item->type];
    if (func != NULL)
    {
        func(item->node);
    }
}
