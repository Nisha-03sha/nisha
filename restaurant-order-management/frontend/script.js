/**
 * ============================================================================
 * RESTAURANT ORDER MANAGEMENT SYSTEM - VANILLA JAVASCRIPT
 * Data Structures Mini Project: FIFO Queue & Singly Linked List
 * Direct communication with C Backend via RESTful HTTP APIs
 * ============================================================================
 */

/* Application State */
const state = {
  menu: [],
  cart: [],
  queueOrders: [],
  completedOrders: [],
  cancelledOrders: [],
  stats: {
    totalOrders: 0,
    pendingOrders: 0,
    preparingOrders: 0,
    readyOrders: 0,
    completedOrders: 0,
    cancelledOrders: 0,
    queueLength: 0,
    totalRevenue: 0
  },
  visualization: null,
  logs: [],
  selectedVisOrder: null,
  activeCategory: 'all',
  searchQuery: '',
  cServerOnline: false
};

/* API Base URL - relative for standard proxy or direct C server */
const API_BASE = '/api';

/* DOM Elements Cache */
const elements = {};

// Wait for DOM
document.addEventListener('DOMContentLoaded', () => {
  initElements();
  setupNavigation();
  setupEventListeners();
  loadInitialData();

  // Polling for real-time queue updates every 4 seconds
  setInterval(refreshQueueAndStats, 4000);
});

function initElements() {
  elements.navButtons = document.querySelectorAll('.nav-btn');
  elements.pageSections = document.querySelectorAll('.page-section');
  elements.menuGrid = document.getElementById('menu-grid');
  elements.categoryTabs = document.getElementById('category-tabs');
  elements.cartItemsContainer = document.getElementById('cart-items-container');
  elements.cartTotalAmount = document.getElementById('cart-total-amount');
  elements.cartBadgeCount = document.getElementById('cart-badge-count');
  elements.navCartCounter = document.getElementById('nav-cart-counter');
  elements.navQueueCounter = document.getElementById('nav-queue-counter');
  elements.customerNameInput = document.getElementById('customer-name-input');
  elements.tableNumberInput = document.getElementById('table-number-input');
  elements.placeOrderBtn = document.getElementById('place-order-btn');
  elements.clearCartBtn = document.getElementById('clear-cart-btn');

  // Queue Dashboard Elements
  elements.nowServingContainer = document.getElementById('now-serving-container');
  elements.upNextContainer = document.getElementById('up-next-container');
  elements.upNextCount = document.getElementById('up-next-count');
  elements.processNextBtn = document.getElementById('process-next-btn');

  // Kitchen Dashboard Elements
  elements.statTotalOrders = document.getElementById('stat-total-orders');
  elements.statPendingOrders = document.getElementById('stat-pending-orders');
  elements.statPreparingOrders = document.getElementById('stat-preparing-orders');
  elements.statCompletedOrders = document.getElementById('stat-completed-orders');
  elements.statTotalRevenue = document.getElementById('stat-total-revenue');
  elements.kitchenSearchInput = document.getElementById('kitchen-search-input');
  elements.colPending = document.getElementById('tickets-pending');
  elements.colPreparing = document.getElementById('tickets-preparing');
  elements.colReady = document.getElementById('tickets-ready');
  elements.colCompleted = document.getElementById('tickets-completed');

  // DS Visualization Elements
  elements.dsQueueFlow = document.getElementById('ds-queue-flow');
  elements.dsFrontPtr = document.getElementById('ds-front-ptr');
  elements.dsRearPtr = document.getElementById('ds-rear-ptr');
  elements.dsQueueCount = document.getElementById('ds-queue-count');
  elements.dsLinkedListInspector = document.getElementById('ds-linked-list-inspector');
  elements.dsSelectedOrderBadge = document.getElementById('ds-selected-order-badge');
  elements.dsLlHeadPtr = document.getElementById('ds-ll-head-ptr');
  elements.dsLlNodesChain = document.getElementById('ds-ll-nodes-chain');

  // Console Elements
  elements.cConsoleBody = document.getElementById('c-console-body');
  elements.refreshLogsBtn = document.getElementById('refresh-logs-btn');

  // Modal Elements
  elements.modalOverlay = document.getElementById('modal-overlay');
  elements.modalBody = document.getElementById('modal-body');
  elements.modalCloseBtn = document.getElementById('modal-close-btn');

  // Toast container
  elements.toastContainer = document.getElementById('toast-container');
}

/* Tab Navigation */
function setupNavigation() {
  elements.navButtons.forEach(btn => {
    btn.addEventListener('click', () => {
      const targetId = btn.getAttribute('data-target');
      switchSection(targetId);
    });
  });
}

function switchSection(sectionId) {
  elements.navButtons.forEach(b => {
    b.classList.toggle('active', b.getAttribute('data-target') === sectionId);
  });
  elements.pageSections.forEach(sec => {
    sec.classList.toggle('active', sec.id === sectionId);
  });

  // Section-specific refresh
  if (sectionId === 'section-visualization') {
    renderVisualization();
  } else if (sectionId === 'section-console') {
    fetchLogs();
  } else if (sectionId === 'section-kitchen') {
    renderKitchenDashboard();
  } else if (sectionId === 'section-queue') {
    renderQueueDashboard();
  }
}

