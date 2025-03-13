#ifndef FRX_MIR_H
#define FRX_MIR_H

#include "list.h"

typedef u64 MIRVarID;

#define FRX_MIR_VAR_ID_NONE ((MIRVarID)0)

enum
{
    FRX_MIR_TYPE_KIND_VOID,
    FRX_MIR_TYPE_KIND_I8,
    FRX_MIR_TYPE_KIND_I16,
    FRX_MIR_TYPE_KIND_I32,
    FRX_MIR_TYPE_KIND_I64,
    FRX_MIR_TYPE_KIND_U8,
    FRX_MIR_TYPE_KIND_U16,
    FRX_MIR_TYPE_KIND_U32,
    FRX_MIR_TYPE_KIND_U64,
    FRX_MIR_TYPE_KIND_F32,
    FRX_MIR_TYPE_KIND_F64,
    FRX_MIR_TYPE_KIND_PTR,
    FRX_MIR_TYPE_KIND_ARRAY,

    FRX_MIR_TYPE_KIND_COUNT
};

typedef u8 MIRTypeKind;

typedef struct MIRType
{
    MIRTypeKind kind;

    union
    {
        struct
        {
            struct MIRType* base;
        } ptr;
        struct
        {
            struct MIRType* base;
            usize size;
        } array;
    };
} MIRType;

typedef struct MIRVariable
{
    MIRType* type;
    MIRVarID id;
    const char* name;
} MIRVariable;

enum
{
    FRX_MIR_INSTRUCTION_TYPE_ADD,
    FRX_MIR_INSTRUCTION_TYPE_SUB,
    FRX_MIR_INSTRUCTION_TYPE_MUL,
    FRX_MIR_INSTRUCTION_TYPE_DIV,
    FRX_MIR_INSTRUCTION_TYPE_MOD,
    FRX_MIR_INSTRUCTION_TYPE_RET,
    FRX_MIR_INSTRUCTION_TYPE_CALL,
    FRX_MIR_INSTRUCTION_TYPE_COUNT
};

typedef u8 MIRInstructionType;

typedef struct MIRInstruction
{
    MIRInstructionType type;
    MIRVariable* dest;
    MIRVariable* left;
    MIRVariable* right;
    const char* func_name;
    List func_args;
} MIRInstruction;

typedef struct MIRBlock
{
    List instructions;
    struct MIRBlock* next;
} MIRBlock;

typedef struct MIRFuncContext
{
    MIRType* return_type;
    const char* name;
    List params;
    MIRBlock* block;
} MIRFuncContext;

typedef struct MIRContext
{
    List funcs;
    List globals;
} MIRContext;

MIRType* mir_type_create_primitive(MIRTypeKind kind);

MIRType* mir_type_create_ptr(MIRType* base);

MIRType* mir_type_create_array(MIRType* base, usize size);

MIRVariable* mir_variable_create_temp(MIRType* type, MIRVarID id);

MIRVariable* mir_variable_create_func_param(MIRType* type, const char* name);

MIRVariable* mir_variable_create_global(MIRType* type, const char* name);

MIRInstruction* mir_instruction_create(MIRInstructionType type, MIRVariable* dest,
                                       MIRVariable* left, MIRVariable* right);

MIRInstruction* mir_instruction_create_call(MIRVariable* dest, const char* func_name);

void mir_instruction_add_func_arg(MIRInstruction* instruction, MIRVariable* arg);

MIRBlock* mir_block_create(void);

void mir_block_add_instruction(MIRBlock* block, MIRInstruction* instruction);

MIRFuncContext* mir_func_context_create(const char* name, MIRType* return_type);

void mir_func_context_add_param(MIRFuncContext* ctx, MIRVariable* param);

MIRContext* mir_context_create(void);

void mir_context_add_func(MIRContext* ctx, MIRFuncContext* func);

void mir_context_emit_c(const MIRContext* ctx);

#endif
