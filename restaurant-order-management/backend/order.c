/**
 * ============================================================================
 * RESTAURANT ORDER MANAGEMENT SYSTEM - DATA STRUCTURES MINI PROJECT
 * File: order.c
 * Description: Implementation of Order creation, serialization, and lifecycle.
 * ============================================================================
 */

#include "order.h"
#include "linkedlist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * Creates and initializes an Order node.
 * Time Complexity: O(1)
 */
Order* createOrder(int orderId, const char *customerName, int tableNumber) {
    Order *newOrder = (Order *)malloc(sizeof(Order));
    if (newOrder == NULL) {
        fprintf(stderr, "[ERROR] Memory allocation failed for Order node!\n");
        return NULL;
    }

    newOrder->orderId = orderId;
    strncpy(newOrder->customerName, customerName, sizeof(newOrder->customerName) - 1);
    newOrder->customerName[sizeof(newOrder->customerName) - 1] = '\0';
    newOrder->tableNumber = tableNumber > 0 ? tableNumber : 1;

    /* Initially, the item linked list is empty (HEAD is NULL) */
    newOrder->items = NULL;
    newOrder->total = 0.0f;
    strcpy(newOrder->status, "Pending");

    /* Generate current human-readable timestamp */
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t != NULL) {
        strftime(newOrder->createdAt, sizeof(newOrder->createdAt), "%Y-%m-%d %H:%M:%S", t);
    } else {
        strcpy(newOrder->createdAt, "Unknown");
    }

    /* In a queue node, pointer to next order is initially NULL */
    newOrder->next = NULL;

    printf("[ORDER] Created Order #%d for customer '%s' at Table %d (Node address: %p)\n",
           newOrder->orderId, newOrder->customerName, newOrder->tableNumber, (void *)newOrder);

    return newOrder;
}

/**
 * Updates status of an existing order.
 */
void updateOrderStatus(Order *order, const char *newStatus) {
    if (order == NULL || newStatus == NULL) return;
    strncpy(order->status, newStatus, sizeof(order->status) - 1);
    order->status[sizeof(order->status) - 1] = '\0';
    printf("[ORDER] Status of Order #%d updated to '%s'\n", order->orderId, order->status);
}

/**
 * Frees an Order and its associated linked list of food items.
 * Time Complexity: O(N) where N is number of items in the order.
 */
void freeOrder(Order *order) {
    if (order == NULL) return;

    printf("[ORDER] Freeing Order #%d (Node address: %p)...\n", order->orderId, (void *)order);

    /* Free the linked list of items */
    if (order->items != NULL) {
        freeItemList(order->items);
        order->items = NULL;
    }

    /* Free the order node itself */
    free(order);
}

/**
 * Serializes an Order and its Linked List items into a JSON string.
 * Caller must free() the returned pointer.
 */
char* orderToJson(const Order *order) {
    if (order == NULL) return strdup("null");

    char *itemsJson = itemsToJson(order->items);
    if (!itemsJson) {
        itemsJson = strdup("[]");
    }

    /* Calculate buffer size */
    size_t len = strlen(itemsJson) + 512;
    char *json = (char *)malloc(len);
    if (!json) {
        free(itemsJson);
        return NULL;
    }

    snprintf(json, len,
             "{"
             "\"orderId\":%d,"
             "\"customerName\":\"%s\","
             "\"tableNumber\":%d,"
             "\"status\":\"%s\","
             "\"total\":%.2f,"
             "\"createdAt\":\"%s\","
             "\"items\":%s"
             "}",
             order->orderId,
             order->customerName,
             order->tableNumber,
             order->status,
             order->total,
             order->createdAt,
             itemsJson);

    free(itemsJson);
    return json;
}
