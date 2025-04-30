#include "assert.h"
#include "ast.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "codegen.h"

static void translation_unit_init(TranslationUnit* unit)
{
    list_init(&unit->items);
}

static void translation_unit_add_item(TranslationUnit* unit, AST* item)
{
    FRX_ASSERT(unit != NULL);
    FRX_ASSERT(item != NULL);

    list_add(&unit->items, item);
}

AST* translation_unit_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_TRANSLATION_UNIT);
    TranslationUnit* unit = &ast->translation_unit;

    ast->range.start = parser_current_location(parser);

    translation_unit_init(unit);

    while (!parser_match(parser, FRX_TOKEN_TYPE_EOF))
    {
        AST* item = item_parse(parser);
        if (item == NULL)
        {
            continue;
        }

        translation_unit_add_item(unit, item);
    }

    ast->range.end = parser_current_location(parser);

    return ast;
}

void translation_unit_resolve(AST* ast, Parser* parser)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_TRANSLATION_UNIT);

    TranslationUnit* unit = &ast->translation_unit;

    for (usize i = 0; i < list_size(&unit->items); ++i)
    {
        AST* item = list_get(&unit->items, i);
        ast_resolve(item, parser);
    }
}

void translation_unit_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_TRANSLATION_UNIT);

    FRX_ASSERT(ctx != NULL);

    TranslationUnit* unit = &ast->translation_unit;

    for (usize i = 0; i < list_size(&unit->items); ++i)
    {
        AST* item = list_get(&unit->items, i);
        ast_sema(item, ctx);
    }
}

void translation_unit_codegen(AST* ast, CodegenContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_TRANSLATION_UNIT);

    FRX_ASSERT(ctx != NULL);

    TranslationUnit* unit = &ast->translation_unit;

    for (usize i = 0; i < list_size(&unit->items); ++i)
    {
        AST* item = list_get(&unit->items, i);
        ast_codegen(item, ctx);
    }
}
