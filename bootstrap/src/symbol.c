#include "assert.h"
#include "compiler.h"
#include "symbol.h"

Symbol* symbol_create(const char* name, SymbolVisibility visibility,
                      SymbolType type, void* data)
{
    FRX_ASSERT(name != NULL || type == FRX_SYMBOL_TYPE_PRIMITIVE);

    FRX_ASSERT(visibility < FRX_SYMBOL_VISIBILITY_COUNT);

    FRX_ASSERT(type < FRX_SYMBOL_TYPE_COUNT);

    Symbol* symbol = compiler_alloc(sizeof(Symbol));

    symbol->name = name;
    symbol->visibility = visibility;
    symbol->type = type;
    symbol->data = data;
    symbol->associated_type = NULL;

    return symbol;
}
