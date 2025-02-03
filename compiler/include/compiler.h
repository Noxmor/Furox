#ifndef FRX_COMPILER_H
#define FRX_COMPILER_H

#include "types.h"

int compiler_run(int argc, char** argv);

void* compiler_alloc(usize size);

#endif
