#ifndef FRX_VARIABLE_TABLE_H
#define FRX_VARIABLE_TABLE_H

#include "token.h"
#include "namespace.h"

#include "symbols/type_category.h"
#include "symbols/struct_table.h"

#define FRX_VARIABLE_TABLE_CAPACITY 1024

typedef u8 VariableType;

typedef struct VariableSymbol
{
    TypeCategory type_category;

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
