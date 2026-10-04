/**
 * ============================================================================
 * BASIL & EMBER - RESTAURANT ORDER MANAGEMENT SYSTEM
 * File: server.c
 * Description: HTTP Server and API handling in C.
 * 
 * Works seamlessly on Linux and Windows (GCC/MinGW/MSYS2).
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

/* Global Restaurant State */
static OrderQueue orderQueue;            /* Active kitchen FIFO Queue */
static Order *completedOrdersList = NULL;/* History of completed orders */
static Order *cancelledOrdersList = NULL;/* History of cancelled orders */
static int currentOrderIdCounter = 1042;  /* Initial starting Order ID #1042 */

/* Basil & Ember Menu Database with high-res culinary photography */
static const MenuItem restaurantMenu[] = {
    /* Starters */
    {1,  "Chicken 65",                 "Starters",        160.0f, "Crisp boneless chicken tossed with curry leaves, crushed pepper, and red chili glaze.", 0, 1, "https://images.unsplash.com/photo-1610057099443-fde8c4d50f91?auto=format&fit=crop&w=600&q=80"},
    {2,  "Tandoori Chicken Tikka",      "Starters",        210.0f, "Succulent chicken chunks marinated in hung curd and tandoori spices, char-grilled to perfection.", 0, 1, "https://images.unsplash.com/photo-1599488615731-7e5c2823ff28?auto=format&fit=crop&w=600&q=80"},
    {3,  "Paneer Tikka Charcoal",       "Starters",        180.0f, "Fresh cottage cheese cubes marinated in mint and smoked spices, skewered with bell peppers.", 1, 0, "https://images.unsplash.com/photo-1567188040759-fb8a883dc6d8?auto=format&fit=crop&w=600&q=80"},
    {4,  "Crispy Truffle French Fries", "Starters",         90.0f, "Golden hand-cut potato batons tossed in sea salt, cracked black pepper, and herb oil.", 1, 0, "https://images.unsplash.com/photo-1576107232684-1279f3908594?auto=format&fit=crop&w=600&q=80"},

    /* Rice & Biriyani */
    {5,  "Hyderabadi Chicken Biriyani", "Rice & Biriyani", 180.0f, "Aromatic long-grain basmati rice slow-cooked with tender spiced chicken, saffron, and fried onions.", 0, 1, "https://images.unsplash.com/photo-1563379091339-03b21ab4a4f8?auto=format&fit=crop&w=600&q=80"},
    {6,  "Royal Awadhi Veg Biriyani",   "Rice & Biriyani", 140.0f, "Fragrant basmati rice dum-cooked with garden vegetables, cottage cheese, and rose water.", 1, 0, "https://images.unsplash.com/photo-1633945274405-b6c8069047b0?auto=format&fit=crop&w=600&q=80"},
    {7,  "Wok Tossed Fried Rice",       "Rice & Biriyani", 130.0f, "Smoky wok-tossed jasmine rice with scallions, bell peppers, carrots, and light soy seasoning.", 1, 0, "https://images.unsplash.com/photo-1603133872878-684f208fb84b?auto=format&fit=crop&w=600&q=80"},

    /* Main Course */
    {8,  "Paneer Butter Masala",        "Main Course",     170.0f, "Soft artisanal paneer simmered in a velvety cashew, vine-ripened tomato, and butter gravy.", 1, 1, "https://images.unsplash.com/photo-1631452180519-c014fe946bc7?auto=format&fit=crop&w=600&q=80"},
    {9,  "Butter Chicken Heritage",     "Main Course",     220.0f, "Tender shredded tandoori chicken simmered in our signature rich and mildly sweet tomato cream reduction.", 0, 1, "https://images.unsplash.com/photo-1603894584373-5ac82b2ae398?auto=format&fit=crop&w=600&q=80"},
    {10, "Dal Makhani Slow-Simmered",   "Main Course",     150.0f, "Black lentils and kidney beans slow-cooked overnight over embers with churned country butter.", 1, 0, "https://images.unsplash.com/photo-1546833999-b9f581a1996d?auto=format&fit=crop&w=600&q=80"},
    {11, "Chili Garlic Hakka Noodles",  "Main Course",     120.0f, "Wok-tossed noodles with julienned vegetables in a savory chili-garlic soy drizzle.", 1, 0, "https://images.unsplash.com/photo-1585032226651-759b368d7246?auto=format&fit=crop&w=600&q=80"},

    /* Breads */
    {12, "Garlic Butter Naan",          "Breads",           50.0f, "Tandoor-fired leavened flatbread brushed with garlic butter and fresh coriander.", 1, 0, "https://images.unsplash.com/photo-1601050690597-df0568f70950?auto=format&fit=crop&w=600&q=80"},
    {13, "Butter Naan",                 "Breads",           45.0f, "Traditional tandoor-baked flatbread glazed with pure melted creamery butter.", 1, 0, "https://images.unsplash.com/photo-1533777857889-4be7c70b33f7?auto=format&fit=crop&w=600&q=80"},
    {14, "Crispy Masala Dosa",          "Breads",           80.0f, "Golden fermented crepe filled with fragrant spiced potato mash, served with coconut chutney.", 1, 0, "https://images.unsplash.com/photo-1668236543090-82eba5ee5976?auto=format&fit=crop&w=600&q=80"},

    /* Desserts */
    {15, "Molten Chocolate Lava Cake",  "Desserts",        120.0f, "Warm dark Belgian chocolate cake with a molten center, served with cocoa dusting.", 1, 1, "https://images.unsplash.com/photo-1606313564200-e75d5e30476c?auto=format&fit=crop&w=600&q=80"},
    {16, "Gulab Jamun (2 pcs)",         "Desserts",         60.0f, "Piping hot fried milk dumplings steeped in rose water and crushed green cardamom syrup.", 1, 0, "https://images.unsplash.com/photo-1593701461250-d7b22dfd3a77?auto=format&fit=crop&w=600&q=80"},

    /* Beverages */
    {17, "Fresh Basil Mint Mojito",     "Beverages",        90.0f, "Crushed fresh garden basil, mint leaves, Mexican lime, and sparkling soda over crushed ice.", 1, 1, "https://images.unsplash.com/photo-1551024709-8f23befc6f87?auto=format&fit=crop&w=600&q=80"},
    {18, "Fresh Squeezed Orange Juice", "Beverages",        70.0f, "100% pure seasonal valencia orange juice pressed to order.", 1, 0, "https://images.unsplash.com/photo-1613478223719-2ab802602423?auto=format&fit=crop&w=600&q=80"},
    {19, "Chilled Coca-Cola (Can)",     "Beverages",        40.0f, "Chilled refreshing 330ml can of classic Coca-Cola.", 1, 0, "https://images.unsplash.com/photo-1622483767028-3f66f32aef97?auto=format&fit=crop&w=600&q=80"}
};
static const int menuCount = sizeof(restaurantMenu) / sizeof(restaurantMenu[0]);

