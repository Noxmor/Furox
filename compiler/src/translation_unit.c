#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "sema.h"
#include "codegen.h"

static TranslationUnit* translation_unit_create(void)
{
    TranslationUnit* unit = compiler_alloc(sizeof(TranslationUnit));

    list_init(&unit->items);

    return unit;
}

static void translation_unit_add_item(TranslationUnit* unit, Item* item)
{
    FRX_ASSERT(unit != NULL);
    FRX_ASSERT(item != NULL);

    list_add(&unit->items, item);
}

TranslationUnit* translation_unit_parse(Parser* parser)
{
    TranslationUnit* unit = translation_unit_create();

    while (!parser_match(parser, FRX_TOKEN_TYPE_EOF))
    {
        Item* item = item_parse(parser);
        if (item == NULL)
        {
            continue;
        }

        translation_unit_add_item(unit, item);
    }

    return unit;
}

void translation_unit_sema(TranslationUnit* unit)
{
    FRX_ASSERT(unit != NULL);

    for (usize i = 0; i < list_size(&unit->items); ++i)
    {
        Item* item = list_get(&unit->items, i);
        item_sema(item);
    }
}

void translation_unit_codegen(TranslationUnit* unit)
{
    FRX_ASSERT(unit != NULL);

    for (usize i = 0; i < list_size(&unit->items); ++i)
    {
        Item* item = list_get(&unit->items, i);
        item_codegen(item);
    }
}
