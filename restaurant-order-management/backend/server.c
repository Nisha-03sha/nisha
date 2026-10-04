/**
 * ============================================================================
 * RESTAURANT ORDER MANAGEMENT SYSTEM - DATA STRUCTURES MINI PROJECT
 * File: server.c
 * Description: HTTP Server and API handling in C.
 * 
 * Implements real HTTP/1.1 API endpoints:
 *   GET    /api/menu
 *   GET    /api/orders
 *   GET    /api/orders/:id
 *   POST   /api/orders
 *   POST   /api/orders/:id/items
 *   DELETE /api/orders/:id
 *   PUT    /api/orders/:id/status
 *   POST   /api/orders/process-next
 *   GET    /api/stats
 *   GET    /api/queue/visualize
 *   GET    /api/logs
 * 
 * Works seamlessly on Linux (Debian/Ubuntu) and Windows (GCC/MinGW/MSYS2).
 * ============================================================================
 */

#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#ifdef _WIN32
  #define _WIN32_WINNT 0x0600
  #include <winsock2.h>
  #include <ws2tcpip.h>
  typedef int socklen_t;
  #define CLOSE_SOCKET(s) closesocket(s)
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  #define CLOSE_SOCKET(s) close(s)
  typedef int SOCKET;
  #define INVALID_SOCKET -1
  #define SOCKET_ERROR -1
#endif

#define BUFFER_SIZE 65536
#define MAX_LOGS 100

/* Global Restaurant State */
static OrderQueue orderQueue;            /* FIFO Queue for active pending/preparing orders */
static Order *completedOrdersList = NULL;/* Singly Linked List of completed orders (history) */
static Order *cancelledOrdersList = NULL;/* Singly Linked List of cancelled orders */
static int currentOrderIdCounter = 101;  /* Incremental order counter starting at #101 */

/* In-memory Menu Database */
static const MenuItem restaurantMenu[] = {
    {1,  "Chicken Biriyani",      "Biriyani",    180.0f, "Fragrant basmati rice cooked with tender spiced chicken pieces and aromatic herbs", 0, "🍗"},
    {2,  "Veg Biriyani",          "Biriyani",    140.0f, "Authentic dum biriyani layered with fresh garden vegetables and saffron", 1, "🥕"},
    {3,  "Chicken 65",            "Starters",    160.0f, "Crispy, deep-fried spicy chicken morsels seasoned with curry leaves and chili", 0, "🔥"},
    {4,  "Fried Rice",            "Main Course", 130.0f, "Wok-tossed aromatic rice with fresh bell peppers, scallions, and soy seasonings", 1, "🍚"},
    {5,  "Noodles",               "Main Course", 120.0f, "Stir-fried hakka noodles with crisp shredded veggies in garlic-chili sauce", 1, "🍜"},
    {6,  "Paneer Butter Masala",  "Main Course", 170.0f, "Soft cottage cheese cubes simmered in rich creamy butter-tomato gravy", 1, "🧀"},
    {7,  "Masala Dosa",           "South Indian", 80.0f, "Crispy golden crepe filled with savory spiced potato mash, served with chutney", 1, "🥞"},
    {8,  "French Fries",          "Snacks",       90.0f, "Golden crispy potato batons tossed in sea salt and zesty herb seasoning", 1, "🍟"},
    {9,  "Coke",                  "Beverages",    40.0f, "Chilled refreshing Coca-Cola carbonated beverage can (330ml)", 1, "🥤"},
    {10, "Fresh Juice",           "Beverages",    70.0f, "Freshly squeezed seasonal fruit juice with a touch of mint and citrus", 1, "🍹"},
    {11, "Butter Naan",           "Breads",       45.0f, "Traditional tandoor-baked flatbread brushed with melted salted butter", 1, "🫓"},
    {12, "Gulab Jamun (2 pcs)",   "Desserts",     60.0f, "Soft golden milk dumplings soaked in cardamom and rose sugar syrup", 1, "🍯"}
};
static const int menuCount = sizeof(restaurantMenu) / sizeof(restaurantMenu[0]);

/* Server Event Log Buffer */
static char serverLogs[MAX_LOGS][512];
static int logCount = 0;

