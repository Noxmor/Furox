#ifndef FRX_QUEUE_H
#define FRX_QUEUE_H

#include "types.h"

typedef struct QueueNode
{
    void* data;
    struct QueueNode* next;
} QueueNode;

typedef struct Queue
{
    QueueNode* front;
    QueueNode* rear;
} Queue;

void queue_init(Queue* queue);

b8 queue_is_empty(const Queue* queue);

b8 queue_contains(const Queue* queue, void* data);

void queue_enqueue(Queue* queue, void* data);

void* queue_dequeue(Queue* queue);

#endif