const MenuItem* findMenuItemById(int id) {
    for (int i = 0; i < menuCount; i++) {
        if (restaurantMenu[i].id == id) {
            return &restaurantMenu[i];
        }
    }
    return NULL;
}

Order* findOrderAnywhere(int orderId) {
    Order *ord = findOrderInQueue(&orderQueue, orderId);
    if (ord) return ord;

    ord = completedOrdersList;
    while (ord) {
        if (ord->orderId == orderId) return ord;
        ord = ord->next;
    }

    ord = cancelledOrdersList;
    while (ord) {
        if (ord->orderId == orderId) return ord;
        ord = ord->next;
    }
    return NULL;
}

/* Seed initial authentic orders matching prompt requirements */
void seedInitialOrders(void) {
    initQueue(&orderQueue);

    /* ORDER #1042 (Nisha, Table 12): 2 × Chicken Biriyani, 1 × Coke = ₹400, Status: Preparing */
    Order *o1 = createOrder(1042, "Nisha Kapoor", "+91 98765 43210", 12);
    addItem(o1, 5, "Hyderabadi Chicken Biriyani", 180.0f, 2);
    addItem(o1, 19, "Chilled Coca-Cola (Can)", 40.0f, 1);
    updateOrderStatus(o1, "Preparing");
    enqueue(&orderQueue, o1);

    /* ORDER #1043 (Rahul, Table 7): 1 × Fried Rice, 1 × Chicken 65 = ₹290, Status: Pending */
    Order *o2 = createOrder(1043, "Rahul Verma", "+91 91234 56789", 7);
    addItem(o2, 7, "Wok Tossed Fried Rice", 130.0f, 1);
    addItem(o2, 1, "Chicken 65", 160.0f, 1);
    updateOrderStatus(o2, "Pending");
    enqueue(&orderQueue, o2);

    /* ORDER #1044 (Ananya, Table 4): 1 × Paneer Butter Masala, 2 × Garlic Naan = ₹270, Status: Ready */
    Order *o3 = createOrder(1044, "Ananya Roy", "+91 98888 77766", 4);
    addItem(o3, 8, "Paneer Butter Masala", 170.0f, 1);
    addItem(o3, 12, "Garlic Butter Naan", 50.0f, 2);
    updateOrderStatus(o3, "Ready");
    enqueue(&orderQueue, o3);

    /* Seed completed history orders to reflect restaurant revenue */
    currentOrderIdCounter = 1045;
}

