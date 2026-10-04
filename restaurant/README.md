# 🌿 Basil & Ember — Restaurant Order Management System
### "Good food. Warm moments."
**A Production-Style Restaurant Website Powered by a Native C Backend (FIFO Queue & Singly Linked List)**

---

## 📖 Overview
**Basil & Ember** is a full-featured, modern restaurant online ordering web application. Designed to deliver an authentic customer dining and ordering experience, the client-facing website looks and behaves like an upscale contemporary restaurant.

Behind the scenes, the entire order lifecycle and inventory state are processed by an authentic **native C backend**. The C server manages:
1. **FIFO Order Queue (`queue.c`, `queue.h`):** Incoming customer orders are enqueued at the `REAR` and dequeued from the `FRONT` to enforce fair, first-come first-served food preparation in the kitchen.
2. **Singly Linked List (`linkedlist.c`, `linkedlist.h`):** Each customer order stores its individual food items dynamically as heap-allocated nodes (`malloc` / `free`), accommodating orders of any size with zero memory wastage.

---

## 🍽️ Customer & Staff Features
- **Hero & Landing Page:** Immersive culinary imagery, restaurant introduction, Chef's signature highlights, and guest reviews.
- **Interactive Menu:** Filter by category (Starters, Main Course, Rice & Biriyani, Breads, Desserts, Beverages), pure vegetarian filter toggle, instant search, and diet badges.
- **Slide-Out Cart:** Live quantity adjustments, automatic 5% GST calculation, and one-click checkout progression.
- **Checkout & Validation:** Customer full name, 10-digit phone number, and table number (1–50) selection.
- **Live Order Tracking:** Real-time milestone stepper (*Order Confirmed* &rarr; *Kitchen Preparing* &rarr; *Ready for Service* &rarr; *Served &amp; Completed*) polling the live order status from the C server.
- **Kitchen & Admin Portal (`/admin`):** Real-time order cards sorted in strict FIFO priority, summary metrics (Total Orders, In Preparation, Ready, Completed, Today's Revenue), and status update triggers (`Start Preparing`, `Mark Ready`, `Complete`).

---

## 🧠 Backend Data Structures (C Architecture)

### 1. FIFO Queue for Customer Orders
```c
typedef struct {
    Order *front;  /* Earliest unserved order in line */
    Order *rear;   /* Most recently placed order */
    int count;     /* Total waiting orders in queue */
} OrderQueue;
```
- **`enqueue(OrderQueue *q, Order *newOrder)` ($O(1)$):**
  Appends the newly created order node to the `REAR`.
- **`dequeue(OrderQueue *q)` ($O(1)$):**
  Removes the front order node when kitchen staff dispatches it, advancing `front = front->next`.
- **`peek(OrderQueue *q)` ($O(1)$):**
  Inspects current order in line without mutating pointers.

### 2. Singly Linked List for Order Food Items
```c
typedef struct Item {
    int itemId;
    char name[100];
    float price;
    int quantity;
    struct Item *next;  /* Pointer to next dish in the order */
} Item;
```
- **`addItem(Order *order, int itemId, ...)` ($O(N)$):**
  Allocates a new `Item` node dynamically using `malloc()`, appending it to the order's `items` linked list.
- **`calculateTotal(const Order *order)` ($O(N)$):**
  Traverses the item linked list from `HEAD` to `NULL` to compute cumulative bill amount: $\sum (price \times quantity)$.
- **`freeItemList(Item *head)`:**
  Traverses and frees each node via `free()` to prevent memory leaks.

---

## 🚀 Running on Windows (GCC / MSYS2)

1. Open MSYS2 MinGW 64-bit or Command Prompt with GCC in your PATH:
   ```bash
   cd restaurant/backend
   gcc -Wall -Wextra -O2 -std=c99 main.c server.c queue.c linkedlist.c order.c -o server.exe -lws2_32
   ```
2. Start the C API Server:
   ```bash
   ./server.exe 5050
   ```
3. Open `restaurant/frontend/index.html` in Chrome or Edge.

---

## 🐧 Running on Linux / macOS

1. Compile with GNU Make:
   ```bash
   cd restaurant/backend
   make
   ```
2. Start the C Server:
   ```bash
   ./server 5050
   ```
3. Open `restaurant/frontend/index.html` in your browser.

---

## 📡 RESTful API Endpoints
- `GET  /api/menu` &mdash; Returns complete restaurant menu items with prices, descriptions, and categories.
- `GET  /api/orders` &mdash; Returns active orders in the queue and completed order history.
- `GET  /api/orders/:id` &mdash; Returns single order details and its item linked list.
- `POST /api/orders` &mdash; Creates an Order node, constructs its item linked list, and enqueues to FIFO queue.
- `PUT  /api/orders/:id/status` &mdash; Updates order status (e.g. `Preparing`, `Ready`, `Completed`).
- `POST /api/orders/process-next` &mdash; Dequeues the order at the FRONT of the queue and marks it Completed.
- `GET  /api/stats` &mdash; Returns kitchen analytics (total orders, active orders, today's revenue).

---

## 📁 Directory Structure
```
restaurant/
│
├── frontend/
│   ├── index.html       # Single-Page Application (Home, Menu, Checkout, Tracking, Admin)
│   ├── menu.html        # Direct menu entry
│   ├── checkout.html    # Direct checkout entry
│   ├── tracking.html    # Direct tracking entry
│   ├── admin.html       # Kitchen staff portal entry
│   ├── style.css        # Basil & Ember luxury restaurant design system
│   └── script.js        # Dynamic cart, routing, and fetch client
│
├── backend/
│   ├── main.c           # Entry point and signal handling
│   ├── server.h         # HTTP server header and MenuItem struct
│   ├── server.c         # Cross-platform socket server & REST routing
│   ├── queue.h          # FIFO Queue interface (front, rear)
│   ├── queue.c          # enqueue(), dequeue(), peek(), isEmpty()
│   ├── linkedlist.h     # Singly linked list interface
│   ├── linkedlist.c     # addItem(), calculateTotal(), freeItemList()
│   ├── order.h          # Order & Item struct definitions
│   ├── order.c          # Order creation and JSON serialization
│   └── Makefile         # GCC build script for Linux and Windows
│
└── README.md
```