/* Event Listeners */
function setupEventListeners() {
  if (elements.placeOrderBtn) {
    elements.placeOrderBtn.addEventListener('click', handlePlaceOrder);
  }

  if (elements.clearCartBtn) {
    elements.clearCartBtn.addEventListener('click', clearCart);
  }

  if (elements.processNextBtn) {
    elements.processNextBtn.addEventListener('click', handleProcessNextOrder);
  }

  if (elements.kitchenSearchInput) {
    elements.kitchenSearchInput.addEventListener('input', (e) => {
      state.searchQuery = e.target.value.toLowerCase().trim();
      renderKitchenDashboard();
    });
  }

  if (elements.refreshLogsBtn) {
    elements.refreshLogsBtn.addEventListener('click', fetchLogs);
  }

  if (elements.modalCloseBtn) {
    elements.modalCloseBtn.addEventListener('click', closeModal);
  }

  if (elements.modalOverlay) {
    elements.modalOverlay.addEventListener('click', (e) => {
      if (e.target === elements.modalOverlay) closeModal();
    });
  }
}

/* Initial Data Fetch */
async function loadInitialData() {
  try {
    await fetchMenu();
    await refreshQueueAndStats();
    showToast('Connected to C Backend Server!', 'success');
  } catch (err) {
    console.warn('Initial load error:', err);
    showToast('Connecting to C backend...', 'info');
  }
}

/* Background Refresh */
async function refreshQueueAndStats() {
  await Promise.allSettled([
    fetchOrders(),
    fetchStats(),
    fetchVisualization()
  ]);
}

/* ==========================================================================
   API CALLS (C BACKEND HTTP REST CLIENT)
   ========================================================================== */

/* GET /api/menu */
async function fetchMenu() {
  try {
    const res = await fetch(`${API_BASE}/menu`);
    if (!res.ok) throw new Error('Failed to fetch menu');
    state.menu = await res.json();
    renderCategoryTabs();
    renderMenuGrid();
  } catch (err) {
    console.error('Menu fetch error:', err);
  }
}

/* GET /api/orders */
async function fetchOrders() {
  try {
    const res = await fetch(`${API_BASE}/orders`);
    if (!res.ok) throw new Error('Failed to fetch orders');
    const data = await res.json();
    state.queueOrders = data.queue || [];
    state.completedOrders = data.completed || [];
    state.cancelledOrders = data.cancelled || [];

    updateQueueCounters();
    renderQueueDashboard();
    renderKitchenDashboard();
  } catch (err) {
    console.error('Orders fetch error:', err);
  }
}

/* GET /api/stats */
async function fetchStats() {
  try {
    const res = await fetch(`${API_BASE}/stats`);
    if (!res.ok) throw new Error('Failed to fetch stats');
    state.stats = await res.json();
    renderStats();
  } catch (err) {
    console.error('Stats fetch error:', err);
  }
}

/* GET /api/queue/visualize */
async function fetchVisualization() {
  try {
    const res = await fetch(`${API_BASE}/queue/visualize`);
    if (!res.ok) throw new Error('Failed to fetch visualization');
    state.visualization = await res.json();
    renderVisualization();
  } catch (err) {
    console.error('Visualization fetch error:', err);
  }
}

/* GET /api/logs */
async function fetchLogs() {
  try {
    const res = await fetch(`${API_BASE}/logs`);
    if (!res.ok) throw new Error('Failed to fetch logs');
    state.logs = await res.json();
    renderConsoleLogs();
  } catch (err) {
    console.error('Logs fetch error:', err);
  }
}

/* POST /api/orders - Enqueues new order in C backend */
async function handlePlaceOrder() {
  if (state.cart.length === 0) {
    showToast('Your cart is empty! Add items from the menu.', 'danger');
    return;
  }

  const customerName = elements.customerNameInput.value.trim() || 'Guest';
  const tableNumber = parseInt(elements.tableNumberInput.value, 10);

  if (isNaN(tableNumber) || tableNumber <= 0 || tableNumber > 50) {
    showToast('Please enter a valid table number (1 - 50)', 'danger');
    elements.tableNumberInput.focus();
    return;
  }

  const payload = {
    customerName,
    tableNumber,
    items: state.cart.map(item => ({
      itemId: item.menuItem.id,
      quantity: item.quantity
    }))
  };

  try {
    elements.placeOrderBtn.disabled = true;
    elements.placeOrderBtn.innerHTML = 'Enqueuing in C Backend...';

    const res = await fetch(`${API_BASE}/orders`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(payload)
    });

    if (!res.ok) {
      const errData = await res.json();
      throw new Error(errData.error || 'Server rejected order');
    }

    const createdOrder = await res.json();
    showToast(`Order #${createdOrder.orderId} Enqueued at REAR!`, 'success');

    // Reset cart
    state.cart = [];
    renderCart();
    elements.customerNameInput.value = '';

    // Immediately refresh orders and visualization
    await refreshQueueAndStats();

    // Switch to Queue or Visualization to demonstrate FIFO
    switchSection('section-queue');
  } catch (err) {
    console.error('Order creation failed:', err);
    showToast(`Failed to place order: ${err.message}`, 'danger');
  } finally {
    elements.placeOrderBtn.disabled = false;
    elements.placeOrderBtn.innerHTML = '<span>⚡ Place Order (Enqueue)</span>';
  }
}

/* POST /api/orders/process-next - Dequeues front order from C backend */
async function handleProcessNextOrder() {
  try {
    elements.processNextBtn.disabled = true;
    elements.processNextBtn.innerHTML = 'Processing Dequeue...';

    const res = await fetch(`${API_BASE}/orders/process-next`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' }
    });

    if (!res.ok) {
      const err = await res.json();
      throw new Error(err.error || 'Failed to process order');
    }

    const servedOrder = await res.json();
    showToast(`Order #${servedOrder.orderId} Dequeued from FRONT and Completed!`, 'success');

    await refreshQueueAndStats();
  } catch (err) {
    showToast(err.message, 'danger');
  } finally {
    elements.processNextBtn.disabled = false;
    elements.processNextBtn.innerHTML = '<span>✓ Process Next Order (Dequeue)</span>';
  }
}