void logServerEvent(const char *format, ...) {
    char buffer[384];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timeStr[16];
    if (t != NULL) {
        strftime(timeStr, sizeof(timeStr), "%H:%M:%S", t);
    } else {
        strcpy(timeStr, "00:00:00");
    }

    char fullMsg[512];
    snprintf(fullMsg, sizeof(fullMsg), "[%s] %s", timeStr, buffer);
    printf("%s\n", fullMsg);
    fflush(stdout);

    /* Store in circular log array */
    if (logCount < MAX_LOGS) {
        snprintf(serverLogs[logCount], sizeof(serverLogs[0]), "%s", fullMsg);
        logCount++;
    } else {
        /* Shift array */
        for (int i = 0; i < MAX_LOGS - 1; i++) {
            snprintf(serverLogs[i], sizeof(serverLogs[0]), "%s", serverLogs[i + 1]);
        }
        snprintf(serverLogs[MAX_LOGS - 1], sizeof(serverLogs[0]), "%s", fullMsg);
    }
}

char* getRecentLogsJson(void) {
    size_t size = 128 + (logCount * 300);
    char *json = (char *)malloc(size);
    if (!json) return strdup("[]");

    strcpy(json, "[");
    for (int i = 0; i < logCount; i++) {
        char escaped[300];
        int k = 0;
        for (size_t j = 0; j < strlen(serverLogs[i]) && k < 280; j++) {
            if (serverLogs[i][j] == '"') {
                escaped[k++] = '\\';
                escaped[k++] = '"';
            } else if (serverLogs[i][j] == '\n') {
                escaped[k++] = ' ';
            } else {
                escaped[k++] = serverLogs[i][j];
            }
        }
        escaped[k] = '\0';

        char line[350];
        snprintf(line, sizeof(line), "%s\"%s\"", (i == 0) ? "" : ",", escaped);
        strcat(json, line);
    }
    strcat(json, "]");
    return json;
}

const MenuItem* findMenuItemById(int id) {
    for (int i = 0; i < menuCount; i++) {
        if (restaurantMenu[i].id == id) {
            return &restaurantMenu[i];
        }
    }
    return NULL;
}

/* Helper to find an order by ID across active Queue, Completed list, and Cancelled list */
Order* findOrderAnywhere(int orderId, int *location) {
    /* 1: active queue */
    Order *ord = findOrderInQueue(&orderQueue, orderId);
    if (ord) {
        if (location) *location = 1;
        return ord;
    }

    /* 2: completed list */
    ord = completedOrdersList;
    while (ord) {
        if (ord->orderId == orderId) {
            if (location) *location = 2;
            return ord;
        }
        ord = ord->next;
    }

    /* 3: cancelled list */
    ord = cancelledOrdersList;
    while (ord) {
        if (ord->orderId == orderId) {
            if (location) *location = 3;
            return ord;
        }
        ord = ord->next;
    }

    if (location) *location = 0;
    return NULL;
}

/* Initialize starting orders for mini-project demonstration */
void seedInitialOrders(void) {
    logServerEvent("Initializing Restaurant Data Structures...");
    initQueue(&orderQueue);

    /* Sample Order 1: Rahul at Table 12 */
    Order *o1 = createOrder(currentOrderIdCounter++, "Rahul Sharma", 12);
    addItem(o1, 1, "Chicken Biriyani", 180.0f, 1);
    addItem(o1, 3, "Chicken 65", 160.0f, 1);
    addItem(o1, 9, "Coke", 40.0f, 1);
    updateOrderStatus(o1, "Preparing");
    enqueue(&orderQueue, o1);
    logServerEvent("[SEED] Enqueued Order #%d (Rahul Sharma, Table 12, Total: ₹%.2f)", o1->orderId, o1->total);

    /* Sample Order 2: Priya at Table 4 */
    Order *o2 = createOrder(currentOrderIdCounter++, "Priya Patel", 4);
    addItem(o2, 2, "Veg Biriyani", 140.0f, 1);
    addItem(o2, 6, "Paneer Butter Masala", 170.0f, 1);
    addItem(o2, 11, "Butter Naan", 45.0f, 2);
    updateOrderStatus(o2, "Pending");
    enqueue(&orderQueue, o2);
    logServerEvent("[SEED] Enqueued Order #%d (Priya Patel, Table 4, Total: ₹%.2f)", o2->orderId, o2->total);

    /* Sample Order 3: Ankit at Table 7 */
    Order *o3 = createOrder(currentOrderIdCounter++, "Ankit Verma", 7);
    addItem(o3, 7, "Masala Dosa", 80.0f, 2);
    addItem(o3, 10, "Fresh Juice", 70.0f, 2);
    updateOrderStatus(o3, "Pending");
    enqueue(&orderQueue, o3);
    logServerEvent("[SEED] Enqueued Order #%d (Ankit Verma, Table 7, Total: ₹%.2f)", o3->orderId, o3->total);
}

