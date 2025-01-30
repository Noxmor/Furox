#include "queue.h"

#include <stdlib.h>

#include "assert.h"

void queue_init(Queue* queue)
{
    FRX_ASSERT(queue != NULL);

    queue->front = NULL;
    queue->rear = NULL;
}

b8 queue_is_empty(const Queue* queue)
{
    FRX_ASSERT(queue != NULL);

    return queue->front == NULL;
}

b8 queue_contains(const Queue* queue, void* data)
{
    FRX_ASSERT(queue != NULL);

    QueueNode* node = queue->front;
    while (node != NULL)
    {
        if (node->data == data)
        {
            return FRX_TRUE;
        }

        node = node->next;
    }

    return FRX_FALSE;
}

void queue_enqueue(Queue* queue, void* data)
{
    FRX_ASSERT(queue != NULL);

    QueueNode* node = malloc(sizeof(QueueNode));
    node->data = data;
    node->next = NULL;

    if (queue_is_empty(queue))
    {
        queue->front = node;
        queue->rear = node;
    }
    else
    {
        queue->rear->next = node;
        queue->rear = node;
    }
}

void* queue_dequeue(Queue* queue)
{
    FRX_ASSERT(queue != NULL);

    if(queue_is_empty(queue))
    {
        return NULL;
    }

    QueueNode* node = queue->front;
    void* data = node->data;

    queue->front = queue->front->next;

    free(node);

    return data;
}
