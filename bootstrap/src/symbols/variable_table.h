#ifndef FRX_VARIABLE_TABLE_H
#define FRX_VARIABLE_TABLE_H

#include "symbols/struct_table.h"

#include "token.h"
#include "namespace.h"

#define FRX_VARIABLE_TABLE_CAPACITY 1024

enum
{
    FRX_VARIABLE_TYPE_PRIMITIVE,
    FRX_VARIABLE_TYPE_STRUCT,
    FRX_VARIABLE_TYPE_ENUM,

    FRX_VARIABLE_TYPE_COUNT
};

typedef u8 VariableType;

typedef struct VariableSymbol
{
    VariableType type;

    char name[FRX_TOKEN_IDENTIFIER_CAPACITY];

    //Only for global variables
    Namespace* namespace;

    union
    {
        TokenType primitive_type;
        const StructSymbol* struct_symbol;
        //const EnumSymbol* enum_symbol;
    };
} VariableSymbol;

typedef struct VariableTableEntry
{
    VariableSymbol symbol;

    struct VariableTableEntry* next;
} VariableTableEntry;

typedef struct VariableTable
{
    VariableTableEntry* entries[FRX_VARIABLE_TABLE_CAPACITY];
} VariableTable;

void variable_table_init(VariableTable* table);

VariableSymbol* variable_table_insert(VariableTable* table, const char* name,
        const Namespace* namespace);

VariableSymbol* variable_table_find(const VariableTable* table,
        const char* name, const Namespace* namespace);

void variable_table_delete(VariableTable* table);

#endif
