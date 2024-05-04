#ifndef FRX_SYMBOL_TABLE_H
#define FRX_SYMBOL_TABLE_H

#include "symbols/variable_table.h"
#include "symbols/struct_table.h"
#include "symbols/function_table.h"

typedef struct SymbolTable
{
    VariableTable variable_table;
    StructTable struct_table;
    FunctionTable function_table;
} SymbolTable;

void symbol_table_init(SymbolTable* table);

void symbol_table_delete(SymbolTable* table);

#endif
