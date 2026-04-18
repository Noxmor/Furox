#ifndef FRX_ATTRIBUTES_TABLE_H
#define FRX_ATTRIBUTES_TABLE_H

#include "ast.h"
#include "type_system.h"

void attributes_table_insert_type(ASTNodeID id, const Type* type);

const Type* attributes_table_lookup_type(ASTNodeID id);

#endif
