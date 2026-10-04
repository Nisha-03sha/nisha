/**
 * ============================================================================
 * RESTAURANT ORDER MANAGEMENT SYSTEM - DATA STRUCTURES MINI PROJECT
 * File: queue.h
 * Description: Interface for Queue Data Structure operations.
 * Demonstrates: First-In, First-Out (FIFO) queue implemented with pointers.
 * Core Operations: enqueue(), dequeue(), peek(), isEmpty(), displayQueue().
 * ============================================================================
 */

#ifndef QUEUE_H
#define QUEUE_H

#include "order.h"

/**
 * QUEUE DATA STRUCTURE (FIFO)
 * Maintains two pointers:
 * - front: points to the first order to be prepared/served.
 * - rear: points to the most recently enqueued order.
 * - count: tracks current number of waiting orders in the queue.
 */
typedef struct {
    Order *front;   /* Points to the oldest order (next to be processed) */
    Order *rear;    /* Points to the newest order */
    int count;      /* Current length of the queue */
} OrderQueue;

/* Queue Operations */

/**
 * Initializes the queue pointers to NULL and count to 0.
 */
void initQueue(OrderQueue *q);

/**
 * Checks whether the queue has zero orders.
 * Returns 1 (true) if empty, 0 (false) otherwise.
 */
int isEmpty(const OrderQueue *q);

/**
 * Inserts a new Order at the REAR of the queue (FIFO principle).
 * Updates rear pointer, and front pointer if queue was previously empty.
 * Time Complexity: O(1)
 */
int enqueue(OrderQueue *q, Order *newOrder);

/**
 * Removes and returns the Order at the FRONT of the queue.
 * Updates front pointer to the next order in line.
 * Time Complexity: O(1)
 */
Order* dequeue(OrderQueue *q);

/**
 * Inspects the order at the FRONT of the queue without removing it.
 * Returns pointer to front order, or NULL if queue is empty.
 * Time Complexity: O(1)
 */
Order* peek(const OrderQueue *q);

/**
 * Returns current count of orders in the queue.
 */
int getQueueSize(const OrderQueue *q);

/**
 * Searches the queue sequentially to find an order by its ID.
 * Time Complexity: O(N)
 */
Order* findOrderInQueue(const OrderQueue *q, int orderId);

/**
 * Cancels/removes a specific order from the queue if needed.
 * Adjusts front, rear, and next pointers.
 * Returns 1 if found and removed, 0 otherwise.
 */
int removeOrderFromQueue(OrderQueue *q, int orderId);

/**
 * Displays the current queue state in terminal format:
 * FRONT -> [Order 101] -> [Order 102] -> [Order 103] -> REAR
 */
void displayQueue(const OrderQueue *q);

/**
 * Serializes all orders currently in the queue into a JSON array string.
 */
char* queueToJson(const OrderQueue *q);

/**
 * Serializes the complete Queue state including FRONT, REAR pointer addresses,
 * individual node addresses, next pointers, and each order's internal item
 * linked lists for interactive frontend visualization.
 */
char* queueVisualizationToJson(const OrderQueue *q);

/**
 * Frees all orders currently in the queue.
 */
void freeQueue(OrderQueue *q);

#endif /* QUEUE_H */