/* PUT /api/orders/:id/status */
async function updateOrderStatus(orderId, newStatus) {
  try {
    const res = await fetch(`${API_BASE}/orders/${orderId}/status`, {
      method: 'PUT',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ status: newStatus })
    });
    if (!res.ok) throw new Error('Failed to update status');
    showToast(`Order #${orderId} marked as ${newStatus}`, 'info');
    await refreshQueueAndStats();
  } catch (err) {
    showToast(err.message, 'danger');
  }
}

/* DELETE /api/orders/:id - Cancel order */
async function cancelOrder(orderId) {
  if (!confirm(`Are you sure you want to cancel Order #${orderId}?`)) return;

  try {
    const res = await fetch(`${API_BASE}/orders/${orderId}`, {
      method: 'DELETE'
    });
    if (!res.ok) throw new Error('Failed to cancel order');
    showToast(`Order #${orderId} cancelled`, 'info');
    await refreshQueueAndStats();
  } catch (err) {
    showToast(err.message, 'danger');
  }
}

/* ==========================================================================
   RENDER FUNCTIONS
   ========================================================================== */

/* Render Menu Category Tabs */
function renderCategoryTabs() {
  if (!elements.categoryTabs) return;

  const categories = ['all', ...new Set(state.menu.map(m => m.category))];
  elements.categoryTabs.innerHTML = categories.map(cat => `
    <button class="category-tab ${state.activeCategory === cat ? 'active' : ''}" data-cat="${cat}">
      ${cat === 'all' ? '🍽️ All Menu' : cat}
    </button>
  `).join('');

  elements.categoryTabs.querySelectorAll('.category-tab').forEach(tab => {
    tab.addEventListener('click', () => {
      state.activeCategory = tab.getAttribute('data-cat');
      renderCategoryTabs();
      renderMenuGrid();
    });
  });
}

/* Render Food Menu Cards */
function renderMenuGrid() {
  if (!elements.menuGrid) return;

  const filtered = state.activeCategory === 'all'
    ? state.menu
    : state.menu.filter(m => m.category === state.activeCategory);

  elements.menuGrid.innerHTML = filtered.map(item => {
    const inCart = state.cart.find(c => c.menuItem.id === item.id);
    const qty = inCart ? inCart.quantity : 0;

    return `
      <div class="food-card" data-id="${item.id}">
        <div>
          <div class="food-card-top">
            <div class="food-icon-box">${item.icon || '🍛'}</div>
            <span class="diet-badge ${item.isVeg ? 'veg' : 'non-veg'}" title="${item.isVeg ? 'Vegetarian' : 'Non-Vegetarian'}"></span>
          </div>
          <div class="food-info">
            <span class="food-category-label">${item.category}</span>
            <h3>${item.name}</h3>
            <p class="food-desc">${item.description}</p>
          </div>
        </div>

        <div class="food-card-bottom">
          <div class="food-price"><span>₹</span>${item.price.toFixed(2)}</div>
          ${qty === 0 ? `
            <button class="btn btn-primary btn-sm add-to-cart-btn" onclick="addToCart(${item.id})">
              + Add
            </button>
          ` : `
            <div class="quantity-controller">
              <button class="qty-btn" onclick="updateCartQuantity(${item.id}, -1)">-</button>
              <span class="qty-display">${qty}</span>
              <button class="qty-btn" onclick="updateCartQuantity(${item.id}, 1)">+</button>
            </div>
          `}
        </div>
      </div>
    `;
  }).join('');
}

/* ==========================================================================
   CART OPERATIONS
   ========================================================================== */
window.addToCart = function(itemId) {
  const item = state.menu.find(m => m.id === itemId);
  if (!item) return;

  const existing = state.cart.find(c => c.menuItem.id === itemId);
  if (existing) {
    existing.quantity += 1;
  } else {
    state.cart.push({ menuItem: item, quantity: 1 });
  }

  renderCart();
  renderMenuGrid();
};

window.updateCartQuantity = function(itemId, delta) {
  const idx = state.cart.findIndex(c => c.menuItem.id === itemId);
  if (idx === -1) return;

  state.cart[idx].quantity += delta;
  if (state.cart[idx].quantity <= 0) {
    state.cart.splice(idx, 1);
  }

  renderCart();
  renderMenuGrid();
};

function clearCart() {
  state.cart = [];
  renderCart();
  renderMenuGrid();
}

function renderCart() {
  if (!elements.cartItemsContainer) return;

  const totalItems = state.cart.reduce((sum, item) => sum + item.quantity, 0);
  const totalAmount = state.cart.reduce((sum, item) => sum + (item.menuItem.price * item.quantity), 0);

  if (elements.cartBadgeCount) elements.cartBadgeCount.textContent = `${totalItems} items`;
  if (elements.navCartCounter) elements.navCartCounter.textContent = totalItems;
  if (elements.cartTotalAmount) elements.cartTotalAmount.textContent = `₹${totalAmount.toFixed(2)}`;

  if (state.cart.length === 0) {
    elements.cartItemsContainer.innerHTML = `
      <div class="cart-empty-state">
        <div class="cart-empty-icon">🛒</div>
        <p>Your cart is empty.</p>
        <span style="font-size: 0.75rem; color: var(--text-dim);">Selected items will form a Singly Linked List in the C backend order node.</span>
      </div>
    `;
    return;
  }

  elements.cartItemsContainer.innerHTML = state.cart.map(item => `
    <div class="cart-item-row">
      <div class="cart-item-info">
        <h4>${item.menuItem.name}</h4>
        <p>₹${item.menuItem.price.toFixed(2)} × ${item.quantity}</p>
      </div>
      <div style="display: flex; align-items: center;">
        <span class="cart-item-subtotal">₹${(item.menuItem.price * item.quantity).toFixed(2)}</span>
        <button class="cart-item-remove-btn" onclick="updateCartQuantity(${item.menuItem.id}, -${item.quantity})" title="Remove">✕</button>
      </div>
    </div>
  `).join('');
}