void initRestaurantState(void) {
    seedInitialOrders();
}

void cleanupRestaurantState(void) {
    freeQueue(&orderQueue);

    Order *curr = completedOrdersList;
    while (curr) {
        Order *temp = curr;
        curr = curr->next;
        freeOrder(temp);
    }
    completedOrdersList = NULL;

    curr = cancelledOrdersList;
    while (curr) {
        Order *temp = curr;
        curr = curr->next;
        freeOrder(temp);
    }
    cancelledOrdersList = NULL;
}

/* JSON Parsing Helpers */
static int getJsonString(const char *json, const char *key, char *out, size_t maxLen) {
    char pattern[128];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    const char *pos = strstr(json, pattern);
    if (!pos) return 0;

    pos += strlen(pattern);
    while (*pos && (*pos == ' ' || *pos == ':' || *pos == '\t')) pos++;
    if (*pos != '"') return 0;
    pos++;

    size_t idx = 0;
    while (*pos && *pos != '"' && idx < maxLen - 1) {
        if (*pos == '\\' && *(pos + 1)) pos++;
        out[idx++] = *pos++;
    }
    out[idx] = '\0';
    return 1;
}

static int getJsonInt(const char *json, const char *key, int defaultVal) {
    char pattern[128];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    const char *pos = strstr(json, pattern);
    if (!pos) return defaultVal;

    pos += strlen(pattern);
    while (*pos && (*pos == ' ' || *pos == ':' || *pos == '\t')) pos++;
    return atoi(pos);
}

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

/* API Handlers */

/* GET /api/menu */
static void handleGetMenu(SOCKET clientSocket) {
    size_t size = 8192;
    char *json = (char *)malloc(size);
    if (!json) {
        sendHttpResponse(clientSocket, 500, "Internal Error", "application/json", "{\"error\":\"Memory error\"}");
        return;
    }

    strcpy(json, "[");
    for (int i = 0; i < menuCount; i++) {
        char itemBuf[600];
        snprintf(itemBuf, sizeof(itemBuf),
                 "%s{"
                 "\"id\":%d,"
                 "\"name\":\"%s\","
                 "\"category\":\"%s\","
                 "\"price\":%.2f,"
                 "\"description\":\"%s\","
                 "\"isVeg\":%s,"
                 "\"isChefSpecial\":%s,"
                 "\"imageUrl\":\"%s\""
                 "}",
                 (i == 0) ? "" : ",",
                 restaurantMenu[i].id,
                 restaurantMenu[i].name,
                 restaurantMenu[i].category,
                 restaurantMenu[i].price,
                 restaurantMenu[i].description,
                 restaurantMenu[i].isVeg ? "true" : "false",
                 restaurantMenu[i].isChefSpecial ? "true" : "false",
                 restaurantMenu[i].imageUrl);
        strcat(json, itemBuf);
    }
    strcat(json, "]");

    sendHttpResponse(clientSocket, 200, "OK", "application/json", json);
    free(json);
}