void initRestaurantState(void) {
    seedInitialOrders();
}

void cleanupRestaurantState(void) {
    logServerEvent("Cleaning up dynamic memory allocations...");
    freeQueue(&orderQueue);

    /* Free completed list */
    Order *curr = completedOrdersList;
    while (curr) {
        Order *temp = curr;
        curr = curr->next;
        freeOrder(temp);
    }
    completedOrdersList = NULL;

    /* Free cancelled list */
    curr = cancelledOrdersList;
    while (curr) {
        Order *temp = curr;
        curr = curr->next;
        freeOrder(temp);
    }
    cancelledOrdersList = NULL;
    logServerEvent("All C memory freed successfully.");
}

/* Simple JSON String Extraction Helper */
static int getJsonString(const char *json, const char *key, char *out, size_t maxLen) {
    char pattern[128];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    const char *pos = strstr(json, pattern);
    if (!pos) return 0;

    pos += strlen(pattern);
    while (*pos && (*pos == ' ' || *pos == ':' || *pos == '\t')) pos++;
    if (*pos != '"') return 0;
    pos++; /* skip opening quote */

    size_t idx = 0;
    while (*pos && *pos != '"' && idx < maxLen - 1) {
        if (*pos == '\\' && *(pos + 1)) pos++; /* skip escape */
        out[idx++] = *pos++;
    }
    out[idx] = '\0';
    return 1;
}

/* Simple JSON Integer Extraction Helper */
static int getJsonInt(const char *json, const char *key, int defaultVal) {
    char pattern[128];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    const char *pos = strstr(json, pattern);
    if (!pos) return defaultVal;

    pos += strlen(pattern);
    while (*pos && (*pos == ' ' || *pos == ':' || *pos == '\t')) pos++;
    return atoi(pos);
}

/* Send HTTP Response Helper */
static void sendHttpResponse(SOCKET clientSocket, int statusCode, const char *statusText,
                             const char *contentType, const char *body) {
    char header[1024];
    int bodyLen = body ? (int)strlen(body) : 0;

    snprintf(header, sizeof(header),
             "HTTP/1.1 %d %s\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %d\r\n"
             "Access-Control-Allow-Origin: *\r\n"
             "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
             "Access-Control-Allow-Headers: Content-Type, Authorization\r\n"
             "Connection: close\r\n"
             "\r\n",
             statusCode, statusText, contentType, bodyLen);

    send(clientSocket, header, (int)strlen(header), 0);
    if (bodyLen > 0) {
        send(clientSocket, body, bodyLen, 0);
    }
}

/* HTTP Handlers */

/* GET /api/menu */
static void handleGetMenu(SOCKET clientSocket) {
    size_t size = 4096;
    char *json = (char *)malloc(size);
    if (!json) {
        sendHttpResponse(clientSocket, 500, "Internal Error", "application/json", "{\"error\":\"Memory error\"}");
        return;
    }

    strcpy(json, "[");
    for (int i = 0; i < menuCount; i++) {
        char itemBuf[512];
        snprintf(itemBuf, sizeof(itemBuf),
                 "%s{"
                 "\"id\":%d,"
                 "\"name\":\"%s\","
                 "\"category\":\"%s\","
                 "\"price\":%.2f,"
                 "\"description\":\"%s\","
                 "\"isVeg\":%s,"
                 "\"icon\":\"%s\""
                 "}",
                 (i == 0) ? "" : ",",
                 restaurantMenu[i].id,
                 restaurantMenu[i].name,
                 restaurantMenu[i].category,
                 restaurantMenu[i].price,
                 restaurantMenu[i].description,
                 restaurantMenu[i].isVeg ? "true" : "false",
                 restaurantMenu[i].icon);
        strcat(json, itemBuf);
    }
    strcat(json, "]");

    sendHttpResponse(clientSocket, 200, "OK", "application/json", json);
    free(json);
}

