#include "ast.h"
#include "diagnostics.h"
#include "parser.h"
#include "symbol.h"
#include "token.h"

AST* item_parse(Parser* parser)
{
    SymbolVisibility visibility = parse_visibility(parser);

    switch (parser_current_type(parser))
    {
        case FRX_TOKEN_TYPE_KW_USE: return use_stmt_parse(parser);
        case FRX_TOKEN_TYPE_KW_EXTERN:
        case FRX_TOKEN_TYPE_KW_FN: return func_decl_parse(parser, visibility);
        case FRX_TOKEN_TYPE_KW_UNION:
        case FRX_TOKEN_TYPE_KW_STRUCT: return struct_def_parse(parser, visibility);
        case FRX_TOKEN_TYPE_KW_ENUM: return enum_def_parse(parser, visibility);
        case FRX_TOKEN_TYPE_KW_TYPE: return type_alias_parse(parser, visibility);
        case FRX_TOKEN_TYPE_KW_TRAIT: return trait_parse(parser, visibility);
        case FRX_TOKEN_TYPE_KW_IMPL: return impl_block_parse(parser);
        default:
        {
            Diagnostic* d = diagnostic_create(FRX_DIAGNOSTIC_ID_EXPECTED_ITEM,
                                              FRX_DIAGNOSTIC_LVL_ERROR,
                                              parser_current_token(parser)->range,
                                              token_type_to_str(parser_current_type(parser)));
            parser_add_diagnostic(parser, d);

            parser_recover(parser);

            return ast_create(FRX_AST_TYPE_ERROR);
        }
    }
}