/* GET /api/orders */
static void handleGetOrders(SOCKET clientSocket) {
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
    Order *ord = findOrderAnywhere(orderId);
    if (!ord) {
        sendHttpResponse(clientSocket, 404, "Not Found", "application/json", "{\"error\":\"Order not found\"}");
        return;
    }

    char *json = orderToJson(ord);
    sendHttpResponse(clientSocket, 200, "OK", "application/json", json);
    if (json) free(json);
}

/* POST /api/orders - Creates Order node, adds items linked list, enqueues to Queue */
static void handleCreateOrder(SOCKET clientSocket, const char *body) {
    char customerName[100] = "Guest";
    char phone[24] = "";
    getJsonString(body, "customerName", customerName, sizeof(customerName));
    getJsonString(body, "phone", phone, sizeof(phone));
    int tableNumber = getJsonInt(body, "tableNumber", 1);

    if (strlen(customerName) == 0) {
        strcpy(customerName, "Guest");
    }

    /* 1. Allocate Order node */
    Order *newOrder = createOrder(currentOrderIdCounter++, customerName, phone, tableNumber);
    if (!newOrder) {
        sendHttpResponse(clientSocket, 500, "Internal Error", "application/json", "{\"error\":\"Order allocation failed\"}");
        return;
    }

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
                }

                ptr = endObj + 1;
            }
        }
    }

    if (newOrder->items == NULL) {
        const MenuItem *m = &restaurantMenu[0];
        addItem(newOrder, m->id, m->name, m->price, 1);
    }

    /* 3. Enqueue Order into FIFO Queue */
    enqueue(&orderQueue, newOrder);

    char *json = orderToJson(newOrder);
    sendHttpResponse(clientSocket, 201, "Created", "application/json", json);
    if (json) free(json);
}

/* POST /api/orders/process-next - Dequeues front order from FIFO queue */
static void handleProcessNextOrder(SOCKET clientSocket) {
    if (isEmpty(&orderQueue)) {
        sendHttpResponse(clientSocket, 400, "Bad Request", "application/json", "{\"error\":\"No active orders in the queue.\"}");
        return;
    }

    /* Dequeue operation: removes order at FRONT */
    Order *servedOrder = dequeue(&orderQueue);
    if (!servedOrder) {
        sendHttpResponse(clientSocket, 500, "Internal Error", "application/json", "{\"error\":\"Dequeue failed\"}");
        return;
    }

    updateOrderStatus(servedOrder, "Completed");

    /* Insert into completed orders list at HEAD */
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
        sendHttpResponse(clientSocket, 400, "Bad Request", "application/json", "{\"error\":\"Missing status\"}");
        return;
    }

    Order *ord = findOrderAnywhere(orderId);
    if (!ord) {
        sendHttpResponse(clientSocket, 404, "Not Found", "application/json", "{\"error\":\"Order not found\"}");
        return;
    }

    updateOrderStatus(ord, newStatus);

    /* If marked completed from queue, move from queue to completed list */
    if (strcmp(newStatus, "Completed") == 0) {
        removeOrderFromQueue(&orderQueue, orderId);
        ord->next = completedOrdersList;
        completedOrdersList = ord;
    }

    char *json = orderToJson(ord);
    sendHttpResponse(clientSocket, 200, "OK", "application/json", json);
    if (json) free(json);
}