/* GET /api/orders */
static void handleGetOrders(SOCKET clientSocket) {
    /* Return both queue and completed/cancelled */
    size_t cap = 32768;
    char *json = (char *)malloc(cap);
    if (!json) {
        sendHttpResponse(clientSocket, 500, "Internal Error", "application/json", "{\"error\":\"Memory error\"}");
        return;
    }

    strcpy(json, "{\"queue\":");
    char *qJson = queueToJson(&orderQueue);
    strcat(json, qJson ? qJson : "[]");
    if (qJson) free(qJson);

    strcat(json, ",\"completed\":[");
    Order *curr = completedOrdersList;
    int first = 1;
    while (curr) {
        char *ordJson = orderToJson(curr);
        if (ordJson) {
            if (!first) strcat(json, ",");
            strcat(json, ordJson);
            first = 0;
            free(ordJson);
        }
        curr = curr->next;
    }
    strcat(json, "],\"cancelled\":[");

    curr = cancelledOrdersList;
    first = 1;
    while (curr) {
        char *ordJson = orderToJson(curr);
        if (ordJson) {
            if (!first) strcat(json, ",");
            strcat(json, ordJson);
            first = 0;
            free(ordJson);
        }
        curr = curr->next;
    }
    strcat(json, "]}");

    sendHttpResponse(clientSocket, 200, "OK", "application/json", json);
    free(json);
}

/* GET /api/orders/:id */
static void handleGetOrderById(SOCKET clientSocket, int orderId) {
    Order *ord = findOrderAnywhere(orderId, NULL);
    if (!ord) {
        sendHttpResponse(clientSocket, 404, "Not Found", "application/json", "{\"error\":\"Order ID not found\"}");
        return;
    }

    char *json = orderToJson(ord);
    sendHttpResponse(clientSocket, 200, "OK", "application/json", json);
    if (json) free(json);
}

/* POST /api/orders - Creates Order node, adds items linked list, enqueues to Queue */
static void handleCreateOrder(SOCKET clientSocket, const char *body) {
    char customerName[100] = "Guest";
    getJsonString(body, "customerName", customerName, sizeof(customerName));
    int tableNumber = getJsonInt(body, "tableNumber", 1);

    if (strlen(customerName) == 0) {
        strcpy(customerName, "Guest");
    }

    /* 1. Allocate Order node */
    Order *newOrder = createOrder(currentOrderIdCounter++, customerName, tableNumber);
    if (!newOrder) {
        sendHttpResponse(clientSocket, 500, "Internal Error", "application/json", "{\"error\":\"Failed to allocate order\"}");
        return;
    }

    logServerEvent("[ORDER] Created Order #%d for %s (Table %d)", newOrder->orderId, newOrder->customerName, newOrder->tableNumber);

    /* 2. Parse items array from JSON body */
    const char *itemsPos = strstr(body, "\"items\"");
    if (itemsPos) {
        itemsPos = strchr(itemsPos, '[');
        if (itemsPos) {
            const char *ptr = itemsPos;
            while ((ptr = strchr(ptr, '{')) != NULL) {
                const char *endObj = strchr(ptr, '}');
                if (!endObj) break;

                char objBuf[256];
                size_t len = (size_t)(endObj - ptr + 1);
                if (len >= sizeof(objBuf)) len = sizeof(objBuf) - 1;
                strncpy(objBuf, ptr, len);
                objBuf[len] = '\0';

                int itemId = getJsonInt(objBuf, "itemId", 0);
                int quantity = getJsonInt(objBuf, "quantity", 1);

                const MenuItem *m = findMenuItemById(itemId);
                if (m) {
                    /* Call Linked List insert operation */
                    addItem(newOrder, m->id, m->name, m->price, quantity);
                    logServerEvent("[LINKED LIST] Order #%d -> Added Item '%s' x%d (Price: ₹%.2f)",
                                   newOrder->orderId, m->name, quantity, m->price);
                }

                ptr = endObj + 1;
            }
        }
    }

    /* If no items were parsed, add a default item */
    if (newOrder->items == NULL) {
        const MenuItem *m = &restaurantMenu[0];
        addItem(newOrder, m->id, m->name, m->price, 1);
    }

    /* 3. Enqueue Order into FIFO Queue */
    enqueue(&orderQueue, newOrder);
    logServerEvent("[QUEUE ENQUEUE] Enqueued Order #%d at REAR. Current Queue Size: %d",
                   newOrder->orderId, getQueueSize(&orderQueue));

    char *json = orderToJson(newOrder);
    sendHttpResponse(clientSocket, 201, "Created", "application/json", json);
    if (json) free(json);
}

