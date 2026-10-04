# 🍽️ Restaurant Order Management System
### Data Structures Mini Project — Practical Application of FIFO Queues & Singly Linked Lists

A complete, fully functional, real-world web application that demonstrates the practical use of **Queues** and **Linked Lists** in a modern restaurant ordering environment.

Unlike typical academic demos that simulate structures in JavaScript arrays, this project features a **real, native backend written entirely in C**. The C server manually manages dynamic memory (`malloc`/`free`), implements pointer-based FIFO Queue operations for orders, and links ordered food items into singly linked lists.

---

## 📋 Table of Contents
1. [Project Objective](#-project-objective)
2. [Key Features](#-key-features)
3. [Technologies Used](#-technologies-used)
4. [Data Structures Implementation](#-data-structures-implementation)
   - [FIFO Order Queue](#1-fifo-order-queue)
   - [Order Items Singly Linked List](#2-order-items-singly-linked-list)
5. [How Frontend Communicates with C Backend](#-how-frontend-communicates-with-c-backend)
6. [Windows Setup & Compilation Guide (GCC / MSYS2)](#-windows-setup--compilation-guide-gcc--msys2)
7. [Linux / macOS Compilation Guide](#-linux--macos-compilation-guide)
8. [Sample API Endpoints](#-sample-api-endpoints)
9. [Project Directory Structure](#-project-directory-structure)

---

## 🎯 Project Objective
The primary objective of this project is to showcase how fundamental computer science data structures solve real-world operational problems:
- **Fair Customer Service:** Kitchen staff prepare dishes in the order received using a **First-In, First-Out (FIFO) Queue**.
- **Dynamic Order Size:** Customers can order any combination and quantity of dishes without memory wastage using **Singly Linked Lists** with dynamic heap allocation.
- **Visual Memory Proof:** The application inspects the C server's heap memory and renders the actual pointer addresses (`0x558689...`), `HEAD`, `FRONT`, `REAR`, and `next` connections directly on the web browser.

---

## ✨ Key Features
1. **Interactive Customer Order Menu:**
   - Multi-category food catalog (Biriyani, Starters, Main Course, South Indian, Snacks, Beverages, Breads, Desserts).
   - Diet badges (Vegetarian green indicator / Non-Vegetarian red indicator).
   - Dynamic quantity selector and interactive cart.
   - Table number selection and instant order submission.

2. **Real-time Order Queue Dashboard:**
   - **Now Serving:** High-visibility hero card displaying the order at the `FRONT` of the queue.
   - **Up Next:** Sequenced cards representing waiting orders linked behind in the queue.
   - **Process Next Order:** Calls `dequeue()` on the C backend to advance the queue and mark the order as Completed.

3. **Data Structure Visualizer (Star Feature):**
   - Graphical canvas showing the Order Queue with live `FRONT` and `REAR` pointer flags.
   - Hexadecimal pointer memory addresses for each order node and next links (`next -> 0x...`).
   - Terminal node pointing to `NULL`.
   - **Linked List Inspector:** Select any order to view its individual food items arranged as `HEAD -> [Item 1] -> [Item 2] -> NULL`.
   - **Interactive Sandbox:** One-click `enqueue()`, `dequeue()`, and `peek()` operations.

4. **Kitchen / Admin Dashboard:**
   - Real-time aggregate metrics: Total Orders, Pending, Preparing, Ready, Completed, and Total Revenue.
   - Kanban ticket columns: Pending → Preparing → Ready → Completed.
   - Instant search by Order ID, Customer Name, or Table Number.

5. **Bill Receipt Generator:**
   - Professional restaurant receipt with subtotal, 5% GST breakdown, table info, and grand total.
   - One-click receipt printing (`window.print()`) and downloadable text format (`.txt`).

6. **C Backend Console:**
   - Live terminal log streaming stdout events directly from the C server process (showing `malloc`, `free`, and socket activity).

---

## 🛠️ Technologies Used
- **Frontend:**
  - HTML5 (Semantic structure, accessible forms)
  - CSS3 (Responsive grid, modern dark theme, CSS variables)
  - Vanilla JavaScript (ES6+ `fetch()` API, event delegation, zero frameworks)
- **Backend:**
  - C Programming Language (C99 standard)
  - Sockets: POSIX BSD Sockets on Linux/macOS, WinSock2 (`ws2_32`) on Windows
  - Custom RESTful HTTP/1.1 API routing & JSON serializer
- **Tooling:**
  - GCC (GNU Compiler Collection) / MSYS2 / MinGW-w64
  - GNU Make

---

## 🧠 Data Structures Implementation

### 1. FIFO Order Queue
The FIFO queue ensures that orders are prepared strictly in the order they arrived.

#### Node & Queue Structure:
```c
/* Item Linked List Node */
typedef struct Item {
    int itemId;
    char name[100];
    float price;
    int quantity;
    struct Item *next;
} Item;

/* Order Queue Node */
typedef struct Order {
    int orderId;
    char customerName[100];
    int tableNumber;
    Item *items;        /* HEAD pointer to singly linked list of items */
    float total;
    char status[30];    /* "Pending", "Preparing", "Ready", "Completed" */
    char createdAt[32];
    struct Order *next; /* Pointer to next order in Queue */
} Order;

/* FIFO Queue Controller */
typedef struct {
    Order *front;       /* Points to the oldest order (next to be served) */
    Order *rear;        /* Points to the newest order */
    int count;          /* Current size of queue */
} OrderQueue;
```

#### Core Queue Operations:
- `enqueue(OrderQueue *q, Order *newOrder)`:
  - If queue is empty: `q->front = newOrder; q->rear = newOrder;`
  - Else: `q->rear->next = newOrder; q->rear = newOrder;`
  - Time Complexity: **O(1)**
- `dequeue(OrderQueue *q)`:
  - Extracts `q->front`.
  - Advances `q->front = q->front->next`.
  - If `q->front == NULL`, sets `q->rear = NULL`.
  - Time Complexity: **O(1)**
- `peek(const OrderQueue *q)`:
  - Returns `q->front` without removing it.
  - Time Complexity: **O(1)**
- `isEmpty(const OrderQueue *q)`:
  - Returns `q->front == NULL`.
  - Time Complexity: **O(1)**

---

### 2. Order Items Singly Linked List
Each order contains a dynamic list of items. An array would waste memory for small orders or overflow for large orders; a singly linked list allocates memory dynamically on demand.

#### Linked List Operations:
- `createItem(itemId, name, price, quantity)`:
  - Calls `malloc(sizeof(Item))` in heap memory.
  - Initializes `next = NULL`.
- `addItem(Order *order, ...)`:
  - Appends item to the tail of `order->items` list or increments quantity if item already exists.
  - Recalculates `order->total` by traversing the list.
- `removeItem(Order *order, itemId)`:
  - Locates node, updates pointer `prev->next = curr->next`, and calls `free(curr)`.
- `calculateTotal(const Order *order)`:
  - Traverses `HEAD -> ... -> NULL` and computes $\sum (price \times quantity)$.
- `freeItemList(Item *head)`:
  - Traverses and frees all nodes in the linked list using `free()`.

---

## 🌐 How Frontend Communicates with C Backend
```
+--------------------------+                 +-----------------------------+
|    Frontend (Browser)    |                 |      C Backend (Server)     |
|   HTML5 / CSS3 / JS      |                 |    Queue & Linked Lists     |
+--------------------------+                 +-----------------------------+
             |                                              |
             |  1. POST /api/orders (JSON)                  |
             |--------------------------------------------->|  malloc(Order)
             |                                              |  addItem() -> Linked List
             |                                              |  enqueue() -> Queue REAR
             |  2. HTTP 201 Created (Order with Pointers)   |
             |<---------------------------------------------|
             |                                              |
             |  3. POST /api/orders/process-next            |
             |--------------------------------------------->|  dequeue() -> From FRONT
             |                                              |  Update status = Completed
             |  4. HTTP 200 OK (Served Order JSON)          |
             |<---------------------------------------------|
             |                                              |
             |  5. GET /api/queue/visualize                 |
             |--------------------------------------------->|  Traverse front -> rear
             |  6. JSON with 0x... memory pointers          |  Traverse item linked list
             |<---------------------------------------------|
```

The frontend uses standard `window.fetch()` with JSON payloads (`Content-Type: application/json`). The C backend parses the HTTP method and request body, executes the data structure operation in memory, and responds with JSON strings with complete CORS headers enabled (`Access-Control-Allow-Origin: *`).

---

## 🪟 Windows Setup & Compilation Guide (GCC / MSYS2)

### Prerequisites on Windows
1. Install **MSYS2** from [https://www.msys2.org/](https://www.msys2.org/) or install **MinGW-w64**.
2. Open MSYS2 MinGW 64-bit terminal and install GCC & Make:
   ```bash
   pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make
   ```
3. Add `C:\msys64\mingw64\bin` to your Windows System PATH.

### 1. Compile the C Backend on Windows
Open Command Prompt, PowerShell, or MSYS2 terminal:
```bash
cd restaurant-order-management/backend
gcc -Wall -Wextra -O2 -std=c99 main.c server.c queue.c linkedlist.c order.c -o server.exe -lws2_32
```
*(Notice the `-lws2_32` flag which links Windows Sockets library)*

### 2. Start the C Server
```bash
./server.exe 8080
```
You will see:
```text
==========================================================
  RESTAURANT ORDER MANAGEMENT SYSTEM - C BACKEND SERVER  
==========================================================
  [STATUS] HTTP Server running on port 8080
  [DATA STRUCTURES] FIFO Queue & Singly Linked Lists Active
  [READY] Listening for incoming client API requests...
==========================================================
```

### 3. Open the Frontend
Simply open `restaurant-order-management/frontend/index.html` in your web browser (Chrome, Edge, Firefox), or serve it using any HTTP server:
```bash
# Optional: run a local static server
npx serve restaurant-order-management/frontend
```

---

## 🐧 Linux / macOS Compilation Guide

### 1. Compile with Make
```bash
cd restaurant-order-management/backend
make
```

### 2. Run the Server
```bash
./server 8080
```

### 3. Open the Website
Open `restaurant-order-management/frontend/index.html` in your browser.

---

## 🧪 Sample API Endpoints

You can test the C server using `curl`, Postman, or your browser:

### 1. Get Food Menu
```bash
curl -X GET http://localhost:8080/api/menu
```

### 2. Get All Orders (Queue + History)
```bash
curl -X GET http://localhost:8080/api/orders
```

### 3. Enqueue a New Order
```bash
curl -X POST http://localhost:8080/api/orders \
  -H "Content-Type: application/json" \
  -d '{
    "customerName": "Deepak Kumar",
    "tableNumber": 9,
    "items": [
      {"itemId": 1, "quantity": 1},
      {"itemId": 3, "quantity": 2},
      {"itemId": 9, "quantity": 2}
    ]
  }'
```

### 4. Process Next Order (FIFO Dequeue)
```bash
curl -X POST http://localhost:8080/api/orders/process-next
```

### 5. Inspect Queue & Pointer Memory Addresses
```bash
curl -X GET http://localhost:8080/api/queue/visualize
```

### 6. Get Aggregated Kitchen Metrics
```bash
curl -X GET http://localhost:8080/api/stats
```

---

## 📁 Project Directory Structure
```
restaurant-order-management/
│
├── frontend/
│   ├── index.html       # Single Page Application layout
│   ├── style.css        # Modern, responsive dark UI stylesheet
│   └── script.js        # Vanilla JS client communicating with C API
│
├── backend/
│   ├── main.c           # Entry point and signal handling
│   ├── server.h         # HTTP server header and MenuItem struct
│   ├── server.c         # Cross-platform socket server & REST routing
│   ├── queue.h          # FIFO Queue interface (front, rear)
│   ├── queue.c          # enqueue(), dequeue(), peek(), isEmpty()
│   ├── linkedlist.h     # Singly linked list interface
│   ├── linkedlist.c     # createItem(), addItem(), calculateTotal(), freeItemList()
│   ├── order.h          # Order & Item struct definitions
│   ├── order.c          # Order creation, status updates, JSON serialization
│   └── Makefile         # Build configuration for GCC (Linux & Windows)
│
├── README.md            # Complete project documentation and guide
└── screenshots/         # Mockups and architectural diagrams
```

---

## 🎓 Viva & Presentation Talking Points
- **Why Queue for Orders?**
  Food preparation follows fairness. A FIFO queue guarantees that orders placed first are cooked first. Adding at the rear is $O(1)$ and removing from front is $O(1)$.
- **Why Singly Linked List for Items?**
  A table can order 1 item (a coffee) or 20 items (a feast). Arrays require fixed capacity or dynamic reallocation copying. A linked list allocates exact memory per dish using `malloc()`, saving memory and allowing dynamic insertions.
- **Why is this project unique?**
  Most college projects fake backend data with JavaScript arrays. This application has an actual compiled C backend listening on TCP sockets, making system calls to allocate memory, and calculating totals directly in C.
