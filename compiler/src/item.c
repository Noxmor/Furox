#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "diagnostics.h"
#include "parser.h"
#include "sema.h"
#include "codegen.h"
#include "token.h"

typedef void (*ItemSemaFunc)(void*);

static const ItemSemaFunc item_type_to_sema[FRX_ITEM_TYPE_COUNT] = {
    [FRX_ITEM_TYPE_FUNC_DEF] = (ItemSemaFunc)func_def_sema
};

typedef void (*ItemCodegenFunc)(void*);

static const ItemSemaFunc item_type_to_codegen[FRX_ITEM_TYPE_COUNT] = {
    [FRX_ITEM_TYPE_FUNC_DEF] = (ItemCodegenFunc)func_def_codegen
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
        case FRX_TOKEN_TYPE_KW_FN: return item_create(FRX_ITEM_TYPE_FUNC_DEF, func_def_parse(parser));
        default:
        {
            FRX_PARSER_ADD_DIAGNOSTIC(parser, FRX_DIAGNOSTIC_ID_EXPECTED_ITEM,
                                      FRX_DIAGNOSTIC_LVL_ERROR,
                                      parser_current_token(parser)->range,
                                      token_type_to_str(parser_current_type(parser)));
            parser_recover(parser);

            return NULL;
        }
    }
}

void item_sema(Item* item)
{
    FRX_ASSERT(item != NULL);

    item_type_to_sema[item->type](item->node);
}

void item_codegen(Item* item)
{
    FRX_ASSERT(item != NULL);

    item_type_to_codegen[item->type](item->node);
}
