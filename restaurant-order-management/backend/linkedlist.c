/**
 * ============================================================================
 * RESTAURANT ORDER MANAGEMENT SYSTEM - DATA STRUCTURES MINI PROJECT
 * File: linkedlist.c
 * Description: Implementation of Singly Linked List for Order Food Items.
 * 
 * Data Structure Concept:
 * A Singly Linked List is a linear data structure where each element (node)
 * consists of data fields and a pointer (`next`) pointing to the subsequent
 * node. The final node points to NULL.
 * 
 * In this restaurant application, each customer Order possesses a pointer
 * to its own linked list of ordered dishes. This allows each order to have
 * a completely dynamic number of items without fixed array size limitations.
 * ============================================================================
 */

#include "linkedlist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Creates and initializes a new Item node.
 * Time Complexity: O(1)
 * Space Complexity: O(1)
 */
Item* createItem(int itemId, const char *name, float price, int quantity) {
    /* Allocate heap memory for single Item node */
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
    newItem->next = NULL; /* Newly created node initially points to NULL */

    printf("[LINKED LIST] Allocated node at %p for item: '%s' (Qty: %d, Unit Price: ₹%.2f)\n",
           (void *)newItem, newItem->name, newItem->quantity, newItem->price);

    return newItem;
}

/**
 * Inserts an Item node into the Order's linked list.
 * If the list is empty (HEAD is NULL), the new node becomes the HEAD.
 * Otherwise, traverses to the tail and links the new node.
 * If an item with the same itemId already exists, its quantity is incremented.
 * 
 * Time Complexity: O(N) where N is number of items in the list.
 */
void addItem(Order *order, int itemId, const char *name, float price, int quantity) {
    if (order == NULL) {
        fprintf(stderr, "[ERROR] Cannot add item to NULL order!\n");
        return;
    }

    /* Check if item already exists in this order's linked list */
    Item *curr = order->items;
    while (curr != NULL) {
        if (curr->itemId == itemId) {
            curr->quantity += quantity;
            printf("[LINKED LIST] Updated existing item '%s' quantity to %d\n", curr->name, curr->quantity);
            order->total = calculateTotal(order);
            return;
        }
        curr = curr->next;
    }

    /* Allocate new node */
    Item *newNode = createItem(itemId, name, price, quantity);
    if (newNode == NULL) return;

    /* Case 1: List is currently empty (HEAD == NULL) */
    if (order->items == NULL) {
        order->items = newNode;
        printf("[LINKED LIST] HEAD of Order #%d set to item '%s' (%p)\n",
               order->orderId, newNode->name, (void *)newNode);
    } else {
        /* Case 2: List contains nodes; traverse to the end */
        curr = order->items;
        while (curr->next != NULL) {
            curr = curr->next;
        }
        /* Link new node at the end */
        curr->next = newNode;
        printf("[LINKED LIST] Appended '%s' to tail of Order #%d linked list (Previous tail %p -> New %p)\n",
               newNode->name, order->orderId, (void *)curr, (void *)newNode);
    }

    /* Recalculate order total amount */
    order->total = calculateTotal(order);
}

/**
 * Removes an item matching itemId from the linked list.
 * Adjusts pointers to bypass the deleted node, then frees its memory.
 * 
 * Time Complexity: O(N)
 */
int removeItem(Order *order, int itemId) {
    if (order == NULL || order->items == NULL) {
        return 0; /* List is empty */
    }

    Item *curr = order->items;
    Item *prev = NULL;

    /* Traverse to locate the node with matching itemId */
    while (curr != NULL && curr->itemId != itemId) {
        prev = curr;
        curr = curr->next;
    }

    if (curr == NULL) {
        /* Item not found in linked list */
        return 0;
    }

    /* Node found! Update pointers */
    if (prev == NULL) {
        /* Deleting the HEAD node */
        order->items = curr->next;
        printf("[LINKED LIST] Removed HEAD node '%s' (%p). New HEAD: %p\n",
               curr->name, (void *)curr, (void *)order->items);
    } else {
        /* Deleting middle or tail node */
        prev->next = curr->next;
        printf("[LINKED LIST] Removed node '%s' (%p). Previous node (%p) now points to (%p)\n",
               curr->name, (void *)curr, (void *)prev, (void *)curr->next);
    }

    /* Free memory to prevent memory leaks */
    free(curr);

    /* Recalculate order total */
    order->total = calculateTotal(order);
    return 1;
}

