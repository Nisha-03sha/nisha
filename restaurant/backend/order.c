/**
 * ============================================================================
 * BASIL & EMBER - RESTAURANT ORDER MANAGEMENT SYSTEM
 * File: order.c
 * Description: Implementation of Order creation, serialization, and status.
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
Order* createOrder(int orderId, const char *customerName, const char *phone, int tableNumber) {
    Order *newOrder = (Order *)malloc(sizeof(Order));
    if (newOrder == NULL) {
        fprintf(stderr, "[ERROR] Memory allocation failed for Order node!\n");
        return NULL;
    }

    newOrder->orderId = orderId;
    strncpy(newOrder->customerName, customerName, sizeof(newOrder->customerName) - 1);
    newOrder->customerName[sizeof(newOrder->customerName) - 1] = '\0';

    if (phone != NULL && strlen(phone) > 0) {
        strncpy(newOrder->phone, phone, sizeof(newOrder->phone) - 1);
        newOrder->phone[sizeof(newOrder->phone) - 1] = '\0';
    } else {
        strcpy(newOrder->phone, "N/A");
    }

    newOrder->tableNumber = tableNumber > 0 ? tableNumber : 1;
    newOrder->items = NULL;
    newOrder->total = 0.0f;
    strcpy(newOrder->status, "Pending");

    /* Current timestamp */
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t != NULL) {
        strftime(newOrder->createdAt, sizeof(newOrder->createdAt), "%Y-%m-%d %H:%M", t);
    } else {
        strcpy(newOrder->createdAt, "Just now");
    }

    newOrder->next = NULL;
    return newOrder;
}

/**
 * Updates status of an order (e.g., Pending -> Preparing -> Ready -> Completed)
 */
void updateOrderStatus(Order *order, const char *newStatus) {
    if (order == NULL || newStatus == NULL) return;
    strncpy(order->status, newStatus, sizeof(order->status) - 1);
    order->status[sizeof(order->status) - 1] = '\0';
}

/**
 * Frees an Order and its associated linked list of food items.
 */
void freeOrder(Order *order) {
    if (order == NULL) return;

    if (order->items != NULL) {
        freeItemList(order->items);
        order->items = NULL;
    }

    free(order);
}

/**
 * Serializes an Order and its items into a clean JSON string.
 */
char* orderToJson(const Order *order) {
    if (order == NULL) return strdup("null");

    char *itemsJson = itemsToJson(order->items);
    if (!itemsJson) {
        itemsJson = strdup("[]");
    }

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
             "\"phone\":\"%s\","
             "\"tableNumber\":%d,"
             "\"status\":\"%s\","
             "\"total\":%.2f,"
             "\"createdAt\":\"%s\","
             "\"items\":%s"
             "}",
             order->orderId,
             order->customerName,
             order->phone,
             order->tableNumber,
             order->status,
             order->total,
             order->createdAt,
             itemsJson);

    free(itemsJson);
    return json;
}
