/**
 * ============================================================================
 * BASIL & EMBER - RESTAURANT ORDER MANAGEMENT SYSTEM
 * File: order.h
 * Description: Order entity and linked list item node definitions.
 * Core Data Structures:
 *   - Singly Linked List: Stores items within an order dynamically.
 *   - FIFO Queue Node: Orders form a queue processed by the kitchen.
 * ============================================================================
 */

#ifndef ORDER_H
#define ORDER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Forward declaration */
struct Item;

/**
 * LINKED LIST NODE: Item
 * Represents an individual food item ordered by a customer.
 * Each order maintains a singly linked list of items.
 */
typedef struct Item {
    int itemId;               /* Unique ID of the menu item */
    char name[100];           /* Name of the food item */
    float price;              /* Unit price in INR */
    int quantity;             /* Ordered quantity */
    struct Item *next;        /* Pointer to the next item node in the linked list */
} Item;

/**
 * QUEUE NODE: Order
 * Represents an order placed by a customer in the FIFO order queue.
 */
typedef struct Order {
    int orderId;              /* Incremental Order Number (#1042, #1043, etc.) */
    char customerName[100];   /* Name of the dining/takeaway customer */
    char phone[24];           /* Customer contact phone number */
    int tableNumber;          /* Table number (e.g., 1 to 50) */
    Item *items;              /* HEAD pointer to the singly linked list of items */
    float total;              /* Total amount calculated from linked list */
    char status[30];          /* "Pending", "Preparing", "Ready", "Completed", "Cancelled" */
    char createdAt[32];       /* Timestamp when the order was placed */
    struct Order *next;       /* Pointer to the next order in the FIFO Queue */
} Order;

/* Order lifecycle helper functions */
Order* createOrder(int orderId, const char *customerName, const char *phone, int tableNumber);
void updateOrderStatus(Order *order, const char *newStatus);
void freeOrder(Order *order);
char* orderToJson(const Order *order);

#endif /* ORDER_H */
