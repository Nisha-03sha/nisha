/**
 * ============================================================================
 * RESTAURANT ORDER MANAGEMENT SYSTEM - DATA STRUCTURES MINI PROJECT
 * File: queue.c
 * Description: Implementation of FIFO (First-In, First-Out) Order Queue.
 * 
 * Data Structure Concept:
 * A Queue is a linear data structure following the FIFO rule:
 * - Elements are inserted at the REAR (Enqueue operation).
 * - Elements are removed from the FRONT (Dequeue operation).
 * 
 * In this restaurant application:
 * When a customer places an order, it is enqueued at the REAR.
 * When the kitchen staff clicks "Process Next", the order at the FRONT
 * is dequeued for serving.
 * ============================================================================
 */

#include "queue.h"
#include "linkedlist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Initializes the Queue to an empty state.
 * Both FRONT and REAR pointers point to NULL.
 */
void initQueue(OrderQueue *q) {
    if (q == NULL) return;
    q->front = NULL;
    q->rear = NULL;
    q->count = 0;
    printf("[QUEUE] Initialized empty FIFO Queue (front: NULL, rear: NULL, count: 0)\n");
}

/**
 * Checks whether the queue is currently empty.
 * Returns 1 if empty, 0 otherwise.
 */
int isEmpty(const OrderQueue *q) {
    if (q == NULL) return 1;
    return (q->front == NULL);
}

/**
 * Inserts an Order at the REAR of the queue.
 * FIFO Property: Every new order waits behind earlier orders.
 * Time Complexity: O(1)
 */
int enqueue(OrderQueue *q, Order *newOrder) {
    if (q == NULL || newOrder == NULL) {
        fprintf(stderr, "[QUEUE ERROR] Invalid queue or order pointer for enqueue!\n");
        return 0;
    }

    newOrder->next = NULL;

    /* If queue is currently empty */
    if (q->front == NULL) {
        q->front = newOrder;
        q->rear = newOrder;
        printf("[QUEUE OPERATION: ENQUEUE] First order #%d inserted. FRONT: %p, REAR: %p\n",
               newOrder->orderId, (void *)q->front, (void *)q->rear);
    } else {
        /* Link current rear's next to the new order */
        q->rear->next = newOrder;
        /* Advance rear pointer to the new order */
        q->rear = newOrder;
        printf("[QUEUE OPERATION: ENQUEUE] Order #%d appended at REAR (%p). FRONT remains at Order #%d (%p)\n",
               newOrder->orderId, (void *)q->rear, q->front->orderId, (void *)q->front);
    }

    q->count++;
    printf("[QUEUE STATUS] Current Queue Size: %d\n", q->count);
    return 1;
}

/**
 * Removes and returns the Order currently at the FRONT of the queue.
 * FIFO Property: Oldest pending order is served first.
 * Time Complexity: O(1)
 */
Order* dequeue(OrderQueue *q) {
    if (q == NULL || isEmpty(q)) {
        printf("[QUEUE OPERATION: DEQUEUE] Attempted to dequeue from an EMPTY queue!\n");
        return NULL;
    }

    /* Store the current front order */
    Order *dequeuedOrder = q->front;

    /* Advance the front pointer to the next order in line */
    q->front = q->front->next;

    /* If queue becomes empty after this dequeue, set rear to NULL as well */
    if (q->front == NULL) {
        q->rear = NULL;
    }

    /* Disconnect the dequeued order's next pointer */
    dequeuedOrder->next = NULL;
    q->count--;

    printf("[QUEUE OPERATION: DEQUEUE] Served Order #%d from FRONT! New FRONT: %p, REAR: %p, Size: %d\n",
           dequeuedOrder->orderId, (void *)q->front, (void *)q->rear, q->count);

    return dequeuedOrder;
}

/**
 * Returns a pointer to the order at FRONT without modifying the queue.
 * Time Complexity: O(1)
 */
Order* peek(const OrderQueue *q) {
    if (q == NULL || isEmpty(q)) {
        return NULL;
    }
    return q->front;
}

/**
 * Returns current count of orders in queue.
 */
int getQueueSize(const OrderQueue *q) {
    if (q == NULL) return 0;
    return q->count;
}

/**
 * Searches sequentially for an order by its ID.
 * Time Complexity: O(N)
 */