/**
 * Traverses the linked list from HEAD to NULL to compute total cost.
 * Time Complexity: O(N)
 */
float calculateTotal(const Order *order) {
    if (order == NULL) return 0.0f;

    float sum = 0.0f;
    const Item *curr = order->items;

    while (curr != NULL) {
        sum += (curr->price * curr->quantity);
        curr = curr->next; /* Move to next node */
    }

    return sum;
}

/**
 * Displays the linked list in terminal format for console debugging.
 */
void displayItems(const Order *order) {
    if (order == NULL) return;

    printf("--- Items for Order #%d ---\n", order->orderId);
    printf("HEAD -> ");

    const Item *curr = order->items;
    while (curr != NULL) {
        printf("[%s | Qty: %d | ₹%.2f | next: %p] -> ",
               curr->name, curr->quantity, curr->price, (void *)curr->next);
        curr = curr->next;
    }
    printf("NULL\n");
    printf("Total: ₹%.2f\n", order->total);
}

/**
 * Traverses and frees every node in the linked list.
 * Time Complexity: O(N)
 */
void freeItemList(Item *head) {
    Item *curr = head;
    while (curr != NULL) {
        Item *temp = curr;
        curr = curr->next; /* Advance pointer before freeing current node */
        printf("[LINKED LIST] Freeing item node '%s' at %p\n", temp->name, (void *)temp);
        free(temp);
    }
}

/**
 * Counts total items in linked list.
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
 * Serializes linked list of items into JSON string array.
 * Example: [{"itemId":1,"name":"Chicken Biriyani","price":180.00,"quantity":2,"subtotal":360.00}]
 */
char* itemsToJson(const Item *head) {
    /* Estimate buffer size */
    size_t bufSize = 256;
    int count = getItemCount(head);
    bufSize += (count * 256);

    char *json = (char *)malloc(bufSize);
    if (!json) return NULL;

    strcpy(json, "[");
    const Item *curr = head;
    int first = 1;

    while (curr != NULL) {
        char itemBuf[256];
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

/**
 * Serializes linked list with memory pointer representation
 * for frontend visual inspection.
 */
char* itemsLinkedListVisualizationToJson(const Item *head) {
    size_t bufSize = 512 + (getItemCount(head) * 350);
    char *json = (char *)malloc(bufSize);
    if (!json) return NULL;

    char headPtrStr[32];
    if (head != NULL) {
        snprintf(headPtrStr, sizeof(headPtrStr), "%p", (void *)head);
    } else {
        strcpy(headPtrStr, "NULL");
    }

    snprintf(json, bufSize,
             "{\"headPtr\":\"%s\",\"count\":%d,\"nodes\":[",
             headPtrStr, getItemCount(head));

    const Item *curr = head;
    int first = 1;
    while (curr != NULL) {
        char nodeBuf[350];
        char nextPtrStr[32];
        if (curr->next != NULL) {
            snprintf(nextPtrStr, sizeof(nextPtrStr), "%p", (void *)curr->next);
        } else {
            strcpy(nextPtrStr, "NULL");
        }

        snprintf(nodeBuf, sizeof(nodeBuf),
                 "%s{\"itemId\":%d,\"name\":\"%s\",\"price\":%.2f,\"quantity\":%d,"
                 "\"ptr\":\"%p\",\"nextPtr\":\"%s\",\"isHead\":%s,\"isTail\":%s}",
                 first ? "" : ",",
                 curr->itemId,
                 curr->name,
                 curr->price,
                 curr->quantity,
                 (void *)curr,
                 nextPtrStr,
                 (curr == head) ? "true" : "false",
                 (curr->next == NULL) ? "true" : "false");

        strcat(json, nodeBuf);
        first = 0;
        curr = curr->next;
    }

    strcat(json, "]}");
    return json;
}
