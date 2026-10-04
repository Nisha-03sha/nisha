/**
 * ============================================================================
 * RESTAURANT ORDER MANAGEMENT SYSTEM - DATA STRUCTURES MINI PROJECT
 * File: linkedlist.h
 * Description: Interface for singly linked list operations on order items.
 * Demonstrates: Node insertion, node deletion, linear traversal,
 * dynamic memory allocation (malloc), and memory deallocation (free).
 * ============================================================================
 */

#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "order.h"

/* Linked List Operations */

/**
 * Creates a new Item node dynamically in heap memory using malloc.
 * Sets node->next = NULL.
 */
Item* createItem(int itemId, const char *name, float price, int quantity);

/**
 * Inserts an item into an Order's singly linked list.
 * Appends to the tail (or inserts if empty).
 * Recalculates order total upon insertion.
 */
void addItem(Order *order, int itemId, const char *name, float price, int quantity);

/**
 * Removes an item matching itemId from the Order's singly linked list.
 * Updates pointers, frees memory with free(), and recalculates total.
 * Returns 1 if removed, 0 if not found.
 */
int removeItem(Order *order, int itemId);

/**
 * Traverses the item linked list starting from order->items (HEAD)
 * and computes the cumulative sum of (price * quantity).
 */
float calculateTotal(const Order *order);

/**
 * Prints the linked list elements to stdout for debugging/console output:
 * HEAD -> [Item 1] -> [Item 2] -> NULL
 */
void displayItems(const Order *order);

/**
 * Traverses and frees all nodes in the linked list to prevent memory leaks.
 */
void freeItemList(Item *head);

/**
 * Counts the number of nodes in the linked list.
 */
int getItemCount(const Item *head);

/**
 * Serializes the linked list of items to a JSON array string.
 * Caller must free() the returned buffer.
 */
char* itemsToJson(const Item *head);

/**
 * Serializes the linked list with exact pointer memory addresses
 * for frontend visualizer demonstration.
 */
char* itemsLinkedListVisualizationToJson(const Item *head);

#endif /* LINKEDLIST_H */
