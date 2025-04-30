#ifndef FRX_TYPE_INFERENCE_H
#define FRX_TYPE_INFERENCE_H

#include "ast.h"

TypeSpecifier* expr_infer_type(AST* expr);

#endif
