#ifndef FRX_TYPE_INFERENCE_H
#define FRX_TYPE_INFERENCE_H

#include "ast.h"
#include "hir.h"

Type* expr_infer_type(const AST* expr);

#endif
