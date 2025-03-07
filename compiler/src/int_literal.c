#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "codegen.h"

static IntLiteral* int_literal_create(b8 error, u64 value)
{
    IntLiteral* literal = compiler_alloc(sizeof(IntLiteral));

    literal->error = error;
    literal->value = value;

    return literal;
}

IntLiteral* int_literal_parse(Parser* parser)
{
    b8 error = FRX_FALSE;

    u64 value = parser_current_token(parser)->int_literal;

    error = parser_eat(parser, FRX_TOKEN_TYPE_INT_LIT);

    return int_literal_create(error, value);
}

void int_literal_sema(IntLiteral* literal)
{
    FRX_ASSERT(literal != NULL);
}

void int_literal_codegen(IntLiteral* literal)
{
    FRX_ASSERT(literal != NULL);
    FRX_ASSERT(!literal->error);

    codegen_write("%zu", literal->value);
}
