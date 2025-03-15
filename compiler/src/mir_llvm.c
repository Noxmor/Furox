#include "assert.h"
#include "mir.h"

#include <stdio.h>

static FILE* f;

#define MIR_EMIT(format, ...) fprintf(f, format, ##__VA_ARGS__)

static void mir_emit_type(const MIRType* type)
{
    FRX_ASSERT(type != NULL);

    FRX_ASSERT(type->kind < FRX_MIR_TYPE_KIND_COUNT);

    switch (type->kind)
    {
        case FRX_MIR_TYPE_KIND_VOID: MIR_EMIT("void"); break;
        case FRX_MIR_TYPE_KIND_I8: MIR_EMIT("i8"); break;
        case FRX_MIR_TYPE_KIND_I16: MIR_EMIT("i16"); break;
        case FRX_MIR_TYPE_KIND_I32: MIR_EMIT("i32"); break;
        case FRX_MIR_TYPE_KIND_I64: MIR_EMIT("i64"); break;
        case FRX_MIR_TYPE_KIND_U8: MIR_EMIT("i8"); break;
        case FRX_MIR_TYPE_KIND_U16: MIR_EMIT("i16"); break;
        case FRX_MIR_TYPE_KIND_U32: MIR_EMIT("i32"); break;
        case FRX_MIR_TYPE_KIND_U64: MIR_EMIT("i64"); break;
        case FRX_MIR_TYPE_KIND_F32: MIR_EMIT("f32"); break;
        case FRX_MIR_TYPE_KIND_F64: MIR_EMIT("f64"); break;
        case FRX_MIR_TYPE_KIND_PTR: mir_emit_type(type->ptr.base); MIR_EMIT("*"); break;
        case FRX_MIR_TYPE_KIND_ARRAY: MIR_EMIT("[%zu x ", type->array.size); mir_emit_type(type->array.base); MIR_EMIT("]"); break;
        default: FRX_ASSERT(FRX_FALSE); break;
    }
}

static void mir_emit_variable(const MIRVariable* variable)
{
    FRX_ASSERT(variable != NULL);

    MIR_EMIT("%%");
    if (variable->id != FRX_MIR_VAR_ID_NONE)
    {
        MIR_EMIT("t%zu", variable->id);
    }
    else
    {
        MIR_EMIT("%s", variable->name);
    }
}

static void mir_emit_instruction(const MIRInstruction* instruction)
{
    FRX_ASSERT(instruction != NULL);

    switch (instruction->type)
    {
        case FRX_MIR_INSTRUCTION_TYPE_ADD:
        {
            mir_emit_variable(instruction->dest);
            MIR_EMIT(" = add i32 ");
            mir_emit_variable(instruction->left);
            MIR_EMIT(", ");
            mir_emit_variable(instruction->right);

            break;
        }
        case FRX_MIR_INSTRUCTION_TYPE_SUB:
        {
            mir_emit_variable(instruction->dest);
            MIR_EMIT(" = sub i32 ");
            mir_emit_variable(instruction->left);
            MIR_EMIT(", ");
            mir_emit_variable(instruction->right);

            break;
        }
        case FRX_MIR_INSTRUCTION_TYPE_MUL:
        {
            mir_emit_variable(instruction->dest);
            MIR_EMIT(" = mul i32 ");
            mir_emit_variable(instruction->left);
            MIR_EMIT(", ");
            mir_emit_variable(instruction->right);

            break;
        }
        case FRX_MIR_INSTRUCTION_TYPE_DIV:
        {
            mir_emit_variable(instruction->dest);
            MIR_EMIT(" = sdiv i32 ");
            mir_emit_variable(instruction->left);
            MIR_EMIT(", ");
            mir_emit_variable(instruction->right);

            break;
        }
        case FRX_MIR_INSTRUCTION_TYPE_MOD:
        {
            mir_emit_variable(instruction->dest);
            MIR_EMIT(" = srem i32 ");
            mir_emit_variable(instruction->left);
            MIR_EMIT(", ");
            mir_emit_variable(instruction->right);

            break;
        }
        case FRX_MIR_INSTRUCTION_TYPE_RET:
        {
            MIR_EMIT("ret ");
            if (instruction->dest != NULL)
            {
                mir_emit_type(instruction->dest->type);
                MIR_EMIT(" ");
                mir_emit_variable(instruction->dest);
            }
            else
            {
                MIR_EMIT(" void");
            }

            break;
        }
        case FRX_MIR_INSTRUCTION_TYPE_CALL:
        {
            mir_emit_variable(instruction->dest);
            MIR_EMIT(" = call i32 @%s(", instruction->func_name);

            for (usize i = 0; i < list_size(&instruction->func_args); ++i)
            {
                MIRVariable* arg = list_get(&instruction->func_args, i);
                mir_emit_type(arg->type);
                MIR_EMIT(" ");
                mir_emit_variable(arg);

                if (i + 1 < list_size(&instruction->func_args))
                {
                    MIR_EMIT(", ");
                }
            }

            MIR_EMIT(")");

            break;
        }
        default: FRX_ASSERT(FRX_FALSE);
    }

    MIR_EMIT("\n");
}

static void mir_emit_block(const MIRBlock* block)
{
    FRX_ASSERT(block != NULL);

    MIR_EMIT("%s:\n", block->label);

    for (usize i = 0; i < list_size(&block->instructions); ++i)
    {
        MIRInstruction* instruction = list_get(&block->instructions, i);
        mir_emit_instruction(instruction);
    }
}

static void mir_emit_func_decl(const MIRFuncContext* ctx)
{
    FRX_ASSERT(ctx != NULL);

    MIR_EMIT("declare ");
    mir_emit_type(ctx->return_type);
    MIR_EMIT(" @%s(", ctx->name);

    for (usize i = 0; i < list_size(&ctx->params); ++i)
    {
        MIRVariable* param = list_get(&ctx->params, i);
        mir_emit_type(param->type);

        if (i + 1 < list_size(&ctx->params))
        {
            MIR_EMIT(", ");
        }
    }

    MIR_EMIT(")\n");
}

static void mir_emit_func_def(const MIRFuncContext* ctx)
{
    FRX_ASSERT(ctx != NULL);

    MIR_EMIT("define ");
    mir_emit_type(ctx->return_type);
    MIR_EMIT(" @%s(", ctx->name);

    for (usize i = 0; i < list_size(&ctx->params); ++i)
    {
        MIRVariable* param = list_get(&ctx->params, i);
        mir_emit_type(param->type);
        MIR_EMIT(" %%%s", param->name);

        if (i + 1 < list_size(&ctx->params))
        {
            MIR_EMIT(", ");
        }
    }

    MIR_EMIT(") {\n");

    mir_emit_block(ctx->block);

    MIR_EMIT("}\n");
}

void mir_context_emit_llvm(const MIRContext* ctx)
{
    FRX_ASSERT(ctx != NULL);

    f = fopen("frx.ll", "w");
    if (f == NULL)
    {
        return;
    }

    for (usize i = 0; i < list_size(&ctx->funcs); ++i)
    {
        MIRFuncContext* func = list_get(&ctx->funcs, i);
        //TODO: Do we need tis?
        //mir_emit_func_decl(func);
        MIR_EMIT("\n");
    }

    for (usize i = 0; i < list_size(&ctx->funcs); ++i)
    {
        MIRFuncContext* func = list_get(&ctx->funcs, i);
        mir_emit_func_def(func);
        MIR_EMIT("\n");
    }

    fclose(f);
}
