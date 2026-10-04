/**
 * ============================================================================
 * RESTAURANT ORDER MANAGEMENT SYSTEM - DATA STRUCTURES MINI PROJECT
 * File: order.h
 * Description: Header definitions for Order and Item structures.
 * Practical Application of Queue and Singly Linked List Data Structures.
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
 * DATA STRUCTURE 1: LINKED LIST NODE (Item)
 * Represents a single food item in an order.
 * Each order maintains a singly linked list of items.
 */
typedef struct Item {
    int itemId;               /* Unique ID of the menu item */
    char name[100];           /* Name of the food item */
    float price;              /* Unit price of the item */
    int quantity;             /* Ordered quantity */
    struct Item *next;        /* Pointer to the next item node in the linked list */
} Item;

/**
 * DATA STRUCTURE 2: QUEUE NODE (Order)
 * Represents a complete customer order in the FIFO order queue.
 * Contains metadata and a pointer to the head of its item linked list.
 */
typedef struct Order {
    int orderId;              /* Unique incremental Order ID (e.g., 101, 102) */
    char customerName[100];   /* Name of the dining or takeaway customer */
    int tableNumber;          /* Dine-in table number */
    Item *items;              /* HEAD pointer to the linked list of food items */
    float total;              /* Calculated total bill amount (sum of price * qty) */
    char status[30];          /* Status: "Pending", "Preparing", "Ready", "Completed", "Cancelled" */
    char createdAt[32];       /* Timestamp when the order was enqueued */
    struct Order *next;       /* Pointer to next order in the FIFO Queue */
} Order;

/* Order lifecycle helper functions */
Order* createOrder(int orderId, const char *customerName, int tableNumber);
void updateOrderStatus(Order *order, const char *newStatus);
void freeOrder(Order *order);
char* orderToJson(const Order *order);

#endif /* ORDER_H */
