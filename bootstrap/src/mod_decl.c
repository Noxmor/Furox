#include "ast.h"

#include "compiler.h"
#include "parser.h"

static AST* mod_decl_create(AST* path_expr)
{
    AST* ast = ast_create(FRX_AST_TYPE_MOD_DECL);

    ASTModDecl* mod_decl = &ast->mod_decl;
    mod_decl->path_expr = path_expr;

    return ast;
}

AST* mod_decl_parse(Parser* parser)
{
    parser_eat(parser, FRX_TOKEN_TYPE_KW_MOD);

    AST* path_expr = NULL;
    if (parser_current_type(parser) != FRX_TOKEN_TYPE_SEMI)
    {
        path_expr = path_expr_parse(parser, FRX_PATH_STYLE_TYPE);
    }

    parser_eat(parser, FRX_TOKEN_TYPE_SEMI);

    AST* mod_decl = mod_decl_create(path_expr);

    Module* root_mod = compiler_root_module();
    Module* current_mod = root_mod;

    for (usize i = 0; path_expr != NULL && i < list_size(&path_expr->path_expr.path_segments); ++i)
    {
        AST* path_segment = list_get(&path_expr->path_expr.path_segments, i);
        Module* submodule = module_find_submodule_by_name(current_mod, path_segment->path_segment.name);

        if (submodule == NULL)
        {
            current_mod = module_create(current_mod, path_segment->path_segment.name);
        }
        else
        {
            current_mod = submodule;
        }
    }

    parser->src_file->module = current_mod;

    return mod_decl;
}