function updateQueueCounters() {
  const count = state.queueOrders.length;
  if (elements.navQueueCounter) elements.navQueueCounter.textContent = count;
  if (elements.upNextCount) elements.upNextCount.textContent = `${Math.max(0, count - 1)} waiting`;
}

/* ==========================================================================
   ORDER QUEUE DASHBOARD
   ========================================================================== */
function renderQueueDashboard() {
  if (!elements.nowServingContainer || !elements.upNextContainer) return;

  if (state.queueOrders.length === 0) {
    elements.nowServingContainer.innerHTML = `
      <div class="empty-queue-box">
        <div style="font-size: 3rem; margin-bottom: 0.75rem;">🍽️</div>
        <h3 style="color: #fff; margin-bottom: 0.25rem;">Queue is Empty</h3>
        <p style="font-size: 0.85rem; margin-bottom: 1.25rem;">No customer orders are currently waiting in the C FIFO queue.</p>
        <button class="btn btn-primary btn-sm" onclick="switchSection('section-order')">Place an Order</button>
      </div>
    `;
    elements.upNextContainer.innerHTML = `
      <div style="text-align: center; padding: 2rem; color: var(--text-dim); font-size: 0.85rem;">
        No upcoming orders in queue.
      </div>
    `;
    return;
  }

  // Front order = Now Serving
  const frontOrder = state.queueOrders[0];
  const upNextOrders = state.queueOrders.slice(1);

  // Render Now Serving
  elements.nowServingContainer.innerHTML = `
    <div class="now-serving-card">
      <div class="serving-badge-header">
        <span class="now-serving-label">🔔 Now Serving</span>
        <span class="queue-pointer-pill">Queue FRONT (Head)</span>
      </div>

      <div class="order-hero-number">#${frontOrder.orderId}</div>
      <div class="order-hero-customer">${frontOrder.customerName}</div>

      <div class="order-meta-grid">
        <div>
          <div class="meta-item-label">Table Number</div>
          <div class="meta-item-value">Table ${frontOrder.tableNumber}</div>
        </div>
        <div>
          <div class="meta-item-label">Total Amount</div>
          <div class="meta-item-value" style="color: var(--primary);">₹${frontOrder.total.toFixed(2)}</div>
        </div>
        <div>
          <div class="meta-item-label">Status</div>
          <div class="meta-item-value">
            <span class="status-badge ${frontOrder.status.toLowerCase()}">${frontOrder.status}</span>
          </div>
        </div>
        <div>
          <div class="meta-item-label">Time Placed</div>
          <div class="meta-item-value" style="font-size: 0.8rem; font-family: var(--font-mono);">${frontOrder.createdAt.split(' ')[1] || frontOrder.createdAt}</div>
        </div>
      </div>

      <div class="order-items-preview">
        <div class="order-items-preview-title">
          <span>Ordered Items (Linked List)</span>
          <span style="font-size: 0.7rem; color: #10b981; font-family: var(--font-mono);">${frontOrder.items.length} nodes</span>
        </div>
        <div class="preview-items-chips">
          ${frontOrder.items.map(it => `
            <div class="preview-item-row">
              <span>${it.name} <strong style="color: var(--primary);">× ${it.quantity}</strong></span>
              <span style="font-family: var(--font-mono); color: var(--text-muted);">₹${it.subtotal.toFixed(2)}</span>
            </div>
          `).join('')}
        </div>
      </div>

      <div class="now-serving-actions">
        <button class="btn btn-success btn-lg" onclick="handleProcessNextOrder()">
          ✓ Mark Completed (Dequeue)
        </button>
        <div style="display: flex; gap: 0.5rem;">
          <button class="btn btn-secondary btn-sm" style="flex: 1;" onclick="openOrderDetailsModal(${frontOrder.orderId})">
            📄 View Details
          </button>
          <button class="btn btn-secondary btn-sm" style="flex: 1;" onclick="openBillModal(${frontOrder.orderId})">
            🧾 Generate Bill
          </button>
          <button class="btn btn-danger btn-sm" onclick="cancelOrder(${frontOrder.orderId})">
            Cancel
          </button>
        </div>
      </div>
    </div>
  `;

  // Render Up Next
  if (upNextOrders.length === 0) {
    elements.upNextContainer.innerHTML = `
      <div style="text-align: center; padding: 2.5rem 1rem; color: var(--text-muted);">
        <p style="font-weight: 600; color: #fff;">Queue is clear after this order!</p>
        <p style="font-size: 0.8rem; margin-top: 0.25rem;">New incoming orders will be enqueued at the REAR.</p>
      </div>
    `;
  } else {
    elements.upNextContainer.innerHTML = upNextOrders.map((ord, idx) => `
      <div class="up-next-card">
        <div class="queue-index-badge">${idx + 1}</div>
        <div class="up-next-details">
          <div class="up-next-title-row">
            <span class="up-next-order-id">#${ord.orderId}</span>
            <span class="status-badge ${ord.status.toLowerCase()}">${ord.status}</span>
            <span style="font-size: 0.75rem; color: var(--text-dim); margin-left: auto;">Table ${ord.tableNumber}</span>
          </div>
          <div class="up-next-customer">${ord.customerName}</div>
          <div class="up-next-summary">
            ${ord.items.map(i => `${i.name} (${i.quantity})`).join(', ')} • <strong>₹${ord.total.toFixed(2)}</strong>
          </div>
        </div>
        <div style="display: flex; flex-direction: column; gap: 0.35rem;">
          <button class="btn btn-secondary btn-sm" onclick="openOrderDetailsModal(${ord.orderId})">View</button>
          <button class="btn btn-secondary btn-sm" onclick="openBillModal(${ord.orderId})">Bill</button>
        </div>
      </div>
    `).join('');
  }
}

