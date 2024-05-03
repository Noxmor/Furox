#ifndef FRX_SYMBOL_TABLE_H
#define FRX_SYMBOL_TABLE_H

#include "symbols/variable_table.h"
#include "symbols/struct_table.h"

typedef struct SymbolTable
{
    VariableTable variable_table;
    StructTable struct_table;
} SymbolTable;

void symbol_table_init(SymbolTable* table);

void symbol_table_delete(SymbolTable* table);

#endif