/* POST /api/orders/process-next - Dequeues front order from FIFO queue */
static void handleProcessNextOrder(SOCKET clientSocket) {
    if (isEmpty(&orderQueue)) {
        logServerEvent("[QUEUE DEQUEUE] Attempted to process order from empty queue");
        sendHttpResponse(clientSocket, 400, "Bad Request", "application/json", "{\"error\":\"Order queue is empty! No pending orders to serve.\"}");
        return;
    }

    /* Dequeue operation: removes order at FRONT */
    Order *servedOrder = dequeue(&orderQueue);
    if (!servedOrder) {
        sendHttpResponse(clientSocket, 500, "Internal Error", "application/json", "{\"error\":\"Dequeue failed\"}");
        return;
    }

    updateOrderStatus(servedOrder, "Completed");
    logServerEvent("[QUEUE DEQUEUE] Order #%d dequeued from FRONT and marked Completed! Remaining Queue Size: %d",
                   servedOrder->orderId, getQueueSize(&orderQueue));

    /* Insert into completed orders linked list (at HEAD) */
    servedOrder->next = completedOrdersList;
    completedOrdersList = servedOrder;

    char *json = orderToJson(servedOrder);
    sendHttpResponse(clientSocket, 200, "OK", "application/json", json);
    if (json) free(json);
}

/* PUT/POST /api/orders/:id/status - Update order status */
static void handleUpdateStatus(SOCKET clientSocket, int orderId, const char *body) {
    char newStatus[30] = "";
    getJsonString(body, "status", newStatus, sizeof(newStatus));

    if (strlen(newStatus) == 0) {
        sendHttpResponse(clientSocket, 400, "Bad Request", "application/json", "{\"error\":\"Missing status field\"}");
        return;
    }

    Order *ord = findOrderAnywhere(orderId, NULL);
    if (!ord) {
        sendHttpResponse(clientSocket, 404, "Not Found", "application/json", "{\"error\":\"Order not found\"}");
        return;
    }

    updateOrderStatus(ord, newStatus);
    logServerEvent("[ORDER] Order #%d status updated to '%s'", orderId, newStatus);

    char *json = orderToJson(ord);
    sendHttpResponse(clientSocket, 200, "OK", "application/json", json);
    if (json) free(json);
}

/* DELETE /api/orders/:id - Cancel order */
static void handleCancelOrder(SOCKET clientSocket, int orderId) {
    Order *ord = findOrderInQueue(&orderQueue, orderId);
    if (ord) {
        /* Remove from queue */
        removeOrderFromQueue(&orderQueue, orderId);
        updateOrderStatus(ord, "Cancelled");

        /* Move to cancelled list */
        ord->next = cancelledOrdersList;
        cancelledOrdersList = ord;

        logServerEvent("[QUEUE] Order #%d cancelled and moved to cancelled list", orderId);

        char *json = orderToJson(ord);
        sendHttpResponse(clientSocket, 200, "OK", "application/json", json);
        if (json) free(json);
        return;
    }

    sendHttpResponse(clientSocket, 404, "Not Found", "application/json", "{\"error\":\"Active order not found in queue\"}");
}

