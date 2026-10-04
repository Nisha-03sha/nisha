/**
 * ============================================================================
 * BASIL & EMBER - RESTAURANT JAVASCRIPT ENGINE
 * Production-style Online Ordering, Real-Time Cart, and Live Kitchen Tracking
 * ============================================================================
 */

/* Application State */
const state = {
  menu: [],
  cart: [],
  activeCategory: 'all',
  vegOnlyFilter: false,
  searchQuery: '',
  diningType: 'dine-in', // 'dine-in' or 'takeaway'
  activeOrderId: null,
  activeOrderData: null,
  kitchenOrders: [],
  kitchenStats: {
    totalOrders: 24,
    pendingOrders: 2,
    preparingOrders: 4,
    readyOrders: 2,
    completedOrders: 16,
    totalRevenue: 8420
  }
};

const API_BASE = '/api';

// Initialize when DOM is ready
document.addEventListener('DOMContentLoaded', () => {
  initApp();
});

async function initApp() {
  setupNavigation();
  setupCartDrawer();
  setupCheckout();
  setupAdminControls();
  
  // Load saved cart from localStorage if present
  loadSavedCart();

  // Fetch live menu from C backend
  await fetchMenu();

  // Handle direct hash navigation (#menu, #tracking, #admin, #checkout)
  handleHashNavigation();
  window.addEventListener('hashchange', handleHashNavigation);

  // Periodic polling for kitchen stats and active order tracking
  setInterval(() => {
    if (state.activeOrderId) {
      pollActiveOrderStatus(state.activeOrderId);
    }
    const currentHash = window.location.hash;
    if (currentHash === '#admin') {
      fetchKitchenData();
    }
  }, 3500);
}

/* ==========================================================================
   NAVIGATION & ROUTING
   ========================================================================== */
function setupNavigation() {
  document.querySelectorAll('[data-route]').forEach(el => {
    el.addEventListener('click', (e) => {
      e.preventDefault();
      const route = el.getAttribute('data-route');
      navigateTo(route);
    });
  });
}

function handleHashNavigation() {
  const hash = window.location.hash.replace('#', '') || 'home';
  navigateTo(hash, false);
}

function navigateTo(route, updateHash = true) {
  if (updateHash) {
    window.location.hash = route;
  }

  // Update nav links
  document.querySelectorAll('.nav-link-btn').forEach(btn => {
    btn.classList.toggle('active', btn.getAttribute('data-route') === route);
  });

  // Switch visible section
  document.querySelectorAll('.view-section').forEach(sec => {
    sec.classList.remove('active');
  });

  const targetSec = document.getElementById(`view-${route}`);
  if (targetSec) {
    targetSec.classList.add('active');
    window.scrollTo({ top: 0, behavior: 'smooth' });
  }

  if (route === 'admin') {
    fetchKitchenData();
  } else if (route === 'tracking' && state.activeOrderId) {
    pollActiveOrderStatus(state.activeOrderId);
  }
}

/* ==========================================================================
   MENU DATA & RENDERING
   ========================================================================== */
async function fetchMenu() {
  try {
    const res = await fetch(`${API_BASE}/menu`);
    if (!res.ok) throw new Error('Menu fetch failed');
    state.menu = await res.json();
    renderCategories();
    renderMenuGrid();
    renderFeaturedSpecials();
  } catch (err) {
    console.error('Menu load error:', err);
  }
}

function renderCategories() {
  const container = document.getElementById('category-nav-bar');
  if (!container) return;

  const categories = ['all', ...new Set(state.menu.map(m => m.category))];
  
  container.innerHTML = categories.map(cat => `
    <button class="cat-tab-btn ${state.activeCategory === cat ? 'active' : ''}" onclick="selectCategory('${cat}')">
      ${cat === 'all' ? 'All Dishes' : cat}
    </button>
  `).join('');
}

