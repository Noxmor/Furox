#include "assert.h"
#include "ast.h"
#include "parser.h"
#include "early_resolution.h"
#include "late_resolution.h"
#include "sema.h"

static void translation_unit_init(ASTTranslationUnit* unit)
{
    FRX_ASSERT(unit != NULL);

    unit->mod_decl = NULL;
    list_init(&unit->items);
}

static void translation_unit_add_item(ASTTranslationUnit* unit, AST* item)
{
    FRX_ASSERT(unit != NULL);
    FRX_ASSERT(item != NULL);

    list_add(&unit->items, item);
}

AST* translation_unit_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_TRANSLATION_UNIT);
    ASTTranslationUnit* unit = &ast->translation_unit;

    ast->range.start = parser_current_location(parser);

    translation_unit_init(unit);

    if (parser_match(parser, FRX_TOKEN_TYPE_KW_MOD))
    {
        unit->mod_decl = mod_decl_parse(parser);
    }

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

void translation_unit_resolve_early(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_TRANSLATION_UNIT);

    ASTTranslationUnit* unit = &ast->translation_unit;

    for (usize i = 0; i < list_size(&unit->items); ++i)
    {
        AST* item = list_get(&unit->items, i);
        ast_resolve_early(item, ctx);
    }
}

void translation_unit_resolve_late(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_TRANSLATION_UNIT);

    ASTTranslationUnit* unit = &ast->translation_unit;

    for (usize i = 0; i < list_size(&unit->items); ++i)
    {
        AST* item = list_get(&unit->items, i);
        ast_resolve_late(item, ctx);
    }
}

void translation_unit_sema(AST* ast, SemaContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_TRANSLATION_UNIT);

    FRX_ASSERT(ctx != NULL);

    ASTTranslationUnit* unit = &ast->translation_unit;

    for (usize i = 0; i < list_size(&unit->items); ++i)
    {
        AST* item = list_get(&unit->items, i);
        ast_sema(item, ctx);
    }
}
