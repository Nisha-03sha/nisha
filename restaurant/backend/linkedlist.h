/**
 * ============================================================================
 * BASIL & EMBER - RESTAURANT ORDER MANAGEMENT SYSTEM
 * File: linkedlist.h
 * Description: Singly Linked List operations for managing order items in C.
 * ============================================================================
 */

#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "order.h"

/* Linked List Operations */
Item* createItem(int itemId, const char *name, float price, int quantity);
void addItem(Order *order, int itemId, const char *name, float price, int quantity);
int removeItem(Order *order, int itemId);
float calculateTotal(const Order *order);
void freeItemList(Item *head);
int getItemCount(const Item *head);
char* itemsToJson(const Item *head);

#endif /* LINKEDLIST_H */
