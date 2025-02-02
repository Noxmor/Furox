#ifndef FRX_ASSERT_H
#define FRX_ASSERT_H

#ifdef FRX_ENABLE_ASSERTS
#include <stdlib.h>
#include "log.h"
#define FRX_ASSERT(condition) do { if (!(condition)) { FRX_LOG_ERROR("Assertion failed: %s:%d: %s", __FILE__, __LINE__, #condition);\
    exit(EXIT_FAILURE); } } while(0)
#else
#define FRX_ASSERT(condition)
#endif

#endif