Order* findOrderInQueue(const OrderQueue *q, int orderId) {
    if (q == NULL) return NULL;

    Order *curr = q->front;
    while (curr != NULL) {
        if (curr->orderId == orderId) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

/**
 * Cancels and removes an order from anywhere inside the queue.
 * Handles front, middle, and rear node removals.
 */
int removeOrderFromQueue(OrderQueue *q, int orderId) {
    if (q == NULL || isEmpty(q)) return 0;

    Order *curr = q->front;
    Order *prev = NULL;

    while (curr != NULL && curr->orderId != orderId) {
        prev = curr;
        curr = curr->next;
    }

    if (curr == NULL) return 0; /* Not found */

    if (prev == NULL) {
        /* Target is FRONT node */
        q->front = curr->next;
        if (q->front == NULL) {
            q->rear = NULL;
        }
    } else {
        /* Target is internal or REAR node */
        prev->next = curr->next;
        if (curr == q->rear) {
            q->rear = prev;
        }
    }

    curr->next = NULL;
    q->count--;
    printf("[QUEUE] Removed Order #%d from queue. New count: %d\n", orderId, q->count);
    return 1;
}

/**
 * Displays the entire queue in terminal.
 */
void displayQueue(const OrderQueue *q) {
    if (q == NULL || isEmpty(q)) {
        printf("[QUEUE DISPLAY] (Empty)\n");
        return;
    }

    printf("\n=======================================================\n");
    printf("              CURRENT ORDER QUEUE (FIFO)               \n");
    printf("=======================================================\n");
    printf("FRONT (%p)\n   ↓\n", (void *)q->front);

    const Order *curr = q->front;
    int pos = 1;
    while (curr != NULL) {
        printf(" [%d] Order #%d | Table %d | %s | Status: %s | Total: ₹%.2f (next: %p)\n",
               pos++, curr->orderId, curr->tableNumber, curr->customerName,
               curr->status, curr->total, (void *)curr->next);
        curr = curr->next;
    }

    printf("   ↑\nREAR (%p)\n", (void *)q->rear);
    printf("Total Orders in Queue: %d\n", q->count);
    printf("=======================================================\n\n");
}

/**
 * Serializes all queue orders into a JSON array string.
 */
char* queueToJson(const OrderQueue *q) {
    size_t cap = 2048 + (q->count * 1024);
    char *json = (char *)malloc(cap);
    if (!json) return NULL;

    strcpy(json, "[");
    const Order *curr = q->front;
    int first = 1;

    while (curr != NULL) {
        char *ordJson = orderToJson(curr);
        if (ordJson) {
            if (!first) strcat(json, ",");
            strcat(json, ordJson);
            first = 0;
            free(ordJson);
        }
        curr = curr->next;
    }

    strcat(json, "]");
    return json;
}

/**
 * Serializes the Queue structure with explicit FRONT, REAR, and node pointers,
 * alongside each order's internal item linked list pointer data.
 */
char* queueVisualizationToJson(const OrderQueue *q) {
    size_t cap = 8192 + (q->count * 2048);
    char *json = (char *)malloc(cap);
    if (!json) return NULL;

    char frontStr[32], rearStr[32];
    if (q->front) snprintf(frontStr, sizeof(frontStr), "%p", (void *)q->front);
    else strcpy(frontStr, "NULL");

    if (q->rear) snprintf(rearStr, sizeof(rearStr), "%p", (void *)q->rear);
    else strcpy(rearStr, "NULL");

    snprintf(json, cap,
             "{"
             "\"frontPtr\":\"%s\","
             "\"rearPtr\":\"%s\","
             "\"count\":%d,"
             "\"isEmpty\":%s,"
             "\"nodes\":[",
             frontStr, rearStr, q->count,
             isEmpty(q) ? "true" : "false");

    const Order *curr = q->front;
    int first = 1;
    int index = 0;

    while (curr != NULL) {
        char nodeBuf[2048];
        char nextStr[32];
        if (curr->next) snprintf(nextStr, sizeof(nextStr), "%p", (void *)curr->next);
        else strcpy(nextStr, "NULL");

        char *itemsVis = itemsLinkedListVisualizationToJson(curr->items);

        snprintf(nodeBuf, sizeof(nodeBuf),
                 "%s{"
                 "\"index\":%d,"
                 "\"orderId\":%d,"
                 "\"customerName\":\"%s\","
                 "\"tableNumber\":%d,"
                 "\"status\":\"%s\","
                 "\"total\":%.2f,"
                 "\"createdAt\":\"%s\","
                 "\"nodePtr\":\"%p\","
                 "\"nextPtr\":\"%s\","
                 "\"isFront\":%s,"
                 "\"isRear\":%s,"
                 "\"itemsLinkedList\":%s"
                 "}",
                 first ? "" : ",",
                 index++,
                 curr->orderId,
                 curr->customerName,
                 curr->tableNumber,
                 curr->status,
                 curr->total,
                 curr->createdAt,
                 (void *)curr,
                 nextStr,
                 (curr == q->front) ? "true" : "false",
                 (curr == q->rear) ? "true" : "false",
                 itemsVis ? itemsVis : "{}");

        if (itemsVis) free(itemsVis);

        strcat(json, nodeBuf);
        first = 0;
        curr = curr->next;
    }

    strcat(json, "]}");
    return json;
}

/**
 * Frees all orders currently in the queue.
 */
void freeQueue(OrderQueue *q) {
    if (q == NULL) return;
    printf("[QUEUE] Freeing all orders in queue...\n");
    while (!isEmpty(q)) {
        Order *ord = dequeue(q);
        if (ord != NULL) {
            freeOrder(ord);
        }
    }
}
