/**
 * ============================================================================
 * RESTAURANT ORDER MANAGEMENT SYSTEM - DATA STRUCTURES MINI PROJECT
 * File: main.c
 * Description: Program entry point for the C Backend Server.
 * Demonstrates: Initializing Queue, seeding orders, starting HTTP server,
 * handling graceful termination signals to free heap-allocated memory.
 * ============================================================================
 */

#include "order.h"
#include "queue.h"
#include "linkedlist.h"
#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

/* Signal handler for clean exit and memory cleanup */
static void handleSignal(int sig) {
    (void)sig;
    printf("\n\n[SHUTDOWN] Received termination signal. Cleaning up data structures...\n");
    cleanupRestaurantState();
    printf("[SHUTDOWN] Clean exit. Bye!\n");
    exit(0);
}

int main(int argc, char *argv[]) {
    int port = 8080;
    if (argc > 1) {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            port = 8080;
        }
    }

    /* Register signal handlers for memory leak prevention */
    signal(SIGINT, handleSignal);
    signal(SIGTERM, handleSignal);

    /* Initialize Queue and Seed Sample Orders */
    initRestaurantState();

    /* Start HTTP API Server */
    int result = startHttpServer(port);

    /* Clean up before return */
    cleanupRestaurantState();
    return result;
}
