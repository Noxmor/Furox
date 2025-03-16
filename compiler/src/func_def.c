#include "assert.h"
#include "ast.h"
#include "codegen.h"
#include "compiler.h"
#include "mir.h"
#include "parser.h"
#include "resolution.h"
#include "sema.h"
#include "codegen.h"
#include "token.h"

#include <string.h>

static FuncDef* func_def_create(const char* name, GenericParams* generic_params,
                                FuncParams* params, TypeSpecifier* return_type,
                                Scope* body)
{
    FRX_ASSERT(name != NULL);

    FuncDef* func_def = compiler_alloc_ast(sizeof(FuncDef));

    func_def->name = name;
    func_def->generic_params = generic_params;
    list_init(&func_def->generic_instantiations);
    func_def->params = params;
    func_def->return_type = return_type;
    func_def->body = body;

    return func_def;
}

FuncDef* func_def_parse(Parser* parser)
{
    parser_eat(parser, FRX_TOKEN_TYPE_KW_FN);

    const char* name = parser_current_token(parser)->identifier;
    parser_eat(parser, FRX_TOKEN_TYPE_IDENT);

    GenericParams* generic_params = NULL;
    if (parser_match(parser, FRX_TOKEN_TYPE_LT))
    {
        generic_params = generic_params_parse(parser);
    }

    FuncParams* params = func_params_parse(parser);

    parser_eat(parser, FRX_TOKEN_TYPE_ARROW);

    TypeSpecifier* return_type = type_specifier_parse(parser);

    FuncDef* func_def = func_def_create(name, generic_params, params,
                                        return_type, scope_parse(parser));

    parser_insert_symbol(parser, parser->visibility, FRX_SYMBOL_TYPE_FUNC, func_def->name, func_def);

    return func_def;
}

void func_def_resolve(Parser* parser, FuncDef* func_def)
{
    FRX_ASSERT(func_def != NULL);

    if (func_def->params != NULL)
    {
        func_params_resolve(parser, func_def->params);
    }

    if (func_def->return_type != NULL)
    {
        type_specifier_resolve(parser, func_def->return_type);
    }

    if (func_def->body != NULL)
    {
        scope_resolve(parser, func_def->body);
    }
}

void func_def_sema(FuncDef* func_def)
{
    FRX_ASSERT(func_def != NULL);

    if (func_def->params != NULL)
    {
        func_params_sema(func_def->params);
    }

    if (func_def->return_type != NULL)
    {
        type_specifier_sema(func_def->return_type);
    }

    if (func_def->body != NULL)
    {
        scope_sema(func_def->body);
    }
}

void func_def_lower_to_mir(FuncDef* func_def, MIRContext* ctx)
{
    FRX_ASSERT(func_def != NULL);

    //TODO: Convert return type to MIR type
    MIRFuncContext* func = mir_func_context_create(func_def->name, mir_type_create_primitive(FRX_MIR_TYPE_KIND_I32));
    func->block = scope_lower_to_mir(func_def->body);

    if (func_def->return_type->kind == FRX_TYPE_KIND_PRIMITIVE
        && func_def->return_type->primitive == FRX_TOKEN_TYPE_KW_VOID)
    {
        //TODO: Return void
        MIRVariable* var = mir_variable_create_temp(mir_type_create_primitive(FRX_MIR_TYPE_KIND_VOID), 1);
        MIRInstruction* ret = mir_instruction_create(FRX_MIR_INSTRUCTION_TYPE_RET, var, NULL, NULL);
        mir_block_add_instruction(func->block, ret);
    }

    mir_context_add_func(ctx, func);
}
