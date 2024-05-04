#include "symbol_table.h"

#include "core/assert.h"
#include "symbols/struct_table.h"
#include "symbols/variable_table.h"

void symbol_table_init(SymbolTable* table)
{
    FRX_ASSERT(table != NULL);

    variable_table_init(&table->variable_table);
    struct_table_init(&table->struct_table);
    function_table_init(&table->function_table);
}

void symbol_table_delete(SymbolTable* table)
{
    FRX_ASSERT(table != NULL);

    variable_table_delete(&table->variable_table);
    struct_table_delete(&table->struct_table);
    function_table_delete(&table->function_table);
}
