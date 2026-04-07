#include "ast.h"
#include "compiler.h"
#include "parser.h"

AST* mod_decl_parse(Parser* parser)
{
    AST* ast = parser_create_ast(parser, FRX_AST_TYPE_MOD_DECL);
    ASTModDecl* mod_decl = &ast->mod_decl;

    ast->span.lo = parser_current_span(parser).lo;

    parser_eat(parser, FRX_TOKEN_TYPE_KW_MOD);

    mod_decl->path = NULL;
    if (parser_current_type(parser) != FRX_TOKEN_TYPE_SEMI)
    {
        mod_decl->path = path_parse(parser, FRX_PATH_STYLE_TYPE);
    }

    ast->span.hi = parser_current_span(parser).hi;
    parser_eat(parser, FRX_TOKEN_TYPE_SEMI);

    Module* root_mod = compiler_root_module();
    Module* current_mod = root_mod;

    for (usize i = 0; mod_decl->path != NULL && i < list_size(&mod_decl->path->path.path_segments); ++i)
    {
        AST* path_segment = list_get(&mod_decl->path->path.path_segments, i);
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

    return ast;
}
