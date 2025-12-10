# Competitive Programming Platform - Docker Setup Guide

## Compatibility Check ✓

Frontend and backend are **fully compatible**:
- Backend API: C/Microhttpd server on port 8000
- Frontend: React 18 + Vite + Tailwind CSS on port 3000
- Communication: JSON over HTTP
- Session: User ID returned from `/login` and `/register`, stored client-side

## Prerequisites

- Docker & Docker Compose installed
- `.env` file with Teable credentials (copy from `.env.example`)

## Quick Start

1. **Prepare environment variables:**
   ```bash
   cp .env.example .env
   # Edit .env with your Teable API credentials
   nano .env
   ```

2. **Build and run:**
   ```bash
   docker-compose up --build
   ```

3. **Access the app:**
   - Frontend: http://localhost:3000
   - Backend API: http://localhost:8000

## Service Details

### Backend Service
- **Image**: Built from `./backend/Dockerfile`
- **Port**: 8000 (internally and on host)
- **Container name**: `competitive-backend`
- **Health check**: Verifies `/` endpoint is reachable
- **Dependencies**: Requires Teable API credentials via environment variables

### Frontend Service
- **Image**: Built from `./Frontend/Dockerfile`
- **Port**: 3000 (internally and on host)
- **Container name**: `competitive-frontend`
- **Depends on**: Backend service (waits for health check to pass)
- **API endpoint**: Automatically set to `http://backend:8000` (Docker internal DNS)

## Network Communication

Services communicate via Docker bridge network `competitive-network`:
- Frontend calls backend at `http://backend:8000` (internal DNS)
- Frontend exposes at `http://localhost:3000` (host access)
- Backend exposes at `http://localhost:8000` (host access)

## Environment Variables

See `.env.example` for complete list. Key variables:

**Backend (Teable API):**
- `TEABLE_API_URL`, `TEABLE_API_KEY`
- `TEABLE_USERS_BASE_ID`, `TEABLE_USERS_TABLE_ID`
- `TEABLE_SUBMISSIONS_BASE_ID`, `TEABLE_SUBMISSIONS_TABLE_ID`
- `TEABLE_PROBLEMS_BASE_ID`, `TEABLE_PROBLEMS_TABLE_ID`

**Frontend:**
- `VITE_API_BASE` (default: `http://backend:8000`)

## API Endpoints

| Endpoint | Method | Auth | Request | Response |
|----------|--------|------|---------|----------|
| `/login` | POST | No | `{username, password}` | `{username, message}` |
| `/register` | POST | No | `{username, password}` | `{message}` |
| `/submit` | POST | User ID | `{user_id, problem_id, code}` | `{score, status}` |
| `/dashboard/:user_id` | GET | No | N/A | `{my_subs, top_scores}` |

## Development Notes

- **Frontend build**: Multi-stage Docker build (builder + serve stages)
- **Backend**: Runs directly from compiled C binary
- **Session**: User ID stored in localStorage on client
- **Password hashing**: Backend uses libsodium for secure hashing
- **Database**: Teable (external API, not containerized)

## Troubleshooting

**Backend not starting:**
- Check Teable credentials in `.env`
- Verify Teable API is reachable
- Check logs: `docker logs competitive-backend`

**Frontend can't reach backend:**
- Ensure backend healthcheck passes
- Check browser console for API errors
- Verify `VITE_API_BASE` environment variable

**Port conflicts:**
- Change ports in `docker-compose.yml` if 3000 or 8000 are in use
- Also update `VITE_API_BASE` if backend port changes

**Stop services:**
```bash
docker-compose down
```

**View logs:**
```bash
docker-compose logs -f backend
docker-compose logs -f frontend
```

**Rebuild from scratch:**
```bash
docker-compose down -v
docker-compose up --build
```
