/**
 * ============================================================================
 * BASIL & EMBER - RESTAURANT ORDER MANAGEMENT SYSTEM
 * File: queue.h
 * Description: Interface for First-In, First-Out (FIFO) Order Queue in C.
 * ============================================================================
 */

#ifndef QUEUE_H
#define QUEUE_H

#include "order.h"

/**
 * FIFO ORDER QUEUE
 * - front: points to the earliest unserved order.
 * - rear: points to the most recent order.
 * - count: total active orders waiting in the queue.
 */
typedef struct {
    Order *front;
    Order *rear;
    int count;
} OrderQueue;

/* Queue Operations */
void initQueue(OrderQueue *q);
int isEmpty(const OrderQueue *q);
int enqueue(OrderQueue *q, Order *newOrder);
Order* dequeue(OrderQueue *q);
Order* peek(const OrderQueue *q);
int getQueueSize(const OrderQueue *q);
Order* findOrderInQueue(const OrderQueue *q, int orderId);
int removeOrderFromQueue(OrderQueue *q, int orderId);
char* queueToJson(const OrderQueue *q);
void freeQueue(OrderQueue *q);

#endif /* QUEUE_H */