/* ==========================================================================
   KITCHEN / ADMIN DASHBOARD & METRICS
   ========================================================================== */
function renderStats() {
  if (elements.statTotalOrders) elements.statTotalOrders.textContent = state.stats.totalOrders;
  if (elements.statPendingOrders) elements.statPendingOrders.textContent = state.stats.pendingOrders;
  if (elements.statPreparingOrders) elements.statPreparingOrders.textContent = state.stats.preparingOrders;
  if (elements.statCompletedOrders) elements.statCompletedOrders.textContent = state.stats.completedOrders;
  if (elements.statTotalRevenue) elements.statTotalRevenue.textContent = `₹${state.stats.totalRevenue.toFixed(2)}`;
}

function renderKitchenDashboard() {
  if (!elements.colPending) return;

  const allOrders = [...state.queueOrders, ...state.completedOrders, ...state.cancelledOrders];

  // Apply search query filter
  const filtered = state.searchQuery
    ? allOrders.filter(o =>
        o.orderId.toString().includes(state.searchQuery) ||
        o.customerName.toLowerCase().includes(state.searchQuery) ||
        o.tableNumber.toString().includes(state.searchQuery)
      )
    : allOrders;

  const pending = filtered.filter(o => o.status === 'Pending');
  const preparing = filtered.filter(o => o.status === 'Preparing');
  const ready = filtered.filter(o => o.status === 'Ready');
  const completed = filtered.filter(o => o.status === 'Completed');

  renderTicketColumn(elements.colPending, pending);
  renderTicketColumn(elements.colPreparing, preparing);
  renderTicketColumn(elements.colReady, ready);
  renderTicketColumn(elements.colCompleted, completed.slice(0, 10)); // show recent 10 completed

  // Update column badges
  const setBadge = (id, count) => {
    const el = document.getElementById(id);
    if (el) el.textContent = count;
  };
  setBadge('badge-pending', pending.length);
  setBadge('badge-preparing', preparing.length);
  setBadge('badge-ready', ready.length);
  setBadge('badge-completed', completed.length);
}

function renderTicketColumn(container, orders) {
  if (!container) return;

  if (orders.length === 0) {
    container.innerHTML = `<div style="text-align: center; color: var(--text-dim); padding: 1.5rem; font-size: 0.8rem;">No tickets</div>`;
    return;
  }

  container.innerHTML = orders.map(ord => `
    <div class="kitchen-ticket">
      <div class="ticket-top">
        <span class="ticket-order-id">Order #${ord.orderId}</span>
        <span class="ticket-table">Table ${ord.tableNumber}</span>
      </div>
      <div class="ticket-customer">${ord.customerName}</div>
      <div class="ticket-items-list">
        ${ord.items.map(i => `
          <div class="ticket-item-row">
            <span>${i.name}</span>
            <strong>×${i.quantity}</strong>
          </div>
        `).join('')}
      </div>
      <div class="ticket-footer">
        <span class="ticket-total">₹${ord.total.toFixed(2)}</span>
        <div style="display: flex; gap: 0.35rem;">
          <button class="btn btn-secondary btn-sm" onclick="openOrderDetailsModal(${ord.orderId})">Details</button>
          ${ord.status === 'Pending' ? `
            <button class="btn btn-primary btn-sm" onclick="updateOrderStatus(${ord.orderId}, 'Preparing')">Cook</button>
          ` : ''}
          ${ord.status === 'Preparing' ? `
            <button class="btn btn-primary btn-sm" onclick="updateOrderStatus(${ord.orderId}, 'Ready')">Ready</button>
          ` : ''}
          ${ord.status === 'Ready' ? `
            <button class="btn btn-success btn-sm" onclick="updateOrderStatus(${ord.orderId}, 'Completed')">Serve</button>
          ` : ''}
        </div>
      </div>
    </div>
  `).join('');
}

/* ==========================================================================
   DATA STRUCTURE VISUALIZATION (THE CORE FEATURE)
   ========================================================================== */
