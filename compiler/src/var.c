#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "codegen.h"

static Var* var_create(b8 error, const char* name)
{
    Var* var = compiler_alloc(sizeof(Var));

    var->error = error;
    var->name = name;

    return var;
}

Var* var_parse(Parser* parser)
{
    b8 error = FRX_FALSE;

    const char* name = parser_current_token(parser)->identifier;

    error |= parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    return var_create(error, name);
}

void var_resolve(Parser* parser, Var* var)
{
    FRX_ASSERT(var != NULL);

    if (var->error)
    {
        return;
    }
}

void var_sema(Var* var)
{
    FRX_ASSERT(var != NULL);

    if (var->error)
    {
        return;
    }
}

void var_codegen(Var* var)
{
    FRX_ASSERT(var != NULL);
    FRX_ASSERT(!var->error);

    codegen_write("%s", var->name);
}
