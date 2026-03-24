#include "ast.h"

#include "compiler.h"
#include "parser.h"

static AST* mod_decl_create(AST* path)
{
    AST* ast = ast_create(FRX_AST_TYPE_MOD_DECL);

    ASTModDecl* mod_decl = &ast->mod_decl;
    mod_decl->path = path;

    return ast;
}

AST* mod_decl_parse(Parser* parser)
{
    parser_eat(parser, FRX_TOKEN_TYPE_KW_MOD);

    AST* path = NULL;
    if (parser_current_type(parser) != FRX_TOKEN_TYPE_SEMI)
    {
        path = path_parse(parser, FRX_PATH_STYLE_TYPE);
    }

    parser_eat(parser, FRX_TOKEN_TYPE_SEMI);

    AST* mod_decl = mod_decl_create(path);

    Module* root_mod = compiler_root_module();
    Module* current_mod = root_mod;

    for (usize i = 0; path != NULL && i < list_size(&path->path.path_segments); ++i)
    {
        AST* path_segment = list_get(&path->path.path_segments, i);
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
