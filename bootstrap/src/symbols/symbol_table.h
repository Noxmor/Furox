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

void init_global_symbol_table(void);

VariableTable* get_global_variable_table(void);

StructTable* get_global_struct_table(void);

FunctionTable* get_global_function_table(void);

#endif
