#include "mir.h"
#include "assert.h"
#include "compiler.h"

static MIRType* mir_type_create(MIRTypeKind kind)
{
    FRX_ASSERT(kind < FRX_MIR_TYPE_KIND_COUNT);

    MIRType* type = compiler_alloc_mir(sizeof(MIRType));

    type->kind = kind;

    return type;
}

MIRType* mir_type_create_primitive(MIRTypeKind kind)
{
    FRX_ASSERT(kind < FRX_MIR_TYPE_KIND_PTR);

    MIRType* type = mir_type_create(kind);

    return type;
}

MIRType* mir_type_create_ptr(MIRType* base)
{
    FRX_ASSERT(base != NULL);

    MIRType* type = mir_type_create(FRX_MIR_TYPE_KIND_PTR);

    type->ptr.base = base;

    return type;
}

MIRType* mir_type_create_array(MIRType* base, usize size)
{
    FRX_ASSERT(base != NULL);

    MIRType* type = mir_type_create(FRX_MIR_TYPE_KIND_ARRAY);

    type->array.base = base;
    type->array.size = size;

    return type;
}

static MIRVariable* mir_variable_create(MIRType* type, MIRVarID id, const char* name)
{
    FRX_ASSERT(type != NULL);

    MIRVariable* var = compiler_alloc_mir(sizeof(MIRVariable));

    var->type = type;
    var->id = id;
    var->name = name;

    return var;
}

MIRVariable* mir_variable_create_temp(MIRType* type, MIRVarID id)
{
    FRX_ASSERT(id != FRX_MIR_VAR_ID_NONE);

    return mir_variable_create(type, id, NULL);
}

MIRVariable* mir_variable_create_func_param(MIRType* type, const char* name)
{
    FRX_ASSERT(name != NULL);

    return mir_variable_create(type, FRX_MIR_VAR_ID_NONE, name);
}

MIRVariable* mir_variable_create_global(MIRType* type, const char* name)
{
    FRX_ASSERT(name != NULL);

    return mir_variable_create(type, FRX_MIR_VAR_ID_NONE, name);
}

MIRInstruction* mir_instruction_create(MIRInstructionType type, MIRVariable* dest,
                                       MIRVariable* left, MIRVariable* right)
{
    FRX_ASSERT(type < FRX_MIR_INSTRUCTION_TYPE_COUNT);

    FRX_ASSERT(dest != NULL);

    MIRInstruction* instruction = compiler_alloc_mir(sizeof(MIRInstruction));

    instruction->type = type;
    instruction->dest = dest;
    instruction->left = left;
    instruction->right = right;
    instruction->func_name = NULL;
    list_init(&instruction->func_args);

    return instruction;
}

MIRInstruction* mir_instruction_create_call(MIRVariable* dest, const char* func_name)
{
    MIRInstruction* instruction = mir_instruction_create(FRX_MIR_INSTRUCTION_TYPE_CALL,
                                                         dest, NULL, NULL);

    instruction->func_name = func_name;

    return instruction;
}

void mir_instruction_add_func_arg(MIRInstruction* instruction, MIRVariable* arg)
{
    FRX_ASSERT(instruction != NULL);

    FRX_ASSERT(instruction->type == FRX_MIR_INSTRUCTION_TYPE_CALL);

    FRX_ASSERT(arg != NULL);

    list_add(&instruction->func_args, arg);
}

MIRBlock* mir_block_create(const char* label)
{
    FRX_ASSERT(label != NULL);

    MIRBlock* block = compiler_alloc_mir(sizeof(MIRBlock));

    block->label = label;
    list_init(&block->instructions);
    block->next = NULL;

    return block;
}

void mir_block_add_instruction(MIRBlock* block, MIRInstruction* instruction)
{
    FRX_ASSERT(block != NULL);

    FRX_ASSERT(instruction != NULL);

    FRX_ASSERT(block->next == NULL);

    list_add(&block->instructions, instruction);
}

void mir_block_set_next(MIRBlock* block, MIRBlock* next)
{
    FRX_ASSERT(block != NULL);

    FRX_ASSERT(next != NULL);

    FRX_ASSERT(block->next == NULL);

    block->next = next;
}

MIRFuncContext* mir_func_context_create(const char* name, MIRType* return_type)
{
    FRX_ASSERT(name != NULL);

    FRX_ASSERT(return_type != NULL);

    MIRFuncContext* ctx = compiler_alloc_mir(sizeof(MIRFuncContext));

    ctx->return_type = return_type;
    ctx->name = name;
    list_init(&ctx->params);
    ctx->block = NULL;

    return ctx;
}

void mir_func_context_add_param(MIRFuncContext* ctx, MIRVariable* param)
{
    FRX_ASSERT(ctx != NULL);

    FRX_ASSERT(param != NULL);

    FRX_ASSERT(param->name != NULL);

    list_add(&ctx->params, param);
}

MIRContext* mir_context_create(void)
{
    MIRContext* ctx = compiler_alloc_mir(sizeof(MIRContext));

    list_init(&ctx->funcs);
    list_init(&ctx->globals);

    return ctx;
}

void mir_context_add_func(MIRContext* ctx, MIRFuncContext* func)
{
    FRX_ASSERT(ctx != NULL);

    FRX_ASSERT(func != NULL);

    list_add(&ctx->funcs, func);
}
