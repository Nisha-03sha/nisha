/**
 * ============================================================================
 * BASIL & EMBER - RESTAURANT ORDER MANAGEMENT SYSTEM
 * File: main.c
 * Description: Main entry point for the Basil & Ember C backend server.
 * ============================================================================
 */

#include "order.h"
#include "queue.h"
#include "linkedlist.h"
#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

static void handleSignal(int sig) {
    (void)sig;
    cleanupRestaurantState();
    exit(0);
}

int main(int argc, char *argv[]) {
    int port = 5050;
    if (argc > 1) {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            port = 5050;
        }
    }

    signal(SIGINT, handleSignal);
    signal(SIGTERM, handleSignal);

    initRestaurantState();
    int res = startHttpServer(port);
    cleanupRestaurantState();
    return res;
}