/* POST /api/orders/:id/items - Add item to existing order's linked list */
static void handleAddOrderItem(SOCKET clientSocket, int orderId, const char *body) {
    Order *ord = findOrderInQueue(&orderQueue, orderId);
    if (!ord) {
        sendHttpResponse(clientSocket, 404, "Not Found", "application/json", "{\"error\":\"Active order not found\"}");
        return;
    }

    int itemId = getJsonInt(body, "itemId", 0);
    int quantity = getJsonInt(body, "quantity", 1);

    const MenuItem *m = findMenuItemById(itemId);
    if (!m) {
        sendHttpResponse(clientSocket, 400, "Bad Request", "application/json", "{\"error\":\"Invalid itemId\"}");
        return;
    }

    /* Call Linked List insert operation */
    addItem(ord, m->id, m->name, m->price, quantity);
    logServerEvent("[LINKED LIST] Added item '%s' x%d to Order #%d (New total: ₹%.2f)",
                   m->name, quantity, orderId, ord->total);

    char *json = orderToJson(ord);
    sendHttpResponse(clientSocket, 200, "OK", "application/json", json);
    if (json) free(json);
}

/* GET /api/stats - Aggregated metrics */
static void handleGetStats(SOCKET clientSocket) {
    int totalCount = 0;
    int pendingCount = 0;
    int preparingCount = 0;
    int readyCount = 0;
    int completedCount = 0;
    int cancelledCount = 0;
    float totalRevenue = 0.0f;

    /* Count active queue */
    const Order *curr = orderQueue.front;
    while (curr) {
        totalCount++;
        if (strcmp(curr->status, "Pending") == 0) pendingCount++;
        else if (strcmp(curr->status, "Preparing") == 0) preparingCount++;
        else if (strcmp(curr->status, "Ready") == 0) readyCount++;
        curr = curr->next;
    }

    /* Count completed list */
    curr = completedOrdersList;
    while (curr) {
        totalCount++;
        completedCount++;
        totalRevenue += curr->total;
        curr = curr->next;
    }

    /* Count cancelled list */
    curr = cancelledOrdersList;
    while (curr) {
        totalCount++;
        cancelledCount++;
        curr = curr->next;
    }

    char json[512];
    snprintf(json, sizeof(json),
             "{"
             "\"totalOrders\":%d,"
             "\"pendingOrders\":%d,"
             "\"preparingOrders\":%d,"
             "\"readyOrders\":%d,"
             "\"completedOrders\":%d,"
             "\"cancelledOrders\":%d,"
             "\"queueLength\":%d,"
             "\"totalRevenue\":%.2f"
             "}",
             totalCount, pendingCount, preparingCount, readyCount,
             completedCount, cancelledCount, orderQueue.count, totalRevenue);

    sendHttpResponse(clientSocket, 200, "OK", "application/json", json);
}

/* GET /api/queue/visualize - Complete C memory layout with pointers */
static void handleGetQueueVisualization(SOCKET clientSocket) {
    char *json = queueVisualizationToJson(&orderQueue);
    if (!json) {
        sendHttpResponse(clientSocket, 500, "Internal Error", "application/json", "{\"error\":\"Serialization error\"}");
        return;
    }

    sendHttpResponse(clientSocket, 200, "OK", "application/json", json);
    free(json);
}

/* GET /api/logs - Recent C server logs */
static void handleGetLogs(SOCKET clientSocket) {
    char *json = getRecentLogsJson();
    sendHttpResponse(clientSocket, 200, "OK", "application/json", json);
    free(json);
}

