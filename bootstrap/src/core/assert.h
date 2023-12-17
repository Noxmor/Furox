#ifndef FRX_ASSERT_H
#define FRX_ASSERT_H

#include <stdio.h>
#include <stdlib.h>

#ifdef FRX_ENABLE_ASSERTS
#define FRX_ASSERT(condition) if(!(condition)) { fprintf(stderr, "Assertion failed at %s:%zu: \"%s\"\n", __FILE__, __LINE__, #condition); exit(EXIT_FAILURE); }
#else
#define FRX_ASSERT(condition)
#endif

#endif
