#ifndef FRX_FUNCTION_TABLE_H
#define FRX_FUNCTION_TABLE_H

#include "symbols/type_category.h"
#include "symbols/struct_table.h"

#define FRX_FUNCTION_TABLE_CAPACITY 1024

typedef struct FunctionSymbol
{
    TypeCategory type_category;

    char name[FRX_TOKEN_IDENTIFIER_CAPACITY];

    List parameters;

    Namespace* namespace;

    b8 defined;

    union
    {
        TokenType primitive_return_type;
        StructSymbol* struct_symbol_return_type;
        //EnumSymbol* enum_symbol_return_type;
    };
} FunctionSymbol;

typedef struct FunctionTableEntry
{
    FunctionSymbol symbol;

    struct FunctionTableEntry* next;
} FunctionTableEntry;

typedef struct FunctionTable
{
    FunctionTableEntry* entries[FRX_FUNCTION_TABLE_CAPACITY];
} FunctionTable;

void function_table_init(FunctionTable* table);

FunctionSymbol* function_table_insert(FunctionTable* table, const char* name,
        const Namespace* namespace);

FunctionSymbol* function_table_find(const FunctionTable* table,
        const char* name,
        const Namespace* namespace);

FunctionSymbol* function_table_find_or_insert(FunctionTable* table,
        const char* name,
        const Namespace* namespace);

void function_table_delete(FunctionTable* table);

#endif
