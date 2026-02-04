#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "module.h"
#include "parser.h"
#include "resolution.h"
#include "scope.h"
#include "symbol.h"
#include "token.h"
#include "type_system.h"

static void path_expr_init(ASTPathExpr* path_expr, ASTPathType path_type)
{
    FRX_ASSERT(path_expr != NULL);

    FRX_ASSERT(path_type < FRX_PATH_TYPE_COUNT);

    path_expr->type = path_type;
    list_init(&path_expr->path_segments);
    path_expr->scope = NULL;
    path_expr->mod = NULL;
    path_expr->symbol = NULL;
}

AST* path_expr_parse(Parser* parser)
{
    AST* ast = ast_create(FRX_AST_TYPE_PATH_EXPR);
    ASTPathExpr* path_expr = &ast->path_expr;

    ast->range.start = parser_current_location(parser);

    ASTPathType path_type = FRX_PATH_TYPE_ABSOLUTE;
    if (parser_current_type(parser) == FRX_TOKEN_TYPE_KW_EXTERN)
    {
        path_type = FRX_PATH_TYPE_EXTERN;
        parser_eat(parser, FRX_TOKEN_TYPE_KW_EXTERN);
        parser_eat(parser, FRX_TOKEN_TYPE_RESOLUTION);
    }
    else if (parser_current_type(parser) == FRX_TOKEN_TYPE_KW_MOD)
    {
        path_type = FRX_PATH_TYPE_MOD;
        parser_eat(parser, FRX_TOKEN_TYPE_KW_MOD);
        parser_eat(parser, FRX_TOKEN_TYPE_RESOLUTION);
    }

    path_expr_init(path_expr, path_type);

    path_expr->scope = parser->current_scope;
    path_expr->mod = parser->src_file->module;

    list_add(&path_expr->path_segments, (char*)parser_current_token(parser)->identifier);
    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    while (parser_match(parser, FRX_TOKEN_TYPE_RESOLUTION))
    {
        parser_eat(parser, FRX_TOKEN_TYPE_RESOLUTION);
        list_add(&path_expr->path_segments, (char*)parser_current_token(parser)->identifier);
        parser_eat(parser, FRX_TOKEN_TYPE_IDENT);
    }

    ast->range.end = parser_current_location(parser);

    compiler_register_expr(ast);

    return ast;
}
#include <stdio.h>
void path_expr_resolve(AST* ast, ResolutionContext* ctx)
{
    FRX_ASSERT(ast != NULL);

    FRX_ASSERT(ast->type == FRX_AST_TYPE_PATH_EXPR);

    FRX_ASSERT(ctx != NULL);

    ASTPathExpr* path_expr = &ast->path_expr;

    const char* path = list_get(&path_expr->path_segments, 0);
    Symbol* symbol = scope_lookup_symbol(path_expr->scope, path);
    Module* mod = module_find_submodule_by_name(path_expr->mod, path);
    if (mod == NULL)
    {
        mod = module_find_submodule_by_name(ctx->root_mod, path);
    }

    for (usize i = 1; i < list_size(&path_expr->path_segments) && (symbol != NULL || mod != NULL); ++i)
    {
        path = list_get(&path_expr->path_segments, i);

        if (symbol != NULL)
        {
            symbol = type_lookup_method(symbol_infer_type(symbol), path);
        }
        else if (mod != NULL)
        {
            symbol = module_lookup_symbol(mod, path);
            if (symbol == NULL)
            {
                mod = module_find_submodule_by_name(mod, path);
            }
        }
    }

    const char* name = list_get(&path_expr->path_segments, list_size(&path_expr->path_segments) - 1);

    if (symbol != NULL)
    {
        path_expr->symbol = symbol;
    }
    else
    {
        path_expr->symbol = symbol_create(name, FRX_SYMBOL_VISIBILITY_PRIVATE, FRX_SYMBOL_TYPE_MODULE, mod);
    }

    if (path_expr->symbol == NULL)
    {
        resolution_context_fail(ctx);
        printf("FAIL: %s\n", name);
    }
    else
    {
        printf("PASS: %s\n", name);
    }
}
