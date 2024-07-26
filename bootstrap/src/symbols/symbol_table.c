#include "symbol_table.h"

#include "symbols/struct_table.h"
#include "symbols/variable_table.h"

static SymbolTable symbol_table;

void init_global_symbol_table(void)
{
    variable_table_init(&symbol_table.variable_table);
    struct_table_init(&symbol_table.struct_table);
    function_table_init(&symbol_table.function_table);
}

VariableTable* get_global_variable_table(void)
{
    return &symbol_table.variable_table;
}

StructTable* get_global_struct_table(void)
{
    return &symbol_table.struct_table;
}

FunctionTable* get_global_function_table(void)
{
    return &symbol_table.function_table;
}
