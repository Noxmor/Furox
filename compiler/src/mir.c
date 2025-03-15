#include "mir.h"
#include "assert.h"
#include "compiler.h"

#include <stdio.h>

static FILE* f;

#define FRX_MIR_EMIT(format, ...) fprintf(f, format, ##__VA_ARGS__)

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
    FRX_ASSERT(instruction->type == FRX_MIR_INSTRUCTION_TYPE_CALL);

    list_add(&instruction->func_args, arg);
}

MIRBlock* mir_block_create(void)
{
    MIRBlock* block = compiler_alloc_mir(sizeof(MIRBlock));

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

static void mir_type_emit_c(const MIRType* type)
{
    FRX_ASSERT(type != NULL);

    switch (type->kind)
    {
        case FRX_MIR_TYPE_KIND_VOID: FRX_MIR_EMIT("void"); break;
        case FRX_MIR_TYPE_KIND_I8: FRX_MIR_EMIT("i8"); break;
        case FRX_MIR_TYPE_KIND_I16: FRX_MIR_EMIT("i16"); break;
        case FRX_MIR_TYPE_KIND_I32: FRX_MIR_EMIT("i32"); break;
        case FRX_MIR_TYPE_KIND_I64: FRX_MIR_EMIT("i64"); break;
        case FRX_MIR_TYPE_KIND_U8: FRX_MIR_EMIT("i8"); break;
        case FRX_MIR_TYPE_KIND_U16: FRX_MIR_EMIT("i16"); break;
        case FRX_MIR_TYPE_KIND_U32: FRX_MIR_EMIT("i32"); break;
        case FRX_MIR_TYPE_KIND_U64: FRX_MIR_EMIT("i64"); break;
        case FRX_MIR_TYPE_KIND_F32: FRX_MIR_EMIT("f32"); break;
        case FRX_MIR_TYPE_KIND_F64: FRX_MIR_EMIT("f64"); break;
        case FRX_MIR_TYPE_KIND_PTR: mir_type_emit_c(type->ptr.base); FRX_MIR_EMIT("*"); break;
        default: FRX_ASSERT(FRX_FALSE); break;
    }
}

static void mir_variable_emit_c(const MIRVariable* variable)
{
    FRX_ASSERT(variable != NULL);

    if (variable->name != NULL)
    {
        FRX_MIR_EMIT("%s", variable->name);
    }
    else
    {
        FRX_MIR_EMIT("t%zu", variable->id);
    }
}

static void mir_instruction_emit_c(const MIRInstruction* instruction)
{
    FRX_ASSERT(instruction != NULL);

    switch (instruction->type)
    {
        case FRX_MIR_INSTRUCTION_TYPE_ADD:
        {
            mir_type_emit_c(instruction->dest->type);
            FRX_MIR_EMIT(" ");
            mir_variable_emit_c(instruction->dest);
            FRX_MIR_EMIT(" = ");
            mir_variable_emit_c(instruction->left);
            FRX_MIR_EMIT(" + ");
            mir_variable_emit_c(instruction->right);
            FRX_MIR_EMIT(";");
            break;
        }
        case FRX_MIR_INSTRUCTION_TYPE_SUB:
        {
            mir_type_emit_c(instruction->dest->type);
            FRX_MIR_EMIT(" ");
            mir_variable_emit_c(instruction->dest);
            FRX_MIR_EMIT(" = ");
            mir_variable_emit_c(instruction->left);
            FRX_MIR_EMIT(" - ");
            mir_variable_emit_c(instruction->right);
            FRX_MIR_EMIT(";");
            break;
        }
        case FRX_MIR_INSTRUCTION_TYPE_MUL:
        {
            mir_type_emit_c(instruction->dest->type);
            FRX_MIR_EMIT(" ");
            mir_variable_emit_c(instruction->dest);
            FRX_MIR_EMIT(" = ");
            mir_variable_emit_c(instruction->left);
            FRX_MIR_EMIT(" * ");
            mir_variable_emit_c(instruction->right);
            FRX_MIR_EMIT(";");
            break;
        }
        case FRX_MIR_INSTRUCTION_TYPE_DIV:
        {
            mir_type_emit_c(instruction->dest->type);
            FRX_MIR_EMIT(" ");
            mir_variable_emit_c(instruction->dest);
            FRX_MIR_EMIT(" = ");
            mir_variable_emit_c(instruction->left);
            FRX_MIR_EMIT(" / ");
            mir_variable_emit_c(instruction->right);
            FRX_MIR_EMIT(";");
            break;
        }
        case FRX_MIR_INSTRUCTION_TYPE_MOD:
        {
            mir_type_emit_c(instruction->dest->type);
            FRX_MIR_EMIT(" ");
            mir_variable_emit_c(instruction->dest);
            FRX_MIR_EMIT(" = ");
            mir_variable_emit_c(instruction->left);
            FRX_MIR_EMIT(" %%");
            mir_variable_emit_c(instruction->right);
            FRX_MIR_EMIT(";");
            break;
        }
        case FRX_MIR_INSTRUCTION_TYPE_RET:
        {
            if (instruction->dest == NULL)
            {
                FRX_MIR_EMIT("return;");
            }
            else
            {
                FRX_MIR_EMIT("return ");
                mir_variable_emit_c(instruction->dest);
                FRX_MIR_EMIT(";");
            }

            break;
        }
        case FRX_MIR_INSTRUCTION_TYPE_CALL:
        {
            mir_type_emit_c(instruction->dest->type);
            FRX_MIR_EMIT(" ");
            mir_variable_emit_c(instruction->dest);
            FRX_MIR_EMIT(" = %s(", instruction->func_name);

            for (usize i = 0; i < list_size(&instruction->func_args); ++i)
            {
                MIRVariable* arg = list_get(&instruction->func_args, i);
                mir_variable_emit_c(arg);

                if (i + 1 < list_size(&instruction->func_args))
                {
                    FRX_MIR_EMIT(", ");
                }
            }

            FRX_MIR_EMIT(");");

            break;
        }
    }

    FRX_MIR_EMIT("\n");
}

static void mir_block_emit_c(const MIRBlock* block)
{
    FRX_ASSERT(block != NULL);

    for (usize i = 0; i < list_size(&block->instructions); ++i)
    {
        MIRInstruction* instruction = list_get(&block->instructions, i);
        FRX_MIR_EMIT("    ");
        mir_instruction_emit_c(instruction);
    }

    if (block->next != NULL)
    {
        mir_block_emit_c(block->next);
    }
}

static void mir_func_context_emit_c(const MIRFuncContext* ctx)
{
    FRX_ASSERT(ctx != NULL);

    //TODO: Implement
    mir_type_emit_c(ctx->return_type);
    FRX_MIR_EMIT(" %s(", ctx->name);

    for (usize i = 0; i < list_size(&ctx->params); ++i)
    {
        MIRVariable* param = list_get(&ctx->params, i);
        mir_type_emit_c(param->type);
        FRX_MIR_EMIT(" %s", param->name);

        if (i + 1 < list_size(&ctx->params))
        {
            FRX_MIR_EMIT(", ");
        }
    }

    FRX_MIR_EMIT(")\n{\n");

    mir_block_emit_c(ctx->block);

    FRX_MIR_EMIT("}\n");
}

void mir_context_emit_c(const MIRContext* ctx)
{
    FRX_ASSERT(ctx != NULL);

    f = fopen("frx.c", "w");
    if (f == NULL)
    {
        return;
    }

    FRX_MIR_EMIT("#include <stdint.h>\n\n");
    FRX_MIR_EMIT("typedef int8_t i8;\n");
    FRX_MIR_EMIT("typedef int16_t i16;\n");
    FRX_MIR_EMIT("typedef int32_t i32;\n");
    FRX_MIR_EMIT("typedef int64_t i64;\n");
    FRX_MIR_EMIT("typedef uint8_t u8;\n");
    FRX_MIR_EMIT("typedef uint16_t u16;\n");
    FRX_MIR_EMIT("typedef uint32_t u32;\n");
    FRX_MIR_EMIT("typedef uint64_t u64;\n");
    FRX_MIR_EMIT("\n");

    for (usize i = 0; i < list_size(&ctx->funcs); ++i)
    {
        MIRFuncContext* func = list_get(&ctx->funcs, i);
        mir_func_context_emit_c(func);

        if (i + i < list_size(&ctx->funcs))
        {
            FRX_MIR_EMIT("\n");
        }
    }

    fclose(f);
}