function renderVisualization() {
  if (!elements.dsQueueFlow) return;

  const vis = state.visualization;
  if (!vis) return;

  if (elements.dsFrontPtr) elements.dsFrontPtr.textContent = `FRONT: ${vis.frontPtr}`;
  if (elements.dsRearPtr) elements.dsRearPtr.textContent = `REAR: ${vis.rearPtr}`;
  if (elements.dsQueueCount) elements.dsQueueCount.textContent = `Length: ${vis.count}`;

  // If queue is empty
  if (!vis.nodes || vis.nodes.length === 0) {
    elements.dsQueueFlow.innerHTML = `
      <div style="display: flex; align-items: center; gap: 1rem; padding: 2rem;">
        <div class="pointer-flag front" style="position: static; transform: none;">FRONT → NULL</div>
        <div class="queue-null-box">EMPTY QUEUE (front == NULL, rear == NULL)</div>
        <div class="pointer-flag rear" style="position: static; transform: none;">REAR → NULL</div>
      </div>
    `;
    if (elements.dsLinkedListInspector) {
      elements.dsLinkedListInspector.style.display = 'none';
    }
    return;
  }

  // Render Queue Chain
  const nodesHtml = vis.nodes.map((node, idx) => {
    let pointerFlag = '';
    let flagClass = '';

    if (node.isFront && node.isRear) {
      pointerFlag = 'FRONT & REAR';
      flagClass = 'both';
    } else if (node.isFront) {
      pointerFlag = 'FRONT (Head of Queue)';
      flagClass = 'front';
    } else if (node.isRear) {
      pointerFlag = 'REAR (Tail of Queue)';
      flagClass = 'rear';
    }

    const isSelected = state.selectedVisOrder === node.orderId;

    return `
      <div class="queue-node-box ${node.isFront ? 'is-front' : ''} ${node.isRear ? 'is-rear' : ''} ${isSelected ? 'selected' : ''}"
           onclick="inspectOrderLinkedList(${node.orderId})"
           style="${isSelected ? 'outline: 2px solid #38bdf8;' : ''}">
        
        ${pointerFlag ? `<div class="pointer-flag ${flagClass}">${pointerFlag}</div>` : ''}

        <div class="node-header-row">
          <span class="node-order-title">Order #${node.orderId}</span>
          <span class="node-memory-addr">${node.nodePtr}</span>
        </div>

        <div class="node-customer-line">${node.customerName} (Table ${node.tableNumber})</div>
        
        <div style="display: flex; justify-content: space-between; font-size: 0.75rem; margin-bottom: 0.4rem;">
          <span class="status-badge ${node.status.toLowerCase()}">${node.status}</span>
          <span style="font-weight: 700; color: #fff;">₹${node.total.toFixed(2)}</span>
        </div>

        <div class="node-pointer-field">
          <span>*items (HEAD)</span>
          <span style="color: #10b981;">${node.itemsLinkedList?.headPtr || 'NULL'}</span>
        </div>

        <div class="node-pointer-field">
          <span>*next (Queue)</span>
          <span>${node.nextPtr}</span>
        </div>
      </div>

      <div class="queue-arrow">
        <span class="queue-arrow-icon">→</span>
        <span>next</span>
      </div>
    `;
  }).join('');

  elements.dsQueueFlow.innerHTML = `
    <div class="queue-flow-chain">
      ${nodesHtml}
      <div class="queue-null-box">NULL</div>
    </div>
  `;

  // Auto-inspect the FRONT order if none selected or if selected order is no longer in queue
  const currentInQueue = vis.nodes.find(n => n.orderId === state.selectedVisOrder);
  if (!currentInQueue && vis.nodes.length > 0) {
    inspectOrderLinkedList(vis.nodes[0].orderId);
  } else if (currentInQueue) {
    inspectOrderLinkedList(currentInQueue.orderId);
  }
}

/* Singly Linked List Food Items Inspector */
window.inspectOrderLinkedList = function(orderId) {
  state.selectedVisOrder = orderId;
  const vis = state.visualization;
  if (!vis || !vis.nodes) return;

  const node = vis.nodes.find(n => n.orderId === orderId);
  if (!node || !elements.dsLinkedListInspector) return;

  elements.dsLinkedListInspector.style.display = 'block';
  if (elements.dsSelectedOrderBadge) {
    elements.dsSelectedOrderBadge.textContent = `Order #${node.orderId} (${node.customerName})`;
  }
  if (elements.dsLlHeadPtr) {
    elements.dsLlHeadPtr.textContent = `HEAD: ${node.itemsLinkedList?.headPtr || 'NULL'}`;
  }

  const itemsList = node.itemsLinkedList?.nodes || [];
  if (itemsList.length === 0) {
    elements.dsLlNodesChain.innerHTML = `<div class="queue-null-box">HEAD → NULL (No Items)</div>`;
    return;
  }

  const itemsHtml = itemsList.map((item, idx) => `
    <div class="ll-node-box ${item.isHead ? 'is-head' : ''}">
      ${item.isHead ? `<div class="ll-head-flag">HEAD (order->items)</div>` : ''}

      <div style="display: flex; justify-content: space-between; align-items: flex-start;">
        <span class="ll-node-name">${item.name}</span>
        <span class="node-memory-addr">${item.ptr}</span>
      </div>

      <div class="ll-node-meta">
        <span>Qty: <strong style="color: var(--primary);">${item.quantity}</strong></span>
        <span>Unit: ₹${item.price.toFixed(2)}</span>
      </div>

      <div class="ll-node-next-field">
        <span>*next</span>
        <span>${item.nextPtr}</span>
      </div>
    </div>

    <div class="queue-arrow">
      <span class="queue-arrow-icon" style="color: #10b981;">→</span>
      <span style="color: #10b981;">next</span>
    </div>
  `).join('');

  elements.dsLlNodesChain.innerHTML = `
    ${itemsHtml}
    <div class="queue-null-box" style="border-color: #10b981; color: #34d399;">NULL</div>
  `;
};