/* Main Request Router */
static void handleClientRequest(SOCKET clientSocket) {
    char buffer[BUFFER_SIZE];
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) {
        CLOSE_SOCKET(clientSocket);
        return;
    }

    buffer[bytesRead] = '\0';

    /* Parse request line: METHOD PATH HTTP/1.1 */
    char method[16] = {0};
    char path[256] = {0};
    sscanf(buffer, "%15s %255s", method, path);

    /* Handle CORS Preflight OPTIONS */
    if (strcmp(method, "OPTIONS") == 0) {
        sendHttpResponse(clientSocket, 204, "No Content", "text/plain", "");
        CLOSE_SOCKET(clientSocket);
        return;
    }

    /* Locate HTTP body */
    const char *body = strstr(buffer, "\r\n\r\n");
    if (body) {
        body += 4;
    } else {
        body = "";
    }

    /* Strip query string from path for routing */
    char cleanPath[256];
    strncpy(cleanPath, path, sizeof(cleanPath) - 1);
    cleanPath[sizeof(cleanPath) - 1] = '\0';
    char *queryMark = strchr(cleanPath, '?');
    if (queryMark) *queryMark = '\0';

    /* API Routes */
    if (strcmp(method, "GET") == 0 && strcmp(cleanPath, "/api/menu") == 0) {
        handleGetMenu(clientSocket);
    } else if (strcmp(method, "GET") == 0 && strcmp(cleanPath, "/api/orders") == 0) {
        handleGetOrders(clientSocket);
    } else if (strcmp(method, "GET") == 0 && strcmp(cleanPath, "/api/stats") == 0) {
        handleGetStats(clientSocket);
    } else if (strcmp(method, "GET") == 0 && strcmp(cleanPath, "/api/queue/visualize") == 0) {
        handleGetQueueVisualization(clientSocket);
    } else if (strcmp(method, "GET") == 0 && strcmp(cleanPath, "/api/logs") == 0) {
        handleGetLogs(clientSocket);
    } else if (strcmp(method, "POST") == 0 && strcmp(cleanPath, "/api/orders") == 0) {
        handleCreateOrder(clientSocket, body);
    } else if (strcmp(method, "POST") == 0 && strcmp(cleanPath, "/api/orders/process-next") == 0) {
        handleProcessNextOrder(clientSocket);
    } else if (strncmp(cleanPath, "/api/orders/", 12) == 0) {
        /* Parse order ID */
        int orderId = atoi(cleanPath + 12);
        char *subPath = strchr(cleanPath + 12, '/');

        if (subPath == NULL) {
            if (strcmp(method, "GET") == 0) {
                handleGetOrderById(clientSocket, orderId);
            } else if (strcmp(method, "DELETE") == 0) {
                handleCancelOrder(clientSocket, orderId);
            } else {
                sendHttpResponse(clientSocket, 405, "Method Not Allowed", "application/json", "{\"error\":\"Method not allowed\"}");
            }
        } else if (strcmp(subPath, "/status") == 0 && (strcmp(method, "PUT") == 0 || strcmp(method, "POST") == 0)) {
            handleUpdateStatus(clientSocket, orderId, body);
        } else if (strcmp(subPath, "/items") == 0 && strcmp(method, "POST") == 0) {
            handleAddOrderItem(clientSocket, orderId, body);
        } else {
            sendHttpResponse(clientSocket, 404, "Not Found", "application/json", "{\"error\":\"Route not found\"}");
        }
    } else {
        /* Static file fallback */
        sendHttpResponse(clientSocket, 200, "OK", "application/json",
                         "{\"status\":\"C Backend Active\",\"endpoints\":[\"/api/menu\",\"/api/orders\",\"/api/queue/visualize\",\"/api/stats\",\"/api/logs\"]}");
    }

    CLOSE_SOCKET(clientSocket);
}

/* Start the native C HTTP Server */
int startHttpServer(int port) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "[SERVER ERROR] WSAStartup failed!\n");
        return -1;
    }
#endif

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        fprintf(stderr, "[SERVER ERROR] Failed to create socket!\n");
        return -1;
    }

    int opt = 1;
#ifdef _WIN32
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));
#else
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons((unsigned short)port);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        fprintf(stderr, "[SERVER ERROR] Failed to bind to port %d!\n", port);
        CLOSE_SOCKET(serverSocket);
        return -1;
    }

    if (listen(serverSocket, 10) == SOCKET_ERROR) {
        fprintf(stderr, "[SERVER ERROR] Failed to listen on socket!\n");
        CLOSE_SOCKET(serverSocket);
        return -1;
    }

    printf("\n==========================================================\n");
    printf("  RESTAURANT ORDER MANAGEMENT SYSTEM - C BACKEND SERVER  \n");
    printf("==========================================================\n");
    printf("  [STATUS] HTTP Server running on port %d\n", port);
    printf("  [DATA STRUCTURES] FIFO Queue & Singly Linked Lists Active\n");
    printf("  [READY] Listening for incoming client API requests...\n");
    printf("==========================================================\n\n");
    fflush(stdout);

    while (1) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientLen);

        if (clientSocket != INVALID_SOCKET) {
            handleClientRequest(clientSocket);
        }
    }

    CLOSE_SOCKET(serverSocket);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
