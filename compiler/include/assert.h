#ifndef FRX_ASSERT_H
#define FRX_ASSERT_H

#ifdef FRX_ENABLE_ASSERTS
#include <stdlib.h>
#include <stdio.h>
#define FRX_ASSERT(condition) do { if (!(condition)) fprintf(stderr, "%s:%d:Assertion failed: %s\n", __FILE__, __LINE__, #condition);\
    exit(EXIT_FAILURE); } while(0)
#else
#define FRX_ASSERT(condition)
#endif

#endif