/* Quick DS Sandbox Operations */
window.sandboxEnqueueQuickOrder = async function() {
  const sampleNames = ['Rohan Das', 'Kavita Iyer', 'Siddharth Rao', 'Deepak Joshi', 'Neha Reddy'];
  const randomName = sampleNames[Math.floor(Math.random() * sampleNames.length)];
  const randomTable = Math.floor(Math.random() * 20) + 1;

  // Pick 2 random menu items
  const item1 = state.menu[Math.floor(Math.random() * state.menu.length)] || { id: 1 };
  const item2 = state.menu[Math.floor(Math.random() * state.menu.length)] || { id: 3 };

  const payload = {
    customerName: randomName,
    tableNumber: randomTable,
    items: [
      { itemId: item1.id, quantity: 1 },
      { itemId: item2.id, quantity: 2 }
    ]
  };

  try {
    const res = await fetch(`${API_BASE}/orders`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(payload)
    });
    if (!res.ok) throw new Error('Enqueue failed');
    const ord = await res.json();
    showToast(`Sandbox: Enqueued Order #${ord.orderId} at REAR!`, 'success');
    await refreshQueueAndStats();
  } catch (err) {
    showToast(err.message, 'danger');
  }
};

window.sandboxDequeueOrder = async function() {
  await handleProcessNextOrder();
};

window.sandboxPeekFront = function() {
  if (state.queueOrders.length === 0) {
    showToast('Queue is EMPTY. peek() returns NULL', 'warning');
    return;
  }
  const front = state.queueOrders[0];
  showToast(`peek() returned Order #${front.orderId} (${front.customerName})`, 'info');
};

/* ==========================================================================
   BILL RECEIPT GENERATION & MODALS
   ========================================================================== */
window.openBillModal = async function(orderId) {
  let ord = [...state.queueOrders, ...state.completedOrders, ...state.cancelledOrders].find(o => o.orderId === orderId);

  if (!ord) {
    try {
      const res = await fetch(`${API_BASE}/orders/${orderId}`);
      if (res.ok) ord = await res.json();
    } catch (e) {
      console.error(e);
    }
  }

  if (!ord) {
    showToast('Order details not found', 'danger');
    return;
  }

  const tax = ord.total * 0.05; // 5% GST
  const grandTotal = ord.total + tax;

  elements.modalBody.innerHTML = `
    <div class="bill-container" id="printable-bill">
      <div class="bill-restaurant-name">Spice Kitchen</div>
      <div class="bill-address">Foodie Avenue, Tech City • GSTIN: 29AABCU9603R1ZM</div>
      <hr class="bill-divider">

      <div class="bill-info-grid">
        <div><strong>Order No:</strong> #${ord.orderId}</div>
        <div><strong>Table:</strong> ${ord.tableNumber}</div>
        <div><strong>Customer:</strong> ${ord.customerName}</div>
        <div><strong>Date:</strong> ${ord.createdAt}</div>
        <div><strong>Status:</strong> ${ord.status}</div>
        <div><strong>Type:</strong> Dine-In</div>
      </div>

      <hr class="bill-divider">

      <table class="bill-items-table">
        <thead>
          <tr>
            <th>Item</th>
            <th style="text-align: center;">Qty</th>
            <th style="text-align: right;">Price</th>
            <th>Amount</th>
          </tr>
        </thead>
        <tbody>
          ${ord.items.map(it => `
            <tr>
              <td>${it.name}</td>
              <td style="text-align: center;">${it.quantity}</td>
              <td style="text-align: right;">₹${it.price.toFixed(2)}</td>
              <td>₹${it.subtotal.toFixed(2)}</td>
            </tr>
          `).join('')}
        </tbody>
      </table>

      <div class="bill-divider"></div>

      <div style="display: flex; justify-content: space-between; font-size: 0.85rem; margin-top: 0.5rem;">
        <span>Subtotal:</span>
        <span>₹${ord.total.toFixed(2)}</span>
      </div>
      <div style="display: flex; justify-content: space-between; font-size: 0.8rem; color: #64748b;">
        <span>GST (5%):</span>
        <span>₹${tax.toFixed(2)}</span>
      </div>

      <div class="bill-total-row">
        <span>TOTAL DUE:</span>
        <span style="color: #ea580c;">₹${grandTotal.toFixed(2)}</span>
      </div>

      <div class="bill-footer-note">
        Thank you for dining with us!<br>
        Data Structures Mini Project • Powered by C FIFO Queue & Linked List
      </div>
    </div>

    <div style="display: flex; justify-content: flex-end; gap: 0.75rem; margin-top: 1.5rem;">
      <button class="btn btn-secondary btn-sm" onclick="downloadBillText(${ord.orderId})">💾 Download Receipt</button>
      <button class="btn btn-primary btn-sm" onclick="window.print()">🖨️ Print Bill</button>
    </div>
  `;

  openModal();
};

