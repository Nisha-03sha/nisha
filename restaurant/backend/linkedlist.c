/**
 * ============================================================================
 * BASIL & EMBER - RESTAURANT ORDER MANAGEMENT SYSTEM
 * File: linkedlist.c
 * Description: Implementation of Singly Linked List for Order Food Items.
 * 
 * Each customer Order maintains an internal singly linked list of dishes.
 * Nodes are allocated dynamically via malloc() and freed with free().
 * ============================================================================
 */

#include "linkedlist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Creates and initializes a new Item node.
 * Time Complexity: O(1)
 */
Item* createItem(int itemId, const char *name, float price, int quantity) {
    Item *newItem = (Item *)malloc(sizeof(Item));
    if (newItem == NULL) {
        fprintf(stderr, "[ERROR] Memory allocation failed for Item node!\n");
        return NULL;
    }

    newItem->itemId = itemId;
    strncpy(newItem->name, name, sizeof(newItem->name) - 1);
    newItem->name[sizeof(newItem->name) - 1] = '\0';
    newItem->price = price;
    newItem->quantity = quantity > 0 ? quantity : 1;
    newItem->next = NULL;

    return newItem;
}

/**
 * Inserts an Item node into the Order's singly linked list.
 * If the item already exists in this order, increments the quantity.
 * Recalculates total bill upon insertion.
 * Time Complexity: O(N) where N is number of unique items in order.
 */
void addItem(Order *order, int itemId, const char *name, float price, int quantity) {
    if (order == NULL) return;

    /* Check if item already exists in linked list */
    Item *curr = order->items;
    while (curr != NULL) {
        if (curr->itemId == itemId) {
            curr->quantity += quantity;
            order->total = calculateTotal(order);
            return;
        }
        curr = curr->next;
    }

    Item *newNode = createItem(itemId, name, price, quantity);
    if (newNode == NULL) return;

    /* If list is empty, make new node the HEAD */
    if (order->items == NULL) {
        order->items = newNode;
    } else {
        /* Traverse to tail and link */
        curr = order->items;
        while (curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = newNode;
    }

    order->total = calculateTotal(order);
}

/**
 * Removes an item node matching itemId from the linked list.
 * Time Complexity: O(N)
 */
int removeItem(Order *order, int itemId) {
    if (order == NULL || order->items == NULL) return 0;

    Item *curr = order->items;
    Item *prev = NULL;

    while (curr != NULL && curr->itemId != itemId) {
        prev = curr;
        curr = curr->next;
    }

    if (curr == NULL) return 0;

    if (prev == NULL) {
        order->items = curr->next;
    } else {
        prev->next = curr->next;
    }

    free(curr);
    order->total = calculateTotal(order);
    return 1;
}

/**
 * Traverses the linked list from HEAD to NULL to compute total order cost.
 * Time Complexity: O(N)
 */
float calculateTotal(const Order *order) {
    if (order == NULL) return 0.0f;

    float sum = 0.0f;
    const Item *curr = order->items;

    while (curr != NULL) {
        sum += (curr->price * curr->quantity);
        curr = curr->next;
    }

    return sum;
}

/**
 * Frees all nodes in the linked list to prevent memory leaks.
 * Time Complexity: O(N)
 */
void freeItemList(Item *head) {
    Item *curr = head;
    while (curr != NULL) {
        Item *temp = curr;
        curr = curr->next;
        free(temp);
    }
}

/**
 * Returns total count of unique items in linked list.
 */
int getItemCount(const Item *head) {
    int count = 0;
    const Item *curr = head;
    while (curr != NULL) {
        count++;
        curr = curr->next;
    }
    return count;
}

/**
 * Serializes linked list of items into a clean JSON string array.
 */
char* itemsToJson(const Item *head) {
    size_t bufSize = 512 + (getItemCount(head) * 300);
    char *json = (char *)malloc(bufSize);
    if (!json) return strdup("[]");

    strcpy(json, "[");
    const Item *curr = head;
    int first = 1;

    while (curr != NULL) {
        char itemBuf[300];
        float subtotal = curr->price * curr->quantity;
        snprintf(itemBuf, sizeof(itemBuf),
                 "%s{\"itemId\":%d,\"name\":\"%s\",\"price\":%.2f,\"quantity\":%d,\"subtotal\":%.2f}",
                 first ? "" : ",",
                 curr->itemId,
                 curr->name,
                 curr->price,
                 curr->quantity,
                 subtotal);
        strcat(json, itemBuf);
        first = 0;
        curr = curr->next;
    }

    strcat(json, "]");
    return json;
}
