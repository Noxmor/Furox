#include "ast.h"
#include "compiler.h"
#include "parser.h"

AST* stmt_parse(Parser* parser)
{
    switch (parser_current_type(parser))
    {
        case FRX_TOKEN_TYPE_IDENT:
        case FRX_TOKEN_TYPE_LPAREN:
        case FRX_TOKEN_TYPE_KW_EXTERN:
        case FRX_TOKEN_TYPE_KW_SELF_LOWER:
        case FRX_TOKEN_TYPE_INT_LIT:
        case FRX_TOKEN_TYPE_PLUS:
        case FRX_TOKEN_TYPE_MINUS:
        case FRX_TOKEN_TYPE_STAR:
        case FRX_TOKEN_TYPE_BIT_AND:
        case FRX_TOKEN_TYPE_LOG_NOT:
        case FRX_TOKEN_TYPE_BIT_NOT:
        case FRX_TOKEN_TYPE_PLUS_PLUS:
        case FRX_TOKEN_TYPE_MINUS_MINUS: return expr_stmt_parse(parser);
        case FRX_TOKEN_TYPE_KW_BREAK: return break_stmt_parse(parser);
        case FRX_TOKEN_TYPE_KW_CONTINUE: return continue_stmt_parse(parser);
        case FRX_TOKEN_TYPE_KW_RETURN: return return_stmt_parse(parser);
        case FRX_TOKEN_TYPE_KW_LET: return let_stmt_parse(parser);
        case FRX_TOKEN_TYPE_KW_IF: return if_stmt_parse(parser);
        case FRX_TOKEN_TYPE_KW_FOR: return for_loop_parse(parser);
        case FRX_TOKEN_TYPE_KW_WHILE: return while_loop_parse(parser);
        case FRX_TOKEN_TYPE_KW_LOOP: return loop_parse(parser);
        case FRX_TOKEN_TYPE_LBRACE: return block_parse(parser);
        default:
        {
            Diagnostic* d = diagnostic_create(FRX_DIAGNOSTIC_ID_EXPECTED_STMT,
                                              FRX_DIAGNOSTIC_LVL_ERROR,
                                              parser_current_span(parser),
                                              token_type_to_str(parser_current_type(parser)));
            compiler_add_diagnostic(d);

            parser_recover(parser);

            return parser_create_ast(parser, FRX_AST_TYPE_ERROR);
        }
    }
}