window.selectCategory = function(cat) {
  state.activeCategory = cat;
  renderCategories();
  renderMenuGrid();
};

window.toggleVegFilter = function() {
  state.vegOnlyFilter = !state.vegOnlyFilter;
  const btn = document.getElementById('veg-filter-btn');
  if (btn) btn.classList.toggle('active', state.vegOnlyFilter);
  renderMenuGrid();
};

window.handleMenuSearch = function(query) {
  state.searchQuery = query.toLowerCase().trim();
  renderMenuGrid();
};

function renderMenuGrid() {
  const container = document.getElementById('menu-dishes-grid');
  if (!container) return;

  let filtered = state.menu;

  if (state.activeCategory !== 'all') {
    filtered = filtered.filter(item => item.category === state.activeCategory);
  }

  if (state.vegOnlyFilter) {
    filtered = filtered.filter(item => item.isVeg);
  }

  if (state.searchQuery) {
    filtered = filtered.filter(item => 
      item.name.toLowerCase().includes(state.searchQuery) ||
      item.description.toLowerCase().includes(state.searchQuery)
    );
  }

  if (filtered.length === 0) {
    container.innerHTML = `
      <div style="grid-column: 1/-1; text-align: center; padding: 4rem 1rem; color: var(--text-muted);">
        <p style="font-size: 1.1rem; color: #fff; margin-bottom: 0.5rem;">No dishes found matching your selection.</p>
        <p style="font-size: 0.85rem;">Try clearing your search query or dietary filters.</p>
      </div>
    `;
    return;
  }

  container.innerHTML = filtered.map(dish => {
    const cartItem = state.cart.find(c => c.dish.id === dish.id);
    const qty = cartItem ? cartItem.quantity : 0;

    return `
      <article class="dish-card" id="dish-${dish.id}">
        <div class="dish-image-wrapper">
          <img src="${dish.imageUrl}" alt="${dish.name}" loading="lazy" />
          <div class="dish-badge-pill">
            <span class="diet-indicator ${dish.isVeg ? 'veg' : 'non-veg'}"></span>
            <span>${dish.isVeg ? 'Vegetarian' : 'Non-Veg'}</span>
          </div>
          ${dish.isChefSpecial ? `<span class="chef-special-badge">Chef's Signature</span>` : ''}
        </div>

        <div class="dish-body">
          <span class="dish-category-label">${dish.category}</span>
          <h3 class="dish-title">${dish.name}</h3>
          <p class="dish-desc">${dish.description}</p>

          <div class="dish-footer">
            <div class="dish-price"><span>₹</span>${dish.price.toFixed(2)}</div>
            ${qty === 0 ? `
              <button class="dish-add-btn" onclick="addToCart(${dish.id})">
                + Add
              </button>
            ` : `
              <div class="qty-control">
                <button class="qty-btn" onclick="updateItemQuantity(${dish.id}, -1)">−</button>
                <span class="qty-num">${qty}</span>
                <button class="qty-btn" onclick="updateItemQuantity(${dish.id}, 1)">+</button>
              </div>
            `}
          </div>
        </div>
      </article>
    `;
  }).join('');
}

function renderFeaturedSpecials() {
  const container = document.getElementById('featured-specials-grid');
  if (!container) return;

  const signatures = state.menu.filter(m => m.isChefSpecial).slice(0, 3);
  container.innerHTML = signatures.map(dish => `
    <article class="dish-card">
      <div class="dish-image-wrapper">
        <img src="${dish.imageUrl}" alt="${dish.name}" loading="lazy" />
        <div class="dish-badge-pill">
          <span class="diet-indicator ${dish.isVeg ? 'veg' : 'non-veg'}"></span>
          <span>${dish.category}</span>
        </div>
        <span class="chef-special-badge">Signature</span>
      </div>
      <div class="dish-body">
        <h3 class="dish-title">${dish.name}</h3>
        <p class="dish-desc">${dish.description}</p>
        <div class="dish-footer">
          <div class="dish-price"><span>₹</span>${dish.price.toFixed(2)}</div>
          <button class="btn btn-primary btn-sm" onclick="addToCart(${dish.id}); openCartDrawer();">
            Order Now
          </button>
        </div>
      </div>
    </article>
  `).join('');
}

/* ==========================================================================
   CART OPERATIONS
   ========================================================================== */
function setupCartDrawer() {
  const backdrop = document.getElementById('cart-backdrop');
  const openButtons = document.querySelectorAll('.open-cart-trigger');
  const closeButton = document.getElementById('cart-close-btn');

  openButtons.forEach(btn => btn.addEventListener('click', openCartDrawer));
  if (closeButton) closeButton.addEventListener('click', closeCartDrawer);
  if (backdrop) {
    backdrop.addEventListener('click', (e) => {
      if (e.target === backdrop) closeCartDrawer();
    });
  }
}

window.openCartDrawer = function() {
  const backdrop = document.getElementById('cart-backdrop');
  if (backdrop) backdrop.classList.add('open');
};

window.closeCartDrawer = function() {
  const backdrop = document.getElementById('cart-backdrop');
  if (backdrop) backdrop.classList.remove('open');
};

window.addToCart = function(dishId) {
  const dish = state.menu.find(d => d.id === dishId);
  if (!dish) return;

  const existing = state.cart.find(c => c.dish.id === dishId);
  if (existing) {
    existing.quantity += 1;
  } else {
    state.cart.push({ dish, quantity: 1 });
  }

  saveCart();
  updateCartUI();
  renderMenuGrid();
  showToast(`Added ${dish.name} to your order`, 'success');
};

window.updateItemQuantity = function(dishId, delta) {
  const index = state.cart.findIndex(c => c.dish.id === dishId);
  if (index === -1) return;

  state.cart[index].quantity += delta;
  if (state.cart[index].quantity <= 0) {
    const removedName = state.cart[index].dish.name;
    state.cart.splice(index, 1);
    showToast(`Removed ${removedName}`, 'info');
  }

  saveCart();
  updateCartUI();
  renderMenuGrid();
};

function saveCart() {
  try {
    localStorage.setItem('basil_ember_cart', JSON.stringify(state.cart));
  } catch (e) {
    console.warn(e);
  }
}

function loadSavedCart() {
  try {
    const raw = localStorage.getItem('basil_ember_cart');
    if (raw) state.cart = JSON.parse(raw);
  } catch (e) {
    state.cart = [];
  }
  updateCartUI();
}

function updateCartUI() {
  const totalCount = state.cart.reduce((sum, item) => sum + item.quantity, 0);
  const subtotal = state.cart.reduce((sum, item) => sum + (item.dish.price * item.quantity), 0);
  const taxes = subtotal * 0.05; // 5% GST
  const grandTotal = subtotal + taxes;

  // Header Cart Counter
  document.querySelectorAll('.cart-count-badge').forEach(b => {
    b.textContent = totalCount;
  });

  // Drawer Elements
  const drawerBody = document.getElementById('cart-drawer-items');
  const drawerSubtotal = document.getElementById('cart-drawer-subtotal');
  const drawerTax = document.getElementById('cart-drawer-tax');
  const drawerTotal = document.getElementById('cart-drawer-total');
  const checkoutBtn = document.getElementById('cart-drawer-checkout-btn');

  if (drawerSubtotal) drawerSubtotal.textContent = `₹${subtotal.toFixed(2)}`;
  if (drawerTax) drawerTax.textContent = `₹${taxes.toFixed(2)}`;
  if (drawerTotal) drawerTotal.textContent = `₹${grandTotal.toFixed(2)}`;

  if (checkoutBtn) {
    checkoutBtn.disabled = state.cart.length === 0;
  }

  if (!drawerBody) return;

  if (state.cart.length === 0) {
    drawerBody.innerHTML = `
      <div class="cart-empty-message">
        <div class="icon">🛒</div>
        <h4 style="color: #fff; margin-bottom: 0.35rem;">Your order is currently empty</h4>
        <p style="font-size: 0.85rem;">Discover our freshly prepared dishes and add your favorites to get started.</p>
        <button class="btn btn-primary btn-sm" style="margin-top: 1.25rem;" onclick="closeCartDrawer(); navigateTo('menu');">
          Browse Menu
        </button>
      </div>
    `;
    return;
  }

  drawerBody.innerHTML = state.cart.map(item => `
    <div class="cart-dish-row">
      <div class="cart-dish-info">
        <h4>${item.dish.name}</h4>
        <p>₹${item.dish.price.toFixed(2)} each</p>
      </div>

      <div class="cart-dish-right">
        <div class="qty-control">
          <button class="qty-btn" onclick="updateItemQuantity(${item.dish.id}, -1)">−</button>
          <span class="qty-num">${item.quantity}</span>
          <button class="qty-btn" onclick="updateItemQuantity(${item.dish.id}, 1)">+</button>
        </div>
        <div class="cart-dish-price">₹${(item.dish.price * item.quantity).toFixed(2)}</div>
      </div>
    </div>
  `).join('');

  // Update checkout order summary if active
  renderCheckoutSummary();
}

/* ==========================================================================
   CHECKOUT & PLACING ORDER
   ========================================================================== */
function setupCheckout() {
  const form = document.getElementById('checkout-form');
  if (form) {
    form.addEventListener('submit', handleCheckoutSubmit);
  }

  document.querySelectorAll('.dining-pill').forEach(pill => {
    pill.addEventListener('click', () => {
      document.querySelectorAll('.dining-pill').forEach(p => p.classList.remove('active'));
      pill.classList.add('active');
      state.diningType = pill.getAttribute('data-type');
      
      const tableGroup = document.getElementById('checkout-table-group');
      if (tableGroup) {
        tableGroup.style.display = state.diningType === 'dine-in' ? 'block' : 'none';
      }
    });
  });
}

function renderCheckoutSummary() {
  const summaryContainer = document.getElementById('checkout-summary-items');
  const summarySubtotal = document.getElementById('checkout-subtotal');
  const summaryTax = document.getElementById('checkout-tax');
  const summaryTotal = document.getElementById('checkout-total');

  if (!summaryContainer) return;

  const subtotal = state.cart.reduce((sum, item) => sum + (item.dish.price * item.quantity), 0);
  const taxes = subtotal * 0.05;
  const grandTotal = subtotal + taxes;

  if (summarySubtotal) summarySubtotal.textContent = `₹${subtotal.toFixed(2)}`;
  if (summaryTax) summaryTax.textContent = `₹${taxes.toFixed(2)}`;
  if (summaryTotal) summaryTotal.textContent = `₹${grandTotal.toFixed(2)}`;

  if (state.cart.length === 0) {
    summaryContainer.innerHTML = `<p style="color: var(--text-muted); font-size: 0.85rem;">No items selected yet.</p>`;
    return;
  }

  summaryContainer.innerHTML = state.cart.map(item => `
    <div style="display: flex; justify-content: space-between; font-size: 0.85rem; padding: 0.35rem 0;">
      <span style="color: #fff;">${item.dish.name} <strong style="color: var(--primary);">× ${item.quantity}</strong></span>
      <span style="font-weight: 600;">₹${(item.dish.price * item.quantity).toFixed(2)}</span>
    </div>
  `).join('');
}

async function handleCheckoutSubmit(e) {
  e.preventDefault();

  if (state.cart.length === 0) {
    showToast('Your order is empty. Please add dishes from the menu.', 'error');
    navigateTo('menu');
    return;
  }

  const nameInput = document.getElementById('checkout-name');
  const phoneInput = document.getElementById('checkout-phone');
  const tableInput = document.getElementById('checkout-table');
  const submitBtn = document.getElementById('checkout-submit-btn');

  const customerName = nameInput.value.trim();
  const phone = phoneInput.value.trim();
  const tableNumber = state.diningType === 'dine-in' ? parseInt(tableInput.value, 10) : 0;

  if (!customerName) {
    showToast('Please enter your name.', 'error');
    nameInput.focus();
    return;
  }

  if (!phone || phone.length < 10) {
    showToast('Please enter a valid 10-digit contact number.', 'error');
    phoneInput.focus();
    return;
  }

  if (state.diningType === 'dine-in' && (isNaN(tableNumber) || tableNumber < 1 || tableNumber > 50)) {
    showToast('Please enter a valid table number (1 to 50).', 'error');
    tableInput.focus();
    return;
  }

  const payload = {
    customerName,
    phone,
    tableNumber,
    items: state.cart.map(item => ({
      itemId: item.dish.id,
      quantity: item.quantity
    }))
  };

  try {
    submitBtn.disabled = true;
    submitBtn.innerHTML = 'Sending to Kitchen...';

    const res = await fetch(`${API_BASE}/orders`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(payload)
    });

    if (!res.ok) {
      const err = await res.json();
      throw new Error(err.error || 'Failed to submit order');
    }

    const createdOrder = await res.json();
    state.activeOrderId = createdOrder.orderId;
    state.activeOrderData = createdOrder;

    // Reset Cart
    state.cart = [];
    saveCart();
    updateCartUI();

    showToast(`Order #${createdOrder.orderId} placed successfully!`, 'success');

    // Switch to Tracking view
    navigateTo('tracking');
    renderActiveOrderTracking(createdOrder);
  } catch (err) {
    console.error('Order submission error:', err);
    showToast(err.message, 'error');
  } finally {
    submitBtn.disabled = false;
    submitBtn.innerHTML = 'Place Order';
  }
}

/* ==========================================================================
   LIVE ORDER TRACKING
   ========================================================================== */
async function pollActiveOrderStatus(orderId) {
  try {
    const res = await fetch(`${API_BASE}/orders/${orderId}`);
    if (!res.ok) return;
    const orderData = await res.json();
    state.activeOrderData = orderData;
    renderActiveOrderTracking(orderData);
  } catch (err) {
    console.warn('Tracking poll error:', err);
  }
}

function renderActiveOrderTracking(order) {
  const orderNumEl = document.getElementById('tracking-order-num');
  const greetingEl = document.getElementById('tracking-greeting');
  const statusNoteEl = document.getElementById('tracking-status-note');
  const summaryBox = document.getElementById('tracking-summary-box');

  if (orderNumEl) orderNumEl.textContent = `#${order.orderId}`;
  if (greetingEl) greetingEl.textContent = `Thank you, ${order.customerName}!`;

  const status = (order.status || 'Pending').toLowerCase();

  // Status notes
  if (statusNoteEl) {
    if (status === 'pending') {
      statusNoteEl.textContent = 'Your order has been received by our kitchen staff.';
    } else if (status === 'preparing') {
      statusNoteEl.textContent = 'Our chefs are currently preparing your dishes fresh to order.';
    } else if (status === 'ready') {
      statusNoteEl.textContent = `Your dishes are plated and on their way to Table ${order.tableNumber}!`;
    } else if (status === 'completed') {
      statusNoteEl.textContent = 'Your order has been served. Enjoy your meal with us!';
    }
  }

  // Update Stepper Milestones
  const steps = [
    { id: 'step-confirmed', activeOn: ['pending', 'preparing', 'ready', 'completed'] },
    { id: 'step-preparing', activeOn: ['preparing', 'ready', 'completed'] },
    { id: 'step-ready',     activeOn: ['ready', 'completed'] },
    { id: 'step-completed', activeOn: ['completed'] }
  ];

  steps.forEach(s => {
    const el = document.getElementById(s.id);
    if (!el) return;

    el.classList.remove('active', 'completed');
    if (s.activeOn.includes(status)) {
      if (status === s.activeOn[0] && status !== 'completed') {
        el.classList.add('active');
      } else {
        el.classList.add('completed');
      }
    }
  });

  // Render Order Details inside Tracking
  if (summaryBox) {
    const subtotal = order.total || 0;
    const taxes = subtotal * 0.05;
    const grandTotal = subtotal + taxes;

    summaryBox.innerHTML = `
      <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 0.75rem; padding-bottom: 0.5rem; border-bottom: 1px solid var(--border);">
        <span style="font-weight: 700; color: #fff;">Table ${order.tableNumber}</span>
        <span class="status-tag ${status}">${order.status}</span>
      </div>
      <div style="display: flex; flex-direction: column; gap: 0.4rem; margin-bottom: 0.85rem;">
        ${order.items.map(it => `
          <div style="display: flex; justify-content: space-between; font-size: 0.85rem;">
            <span>${it.name} <strong style="color: var(--primary);">× ${it.quantity}</strong></span>
            <span>₹${it.subtotal.toFixed(2)}</span>
          </div>
        `).join('')}
      </div>
      <div style="display: flex; justify-content: space-between; font-size: 1rem; font-weight: 800; color: #fff; padding-top: 0.5rem; border-top: 1px dashed var(--border-light);">
        <span>Total Amount:</span>
        <span style="color: var(--primary); font-family: var(--font-serif);">₹${grandTotal.toFixed(2)}</span>
      </div>
    `;
  }
}

/* ==========================================================================
   RESTAURANT KITCHEN & ADMIN DASHBOARD
   ========================================================================== */
function setupAdminControls() {
  const searchInput = document.getElementById('kitchen-search-input');
  if (searchInput) {
    searchInput.addEventListener('input', (e) => {
      filterKitchenCards(e.target.value.toLowerCase().trim());
    });
  }

  const processNextBtn = document.getElementById('admin-process-next-btn');
  if (processNextBtn) {
    processNextBtn.addEventListener('click', handleKitchenProcessNext);
  }
}

async function fetchKitchenData() {
  try {
    const [ordersRes, statsRes] = await Promise.all([
      fetch(`${API_BASE}/orders`),
      fetch(`${API_BASE}/stats`)
    ]);

    if (ordersRes.ok) {
      const data = await ordersRes.json();
      state.kitchenOrders = [...(data.queue || []), ...(data.completed || [])];
      renderKitchenCards(state.kitchenOrders);
    }

    if (statsRes.ok) {
      const stats = await statsRes.json();
      renderKitchenStats(stats);
    }
  } catch (err) {
    console.error('Kitchen data fetch error:', err);
  }
}

function renderKitchenStats(stats) {
  const setVal = (id, val) => {
    const el = document.getElementById(id);
    if (el) el.textContent = val;
  };

  setVal('admin-stat-total', stats.totalOrders);
  setVal('admin-stat-preparing', stats.preparingOrders);
  setVal('admin-stat-ready', stats.readyOrders);
  setVal('admin-stat-completed', stats.completedOrders);
  setVal('admin-stat-revenue', `₹${stats.totalRevenue.toFixed(2)}`);
}

function renderKitchenCards(orders) {
  const container = document.getElementById('kitchen-orders-grid');
  if (!container) return;

  if (orders.length === 0) {
    container.innerHTML = `<div style="grid-column: 1/-1; text-align: center; color: var(--text-muted); padding: 3rem;">No active kitchen orders.</div>`;
    return;
  }

  container.innerHTML = orders.map((ord, idx) => {
    const isPriority = idx === 0 && ord.status !== 'Completed';
    const status = (ord.status || 'Pending').toLowerCase();

    return `
      <div class="kitchen-order-card ${isPriority ? 'priority' : ''}" data-order-id="${ord.orderId}">
        <div class="k-card-top">
          <div>
            <span class="k-order-num">ORDER #${ord.orderId}</span>
            <div class="k-customer-name">${ord.customerName} • Table ${ord.tableNumber}</div>
          </div>
          <span class="status-tag ${status}">${ord.status}</span>
        </div>

        <div class="k-items-box">
          ${ord.items.map(it => `
            <div class="k-item-line">
              <span>${it.name}</span>
              <strong>× ${it.quantity}</strong>
            </div>
          `).join('')}
        </div>

        <div class="k-footer">
          <div class="k-total">₹${ord.total.toFixed(2)}</div>
          <div style="display: flex; gap: 0.45rem;">
            ${ord.status === 'Pending' ? `
              <button class="btn btn-primary btn-sm" onclick="setOrderStatus(${ord.orderId}, 'Preparing')">
                Start Preparing
              </button>
            ` : ''}
            ${ord.status === 'Preparing' ? `
              <button class="btn btn-secondary btn-sm" style="border-color: var(--status-ready); color: #c084fc;" onclick="setOrderStatus(${ord.orderId}, 'Ready')">
                Mark Ready
              </button>
            ` : ''}
            ${ord.status === 'Ready' ? `
              <button class="btn btn-primary btn-sm" style="background: #10b981;" onclick="setOrderStatus(${ord.orderId}, 'Completed')">
                Complete
              </button>
            ` : ''}
          </div>
        </div>
      </div>
    `;
  }).join('');
}

function filterKitchenCards(query) {
  const filtered = query
    ? state.kitchenOrders.filter(o => 
        o.orderId.toString().includes(query) ||
        o.customerName.toLowerCase().includes(query) ||
        o.tableNumber.toString().includes(query)
      )
    : state.kitchenOrders;
  renderKitchenCards(filtered);
}

window.setOrderStatus = async function(orderId, newStatus) {
  try {
    const res = await fetch(`${API_BASE}/orders/${orderId}/status`, {
      method: 'PUT',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ status: newStatus })
    });
    if (!res.ok) throw new Error('Failed to update order status');
    showToast(`Order #${orderId} updated to ${newStatus}`, 'success');
    fetchKitchenData();
  } catch (err) {
    showToast(err.message, 'error');
  }
};

