#ifndef FRX_THREAD_POOL_H
#define FRX_THREAD_POOL_H

#include "types.h"

typedef struct Task Task;

typedef void* (*TaskFunc)(void*);

void thread_pool_init(usize max_threads);

Task* thread_pool_exec(TaskFunc func, void* arg);

void* thread_pool_join(Task* task);

void thread_pool_shutdown(void);

#endif
