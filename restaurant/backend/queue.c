/**
 * ============================================================================
 * BASIL & EMBER - RESTAURANT ORDER MANAGEMENT SYSTEM
 * File: queue.c
 * Description: Implementation of FIFO (First-In, First-Out) Order Queue.
 * ============================================================================
 */

#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void initQueue(OrderQueue *q) {
    if (q == NULL) return;
    q->front = NULL;
    q->rear = NULL;
    q->count = 0;
}

int isEmpty(const OrderQueue *q) {
    if (q == NULL) return 1;
    return (q->front == NULL);
}

/**
 * Enqueue: Adds an Order to the REAR of the queue.
 * Time Complexity: O(1)
 */
int enqueue(OrderQueue *q, Order *newOrder) {
    if (q == NULL || newOrder == NULL) return 0;

    newOrder->next = NULL;

    if (q->front == NULL) {
        q->front = newOrder;
        q->rear = newOrder;
    } else {
        q->rear->next = newOrder;
        q->rear = newOrder;
    }

    q->count++;
    return 1;
}

/**
 * Dequeue: Removes the Order from the FRONT of the queue.
 * Time Complexity: O(1)
 */
Order* dequeue(OrderQueue *q) {
    if (q == NULL || isEmpty(q)) return NULL;

    Order *dequeued = q->front;
    q->front = q->front->next;

    if (q->front == NULL) {
        q->rear = NULL;
    }

    dequeued->next = NULL;
    q->count--;
    return dequeued;
}

Order* peek(const OrderQueue *q) {
    if (q == NULL || isEmpty(q)) return NULL;
    return q->front;
}

int getQueueSize(const OrderQueue *q) {
    if (q == NULL) return 0;
    return q->count;
}

Order* findOrderInQueue(const OrderQueue *q, int orderId) {
    if (q == NULL) return NULL;
    Order *curr = q->front;
    while (curr != NULL) {
        if (curr->orderId == orderId) return curr;
        curr = curr->next;
    }
    return NULL;
}

int removeOrderFromQueue(OrderQueue *q, int orderId) {
    if (q == NULL || isEmpty(q)) return 0;

    Order *curr = q->front;
    Order *prev = NULL;

    while (curr != NULL && curr->orderId != orderId) {
        prev = curr;
        curr = curr->next;
    }

    if (curr == NULL) return 0;

    if (prev == NULL) {
        q->front = curr->next;
        if (q->front == NULL) q->rear = NULL;
    } else {
        prev->next = curr->next;
        if (curr == q->rear) q->rear = prev;
    }

    curr->next = NULL;
    q->count--;
    return 1;
}

char* queueToJson(const OrderQueue *q) {
    size_t cap = 2048 + (q->count * 1024);
    char *json = (char *)malloc(cap);
    if (!json) return strdup("[]");

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

void freeQueue(OrderQueue *q) {
    if (q == NULL) return;
    while (!isEmpty(q)) {
        Order *ord = dequeue(q);
        if (ord != NULL) freeOrder(ord);
    }
}
