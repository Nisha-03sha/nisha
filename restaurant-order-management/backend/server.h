/**
 * ============================================================================
 * RESTAURANT ORDER MANAGEMENT SYSTEM - DATA STRUCTURES MINI PROJECT
 * File: server.h
 * Description: HTTP Server and API router interface for C backend.
 * Provides RESTful HTTP endpoints communicating with the Queue and Linked Lists.
 * ============================================================================
 */

#ifndef SERVER_H
#define SERVER_H

#include "order.h"
#include "queue.h"
#include "linkedlist.h"

/* Predefined Menu Item structure */
typedef struct {
    int id;
    char name[60];
    char category[30];
    float price;
    char description[160];
    int isVeg;
    char icon[30];
} MenuItem;

/* Global state function prototypes */
void initRestaurantState(void);
void cleanupRestaurantState(void);
void logServerEvent(const char *format, ...);
char* getRecentLogsJson(void);

/* Server socket runner */
int startHttpServer(int port);

#endif /* SERVER_H */
