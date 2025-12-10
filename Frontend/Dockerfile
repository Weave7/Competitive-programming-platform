# Frontend Dockerfile - React + Vite + Tailwind
# Multi-stage build: build in one stage, serve in another (optimized)

# Stage 1: Build
FROM node:18-alpine AS builder

WORKDIR /app

# Copy package files
COPY package*.json ./

# Install dependencies
RUN npm ci

# Copy source code
COPY . .

# Accept build-time argument for API base URL
ARG VITE_API_BASE=http://localhost:8000
ENV VITE_API_BASE=${VITE_API_BASE}

# Build the React app (Vite output goes to dist/)
RUN npm run build

# Stage 2: Serve (lightweight)
FROM node:18-alpine

WORKDIR /app

# Install a lightweight HTTP server to serve static files
RUN npm install -g serve

# Copy built artifacts from builder stage
COPY --from=builder /app/dist ./dist

# Expose port (frontend typically runs on 3000 or 5173)
EXPOSE 3000

# Set environment variable for API endpoint (can be overridden at runtime)
ENV VITE_API_BASE=http://localhost:8000

# Serve the static build
CMD ["serve", "-s", "dist", "-l", "3000"]
