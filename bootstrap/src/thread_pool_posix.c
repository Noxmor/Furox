#include "platform_detection.h"
#ifdef FRX_PLATFORM_POSIX
#include "thread_pool.h"

#include <stdlib.h>
#include <pthread.h>

#include "assert.h"
#include "log.h"
#include "queue.h"

typedef struct Task
{
    void* ret;
    TaskFunc func;
    void* arg;
    b8 done;
    pthread_mutex_t mutex;
    pthread_cond_t condition;
} Task;

typedef struct ThreadPool
{
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    pthread_t* threads;
    usize threads_count;
    Queue tasks;
    b8 should_terminate;
} ThreadPool;

static ThreadPool pool;

void* worker_thread(void* arg)
{
    (void)arg;

    while (!pool.should_terminate)
    {
        pthread_mutex_lock(&pool.mutex);

        Task* task = NULL;

        if (!queue_is_empty(&pool.tasks))
        {
            task = queue_dequeue(&pool.tasks);
        }
        else
        {
            pthread_cond_wait(&pool.condition, &pool.mutex);
        }

        pthread_mutex_unlock(&pool.mutex);

        if (task != NULL)
        {
            task->ret = task->func(task->arg);

            pthread_mutex_lock(&task->mutex);
            task->done = FRX_TRUE;
            pthread_mutex_unlock(&task->mutex);

            pthread_cond_signal(&task->condition);
        }
    }

    return NULL;
}

void thread_pool_init(usize max_threads)
{
    FRX_ASSERT(max_threads > 0);

    FRX_LOG_INFO("Initializing thread pool...");

    pthread_mutex_init(&pool.mutex, NULL);
    pthread_cond_init(&pool.condition, NULL);
    queue_init(&pool.tasks);

    pool.threads_count = max_threads;
    pool.threads = malloc(sizeof(pthread_t) * pool.threads_count);

    for(usize i = 0; i < pool.threads_count; ++i)
    {
        pthread_create(&pool.threads[i], NULL, worker_thread, NULL);
    }
}

Task* thread_pool_exec(TaskFunc func, void* arg)
{
    FRX_ASSERT(func != NULL);

    Task* task = malloc(sizeof(Task));

    task->ret = NULL;
    task->func = func;
    task->arg = arg;
    task->done = FRX_FALSE;
    pthread_mutex_init(&task->mutex, NULL);
    pthread_cond_init(&task->condition, NULL);

    pthread_mutex_lock(&pool.mutex);
    queue_enqueue(&pool.tasks, task);
    pthread_mutex_unlock(&pool.mutex);

    pthread_cond_signal(&pool.condition);

    return task;
}

void* thread_pool_join(Task* task)
{
    FRX_ASSERT(task != NULL);

    pthread_mutex_lock(&task->mutex);

    if (!task->done)
    {
        pthread_cond_wait(&task->condition, &task->mutex);
    }

    pthread_mutex_unlock(&task->mutex);

    void* ret = task->ret;

    pthread_cond_destroy(&task->condition);
    pthread_mutex_destroy(&task->mutex);
    free(task);

    return ret;
}

void thread_pool_shutdown(void)
{
    FRX_LOG_INFO("Shutting down thread pool...");

    pool.should_terminate = FRX_TRUE;
    pthread_cond_broadcast(&pool.condition);

    for(usize i = 0; i < pool.threads_count; ++i)
    {
        pthread_join(pool.threads[i], NULL);
    }

    FRX_ASSERT(queue_is_empty(&pool.tasks));

    pthread_cond_destroy(&pool.condition);
    pthread_mutex_destroy(&pool.mutex);

    free(pool.threads);
}
#endif
