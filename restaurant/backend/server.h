/**
 * ============================================================================
 * BASIL & EMBER - RESTAURANT ORDER MANAGEMENT SYSTEM
 * File: server.h
 * Description: HTTP Server and API router for Basil & Ember.
 * ============================================================================
 */

#ifndef SERVER_H
#define SERVER_H

#include "order.h"
#include "queue.h"
#include "linkedlist.h"

typedef struct {
    int id;
    char name[60];
    char category[30];
    float price;
    char description[180];
    int isVeg;
    int isChefSpecial;
    char imageUrl[200];
} MenuItem;

void initRestaurantState(void);
void cleanupRestaurantState(void);
int startHttpServer(int port);

#endif /* SERVER_H */
