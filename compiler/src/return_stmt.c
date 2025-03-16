#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "mir.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "codegen.h"

static ReturnStmt* return_stmt_create(Expr* value)
{
    ReturnStmt* return_stmt = compiler_alloc_ast(sizeof(ReturnStmt));

    return_stmt->value = value;

    return return_stmt;
}

ReturnStmt* return_stmt_parse(Parser* parser)
{
    parser_eat(parser, FRX_TOKEN_TYPE_KW_RETURN);

    Expr* value = NULL;

    if (!parser_match(parser, FRX_TOKEN_TYPE_SEMI))
    {
        value = expr_parse(parser);
    }

    parser_eat(parser, FRX_TOKEN_TYPE_SEMI);

    return return_stmt_create(value);
}

void return_stmt_resolve(Parser* parser, ReturnStmt* return_stmt)
{
    FRX_ASSERT(return_stmt != NULL);

    if (return_stmt->value != NULL)
    {
        expr_resolve(parser, return_stmt->value);
    }
}

void return_stmt_sema(ReturnStmt* return_stmt)
{
    FRX_ASSERT(return_stmt != NULL);

    if (return_stmt->value != NULL)
    {
        expr_sema(return_stmt->value);
    }
}

void return_stmt_lower_to_mir(ReturnStmt* return_stmt, MIRBlock* block)
{
    FRX_ASSERT(return_stmt != NULL);

    MIRVariable* dest = mir_variable_create_func_param(mir_type_create_primitive(FRX_MIR_TYPE_KIND_I32), "argc");
    MIRInstruction* instruction = mir_instruction_create(FRX_MIR_INSTRUCTION_TYPE_RET, dest, NULL, NULL);
    mir_block_add_instruction(block, instruction);
}
