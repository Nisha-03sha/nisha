/**
 * ============================================================================
 * RESTAURANT ORDER MANAGEMENT SYSTEM - FULL-STACK ENTRY POINT
 * Spawns Native C Backend (Queue & Linked Lists) and mounts Express + Vite
 * ============================================================================
 */

import express from 'express';
import { createServer as createViteServer } from 'vite';
import http from 'http';
import { spawn, execSync, ChildProcess } from 'child_process';
import path from 'path';
import fs from 'fs';

const app = express();
const PORT = 3000;
const C_SERVER_PORT = 5050;

let cProcess: ChildProcess | null = null;
const cLogs: string[] = [];

// Helper to log with timestamp
function log(msg: string) {
  const timestamp = new Date().toISOString().split('T')[1].slice(0, 8);
  console.log(`[${timestamp}] [NODE PROXY] ${msg}`);
}

// 1. Build and Start the Native C Backend Server
function ensureCBackendServer() {
  const backendDir = path.resolve(process.cwd(), 'restaurant', 'backend');
  const binaryName = process.platform === 'win32' ? 'server.exe' : 'server';
  const binaryPath = path.join(backendDir, binaryName);

  // If binary doesn't exist, compile it using make or gcc
  if (!fs.existsSync(binaryPath)) {
    log('C server binary not found. Compiling with gcc...');
    try {
      execSync('make clean && make', { cwd: backendDir, stdio: 'inherit' });
      log('C server compilation succeeded.');
    } catch (err) {
      log(`Make failed, attempting direct gcc: ${err}`);
      const linkFlag = process.platform === 'win32' ? '-lws2_32' : '';
      execSync(`gcc -Wall -Wextra -O2 -std=c99 main.c server.c queue.c linkedlist.c order.c -o ${binaryName} ${linkFlag}`, {
        cwd: backendDir,
        stdio: 'inherit',
      });
      log('Direct gcc compilation succeeded.');
    }
  }

  // Spawn C Backend Server Process
  log(`Spawning C backend server process on port ${C_SERVER_PORT}...`);
  cProcess = spawn(binaryPath, [C_SERVER_PORT.toString()], {
    cwd: backendDir,
    stdio: ['ignore', 'pipe', 'pipe'],
  });

  cProcess.stdout?.on('data', (data) => {
    const lines = data.toString().split('\n').filter(Boolean);
    for (const line of lines) {
      console.log(`[C-BACKEND] ${line}`);
      cLogs.push(line);
      if (cLogs.length > 200) cLogs.shift();
    }
  });

  cProcess.stderr?.on('data', (data) => {
    const lines = data.toString().split('\n').filter(Boolean);
    for (const line of lines) {
      console.error(`[C-BACKEND ERR] ${line}`);
      cLogs.push(`[ERROR] ${line}`);
      if (cLogs.length > 200) cLogs.shift();
    }
  });

  cProcess.on('exit', (code, signal) => {
    log(`C backend process exited with code ${code}, signal ${signal}`);
  });

  // Ensure cleanup on termination
  process.on('SIGINT', cleanupAndExit);
  process.on('SIGTERM', cleanupAndExit);
  process.on('exit', () => {
    if (cProcess) {
      cProcess.kill();
    }
  });
}

function cleanupAndExit() {
  log('Shutting down C backend server...');
  if (cProcess) {
    cProcess.kill('SIGTERM');
  }
  process.exit(0);
}

// 2. Start Express & Vite
async function startServer() {
  ensureCBackendServer();

  // Wait 300ms for C socket to bind
  await new Promise((resolve) => setTimeout(resolve, 400));

  // Express JSON parser for fallback routes
  app.use(express.json());

  // Status & Logs routes
  app.get('/api/c-server/status', (_req, res) => {
    res.json({
      status: 'active',
      port: C_SERVER_PORT,
      pid: cProcess?.pid || null,
      platform: process.platform,
    });
  });

  app.get('/api/c-server/logs', (_req, res) => {
    res.json(cLogs);
  });

  // Proxy all other /api/* calls to native C Backend
  app.all('/api/*', (req, res) => {
    const options: http.RequestOptions = {
      hostname: '127.0.0.1',
      port: C_SERVER_PORT,
      path: req.originalUrl,
      method: req.method,
      headers: {
        ...req.headers,
        host: `127.0.0.1:${C_SERVER_PORT}`,
      },
    };

    const proxyReq = http.request(options, (proxyRes) => {
      res.writeHead(proxyRes.statusCode || 200, proxyRes.headers);
      proxyRes.pipe(res);
    });

    proxyReq.on('error', (err) => {
      log(`Proxy error to C backend: ${err.message}`);
      res.status(502).json({
        error: 'C Backend Server is initializing or temporarily unreachable',
        details: err.message,
      });
    });

    if (req.body && Object.keys(req.body).length > 0) {
      const bodyData = JSON.stringify(req.body);
      proxyReq.setHeader('Content-Length', Buffer.byteLength(bodyData));
      proxyReq.write(bodyData);
    }

    proxyReq.end();
  });

  // Serve static files from restaurant/frontend
  const frontendDir = path.resolve(process.cwd(), 'restaurant', 'frontend');
  app.use('/static-frontend', express.static(frontendDir));

  // Mount Vite Middleware for development
  if (process.env.NODE_ENV !== 'production') {
    const vite = await createViteServer({
      server: { middlewareMode: true },
      appType: 'spa',
    });
    app.use(vite.middlewares);
  } else {
    const distDir = path.resolve(process.cwd(), 'dist');
    if (fs.existsSync(distDir)) {
      app.use(express.static(distDir));
    }
  }

  app.listen(PORT, '0.0.0.0', () => {
    log(`Restaurant Order Management System running on http://localhost:${PORT}`);
    log(`C Backend Server listening on http://127.0.0.1:${C_SERVER_PORT}`);
  });
}

startServer().catch((err) => {
  console.error('Fatal server startup error:', err);
});
