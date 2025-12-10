# Competitive Programming SPA

Vite + React + Tailwind frontend wired to your C backend running at `http://localhost:8000`.

## Quick start

1. Install dependencies:

```bash
npm install
```

2. Start dev server:

```bash
npm run dev
```

3. Open `http://localhost:5173` and use the Sign In / Sign Up screens.

## Notes

- Login expects the backend `/login` to return JSON: `{ "username": "...", "user_id": "...", "message": "Logged in" }`.
- Submit will POST `{ user_id, problem_id, code }` to `/submit` and show the returned `{ score, status }`.
- Dashboard calls `GET /dashboard/:user_id` and expects `{ my_subs, top_scores }` in the exact format returned by your C backend.

If your backend host or port differ, set the `VITE_API_BASE` env var when running.
```

---

End of project files.

/*
  IMPORTANT: This code is built to match your backend as-is (no backend changes required).
  - It expects `user_id` returned on login and stores it in localStorage.
  - It uses `GET /dashboard/:user_id` and `POST /submit` exactly as implemented.
*/
