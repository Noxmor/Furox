#include "assert.h"
#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "codegen.h"

static Var* var_create(const char* name)
{
    Var* var = compiler_alloc(sizeof(Var));

    var->id = symbol_intern(name);
    var->name = name;

    return var;
}

Var* var_parse(Parser* parser)
{
    const char* name = parser_current_token(parser)->identifier;

    if (parser_eat(parser, FRX_TOKEN_TYPE_IDENT))
    {
        return NULL;
    }

    return var_create(name);
}

void var_resolve(Parser* parser, Var* var)
{
    FRX_ASSERT(var != NULL);

    var->symbol = parser_lookup_symbol(parser, var->id);
}

void var_sema(Var* var)
{
    FRX_ASSERT(var != NULL);
}

void var_codegen(Var* var)
{
    FRX_ASSERT(var != NULL);

    codegen_write("%s", var->name);
}