async function handleKitchenProcessNext() {
  try {
    const res = await fetch(`${API_BASE}/orders/process-next`, {
      method: 'POST'
    });
    if (!res.ok) {
      const err = await res.json();
      throw new Error(err.error || 'Failed to process order');
    }
    const completed = await res.json();
    showToast(`Order #${completed.orderId} served & marked completed!`, 'success');
    fetchKitchenData();
  } catch (err) {
    showToast(err.message, 'error');
  }
}

/* ==========================================================================
   TOAST NOTIFICATIONS
   ========================================================================== */
function showToast(message, type = 'success') {
  let shelf = document.getElementById('toast-shelf');
  if (!shelf) {
    shelf = document.createElement('div');
    shelf.id = 'toast-shelf';
    shelf.className = 'toast-shelf';
    document.body.appendChild(shelf);
  }

  const toast = document.createElement('div');
  toast.className = `toast-msg ${type}`;
  toast.innerHTML = `
    <span>${type === 'success' ? '✓' : '⚠️'}</span>
    <span>${escapeHtml(message)}</span>
  `;

  shelf.appendChild(toast);

  setTimeout(() => {
    toast.style.opacity = '0';
    toast.style.transform = 'translateY(10px)';
    setTimeout(() => toast.remove(), 300);
  }, 3200);
}

function escapeHtml(text) {
  return String(text).replace(/[&<>"']/g, m => ({
    '&': '&amp;',
    '<': '&lt;',
    '>': '&gt;',
    '"': '&quot;',
    "'": '&#039;'
  }[m]));
}