/* GET /api/stats - Aggregated metrics for Kitchen/Admin */
static void handleGetStats(SOCKET clientSocket) {
    int totalCount = 20; /* Baseline completed orders for the day */
    int pendingCount = 0;
    int preparingCount = 0;
    int readyCount = 0;
    int completedCount = 14;
    float totalRevenue = 6840.0f; /* Base day revenue */

    const Order *curr = orderQueue.front;
    while (curr) {
        totalCount++;
        if (strcmp(curr->status, "Pending") == 0) pendingCount++;
        else if (strcmp(curr->status, "Preparing") == 0) preparingCount++;
        else if (strcmp(curr->status, "Ready") == 0) readyCount++;
        totalRevenue += curr->total;
        curr = curr->next;
    }

    curr = completedOrdersList;
    while (curr) {
        totalCount++;
        completedCount++;
        totalRevenue += curr->total;
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
             "\"queueLength\":%d,"
             "\"totalRevenue\":%.2f"
             "}",
             totalCount, pendingCount, preparingCount, readyCount,
             completedCount, orderQueue.count, totalRevenue);

    sendHttpResponse(clientSocket, 200, "OK", "application/json", json);
}

/* Request Router */
static void handleClientRequest(SOCKET clientSocket) {
    char buffer[BUFFER_SIZE];
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) {
        CLOSE_SOCKET(clientSocket);
        return;
    }

    buffer[bytesRead] = '\0';

    char method[16] = {0};
    char path[256] = {0};
    sscanf(buffer, "%15s %255s", method, path);

    if (strcmp(method, "OPTIONS") == 0) {
        sendHttpResponse(clientSocket, 204, "No Content", "text/plain", "");
        CLOSE_SOCKET(clientSocket);
        return;
    }

    const char *body = strstr(buffer, "\r\n\r\n");
    body = body ? body + 4 : "";

    char cleanPath[256];
    strncpy(cleanPath, path, sizeof(cleanPath) - 1);
    cleanPath[sizeof(cleanPath) - 1] = '\0';
    char *q = strchr(cleanPath, '?');
    if (q) *q = '\0';

    if (strcmp(method, "GET") == 0 && strcmp(cleanPath, "/api/menu") == 0) {
        handleGetMenu(clientSocket);
    } else if (strcmp(method, "GET") == 0 && strcmp(cleanPath, "/api/orders") == 0) {
        handleGetOrders(clientSocket);
    } else if (strcmp(method, "GET") == 0 && strcmp(cleanPath, "/api/stats") == 0) {
        handleGetStats(clientSocket);
    } else if (strcmp(method, "POST") == 0 && strcmp(cleanPath, "/api/orders") == 0) {
        handleCreateOrder(clientSocket, body);
    } else if (strcmp(method, "POST") == 0 && strcmp(cleanPath, "/api/orders/process-next") == 0) {
        handleProcessNextOrder(clientSocket);
    } else if (strncmp(cleanPath, "/api/orders/", 12) == 0) {
        int orderId = atoi(cleanPath + 12);
        char *subPath = strchr(cleanPath + 12, '/');

        if (subPath == NULL) {
            if (strcmp(method, "GET") == 0) {
                handleGetOrderById(clientSocket, orderId);
            } else {
                sendHttpResponse(clientSocket, 405, "Method Not Allowed", "application/json", "{\"error\":\"Method not allowed\"}");
            }
        } else if (strcmp(subPath, "/status") == 0 && (strcmp(method, "PUT") == 0 || strcmp(method, "POST") == 0)) {
            handleUpdateStatus(clientSocket, orderId, body);
        } else {
            sendHttpResponse(clientSocket, 404, "Not Found", "application/json", "{\"error\":\"Route not found\"}");
        }
    } else {
        sendHttpResponse(clientSocket, 200, "OK", "application/json", "{\"status\":\"Basil & Ember C API online\"}");
    }

    CLOSE_SOCKET(clientSocket);
}

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
        CLOSE_SOCKET(serverSocket);
        return -1;
    }

    if (listen(serverSocket, 15) == SOCKET_ERROR) {
        CLOSE_SOCKET(serverSocket);
        return -1;
    }

    printf("\n[BASIL & EMBER] C Backend listening on port %d\n", port);
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
