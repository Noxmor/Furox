#include "ast.h"
#include "assert.h"
#include "module.h"
#include "parser.h"
#include "early_resolution.h"
#include "symbol_table.h"
#include "token.h"

AST* use_tree_parse(Parser* parser)
{
    FRX_ASSERT(parser != NULL);

    AST* ast = ast_create(FRX_AST_TYPE_USE_TREE);
    ASTUseTree* use_tree = &ast->use_tree;
    use_tree->path_segment = NULL;
    list_init(&use_tree->childs);

    use_tree->type = FRX_AST_USE_TREE_TYPE_SIMPLE;
    use_tree->path_segment = parser_current_token(parser)->identifier;

    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    if (parser_match(parser, FRX_TOKEN_TYPE_RESOLUTION))
    {
        parser_eat(parser, FRX_TOKEN_TYPE_RESOLUTION);

        if (parser_match(parser, FRX_TOKEN_TYPE_STAR))
        {
            parser_eat(parser, FRX_TOKEN_TYPE_STAR);

            use_tree->type = FRX_AST_USE_TREE_TYPE_GLOB;
            return ast;
        }

        if (parser_match(parser, FRX_TOKEN_TYPE_LBRACE))
        {
            parser_eat(parser, FRX_TOKEN_TYPE_LBRACE);

            use_tree->type = FRX_AST_USE_TREE_TYPE_NESTED;

            while (!parser_match(parser, FRX_TOKEN_TYPE_RBRACE))
            {
                AST* child = use_tree_parse(parser);
                list_add(&use_tree->childs, child);

                if (parser_match(parser, FRX_TOKEN_TYPE_COMMA))
                {
                    parser_eat(parser, FRX_TOKEN_TYPE_COMMA);
                }
                else
                {
                    break;
                }
            }

            parser_eat(parser, FRX_TOKEN_TYPE_RBRACE);

            return ast;
        }

        use_tree->type = FRX_AST_USE_TREE_TYPE_NESTED;
        AST* child = use_tree_parse(parser);
        list_add(&use_tree->childs, child);

        return ast;
    }

    // TODO: Handle 'as'

    return ast;
}

static void use_tree_resolve_early_impl(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_USE_TREE);

    FRX_ASSERT(ctx != NULL);

    ASTUseTree* use_tree = &ast->use_tree;

    Module* mod = ctx->current_mod;

    if (use_tree->path_segment != NULL)
    {
        mod = module_find_submodule_by_name(ctx->current_mod, use_tree->path_segment);
    }

    switch (use_tree->type)
    {
        case FRX_AST_USE_TREE_TYPE_SIMPLE:
        {
            Symbol* symbol = module_lookup_symbol(ctx->current_mod, use_tree->path_segment);
            if (symbol != NULL)
            {
                symbol_table_insert_symbol(&ctx->src_file->global_scope->symbols, symbol);
            }

            break;
        }
        case FRX_AST_USE_TREE_TYPE_GLOB:
        {
            SymbolTable* table = &mod->symbol_table;
            for (usize i = 0; i < FRX_SYMBOL_TABLE_CAPACITY; ++i)
            {
                SymbolTableEntry* entry = table->entries[i];
                while (entry != NULL)
                {
                    symbol_table_insert_symbol(&ctx->src_file->global_scope->symbols, entry->symbol);
                    entry = entry->next;
                }
            }

            break;
        }
        case FRX_AST_USE_TREE_TYPE_NESTED:
        {
            ctx->current_mod = mod;

            for (usize i = 0; i < list_size(&use_tree->childs); ++i)
            {
                AST* child = list_get(&use_tree->childs, i);
                use_tree_resolve_early_impl(child, ctx);
            }

            ctx->current_mod = ctx->current_mod->parent;

            break;
        }

        default: FRX_ASSERT(FRX_FALSE); break;
    }
}

void use_tree_resolve_early(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_USE_TREE);

    FRX_ASSERT(ctx != NULL);

    ASTUseTree* use_tree = &ast->use_tree;

    ctx->current_mod = ctx->src_file->module;
    if (module_lookup_symbol(ctx->current_mod, use_tree->path_segment) == NULL)
    {
        Module* mod = module_find_submodule_by_name(ctx->src_file->module, use_tree->path_segment);
        if (mod == NULL)
        {
            ctx->current_mod = ctx->root_mod;
        }
    }

    use_tree_resolve_early_impl(ast, ctx);

    ctx->current_mod = NULL;
}