window.openOrderDetailsModal = async function(orderId) {
  let ord = [...state.queueOrders, ...state.completedOrders, ...state.cancelledOrders].find(o => o.orderId === orderId);

  if (!ord) {
    try {
      const res = await fetch(`${API_BASE}/orders/${orderId}`);
      if (res.ok) ord = await res.json();
    } catch (e) {
      console.error(e);
    }
  }

  if (!ord) {
    showToast('Order not found', 'danger');
    return;
  }

  elements.modalBody.innerHTML = `
    <div style="margin-bottom: 1.25rem;">
      <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 0.5rem;">
        <h2 style="font-size: 1.5rem; font-weight: 800; color: #fff;">Order #${ord.orderId}</h2>
        <span class="status-badge ${ord.status.toLowerCase()}">${ord.status}</span>
      </div>
      <p style="color: var(--text-muted); font-size: 0.875rem;">Placed at ${ord.createdAt}</p>
    </div>

    <div style="background: rgba(15, 23, 42, 0.7); border-radius: var(--radius-sm); padding: 1rem; margin-bottom: 1.25rem; display: grid; grid-template-columns: 1fr 1fr; gap: 0.75rem;">
      <div>
        <span style="font-size: 0.72rem; color: var(--text-dim); text-transform: uppercase;">Customer</span>
        <div style="font-weight: 700; color: #fff;">${ord.customerName}</div>
      </div>
      <div>
        <span style="font-size: 0.72rem; color: var(--text-dim); text-transform: uppercase;">Table</span>
        <div style="font-weight: 700; color: #fff;">Table ${ord.tableNumber}</div>
      </div>
    </div>

    <h4 style="font-size: 0.95rem; font-weight: 700; color: #fff; margin-bottom: 0.75rem;">
      Ordered Items (Singly Linked List)
    </h4>
    <div style="display: flex; flex-direction: column; gap: 0.5rem; margin-bottom: 1.25rem;">
      ${ord.items.map(it => `
        <div style="display: flex; justify-content: space-between; align-items: center; background: rgba(255, 255, 255, 0.04); padding: 0.6rem 0.85rem; border-radius: 6px;">
          <div>
            <div style="font-weight: 600; color: #fff; font-size: 0.9rem;">${it.name}</div>
            <div style="font-size: 0.75rem; color: var(--text-muted);">₹${it.price.toFixed(2)} × ${it.quantity}</div>
          </div>
          <div style="font-weight: 700; font-family: var(--font-mono); color: #fff;">
            ₹${it.subtotal.toFixed(2)}
          </div>
        </div>
      `).join('')}
    </div>

    <div style="display: flex; justify-content: space-between; align-items: center; padding-top: 1rem; border-top: 1px solid var(--border); font-size: 1.2rem; font-weight: 800; color: #fff;">
      <span>Total Amount:</span>
      <span style="color: var(--primary); font-family: var(--font-mono);">₹${ord.total.toFixed(2)}</span>
    </div>

    <div style="display: flex; gap: 0.75rem; margin-top: 1.5rem;">
      <button class="btn btn-secondary btn-sm" style="flex: 1;" onclick="openBillModal(${ord.orderId})">🧾 View Bill</button>
      <button class="btn btn-primary btn-sm" style="flex: 1;" onclick="inspectOrderLinkedList(${ord.orderId}); switchSection('section-visualization'); closeModal();">
        🧠 Inspect Data Structure
      </button>
    </div>
  `;

  openModal();
};

window.downloadBillText = function(orderId) {
  const ord = [...state.queueOrders, ...state.completedOrders, ...state.cancelledOrders].find(o => o.orderId === orderId);
  if (!ord) return;

  const content = `
========================================
             SPICE KITCHEN              
   Restaurant Order Management System   
========================================
Order No: #${ord.orderId}
Customer: ${ord.customerName}
Table:    Table ${ord.tableNumber}
Date:     ${ord.createdAt}
Status:   ${ord.status}
----------------------------------------
ITEM                      QTY    AMOUNT
----------------------------------------
${ord.items.map(i => `${i.name.padEnd(24)} ×${i.quantity}   ₹${i.subtotal.toFixed(2)}`).join('\n')}
----------------------------------------
Subtotal:                        ₹${ord.total.toFixed(2)}
GST (5%):                        ₹${(ord.total * 0.05).toFixed(2)}
----------------------------------------
TOTAL:                           ₹${(ord.total * 1.05).toFixed(2)}
========================================
Data Structures: C FIFO Queue & Linked List
========================================
`;

  const blob = new Blob([content], { type: 'text/plain;charset=utf-8' });
  const url = URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  a.download = `Bill_Order_${ord.orderId}.txt`;
  document.body.appendChild(a);
  a.click();
  document.body.removeChild(a);
  URL.revokeObjectURL(url);
};

function openModal() {
  if (elements.modalOverlay) elements.modalOverlay.classList.add('active');
}

function closeModal() {
  if (elements.modalOverlay) elements.modalOverlay.classList.remove('active');
}

/* ==========================================================================
   C CONSOLE LOG MONITOR
   ========================================================================== */
function renderConsoleLogs() {
  if (!elements.cConsoleBody) return;

  if (state.logs.length === 0) {
    elements.cConsoleBody.innerHTML = `<div style="color: var(--text-dim);">Listening to C backend server events...</div>`;
    return;
  }

  elements.cConsoleBody.innerHTML = state.logs.map(log => {
    let cssClass = '';
    if (log.includes('[QUEUE')) cssClass = 'queue';
    else if (log.includes('[LINKED LIST')) cssClass = 'll';
    else if (log.includes('[ORDER')) cssClass = 'order';
    else if (log.includes('[ERROR')) cssClass = 'error';

    return `<div class="log-entry ${cssClass}">${escapeHtml(log)}</div>`;
  }).join('');

  elements.cConsoleBody.scrollTop = elements.cConsoleBody.scrollHeight;
}

/* Toast Notifications */
function showToast(message, type = 'info') {
  if (!elements.toastContainer) return;

  const toast = document.createElement('div');
  toast.className = `toast ${type}`;
  toast.innerHTML = `
    <span>${type === 'success' ? '✓' : type === 'danger' ? '⚠️' : 'ℹ️'}</span>
    <div>${escapeHtml(message)}</div>
  `;

  elements.toastContainer.appendChild(toast);

  setTimeout(() => {
    toast.style.opacity = '0';
    toast.style.transform = 'translateX(20px)';
    setTimeout(() => toast.remove(), 300);
  }, 3500);
}

function escapeHtml(text) {
  const map = {
    '&': '&amp;',
    '<': '&lt;',
    '>': '&gt;',
    '"': '&quot;',
    "'": '&#039;'
  };
  return String(text).replace(/[&<>"']/g, m => map[m]);
}
