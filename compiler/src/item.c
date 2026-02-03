#include "ast.h"
#include "diagnostics.h"
#include "parser.h"
#include "token.h"

AST* item_parse(Parser* parser)
{
    if (parser_match(parser, FRX_TOKEN_TYPE_KW_PUB))
    {
        parser_eat(parser, FRX_TOKEN_TYPE_KW_PUB);

        parser->visibility = FRX_SYMBOL_VISIBILITY_PUBLIC;

        switch (parser_current_type(parser))
        {
            case FRX_TOKEN_TYPE_KW_FN: return func_decl_parse(parser);
            case FRX_TOKEN_TYPE_KW_STRUCT: return struct_def_parse(parser);
            case FRX_TOKEN_TYPE_KW_ENUM: return enum_def_parse(parser);
            case FRX_TOKEN_TYPE_KW_TRAIT: return trait_parse(parser);
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

    parser->visibility = FRX_SYMBOL_VISIBILITY_PRIVATE;

    switch (parser_current_type(parser))
    {
        case FRX_TOKEN_TYPE_KW_USE: return use_stmt_parse(parser);
        case FRX_TOKEN_TYPE_KW_EXTERN: parser_eat(parser, FRX_TOKEN_TYPE_KW_EXTERN); return func_decl_parse(parser);
        case FRX_TOKEN_TYPE_KW_FN: return func_decl_parse(parser);
        case FRX_TOKEN_TYPE_KW_STRUCT: return struct_def_parse(parser);
        case FRX_TOKEN_TYPE_KW_ENUM: return enum_def_parse(parser);
        case FRX_TOKEN_TYPE_KW_TRAIT: return trait_parse(parser);
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
