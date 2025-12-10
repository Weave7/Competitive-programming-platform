const API = import.meta.env.VITE_API_BASE || 'http://localhost:8000'

async function request(path, opts = {}) {
  const res = await fetch(API + path, opts)
  if (!res.ok) {
    const text = await res.text()
    throw new Error(text || res.statusText)
  }
  // Some endpoints (like register) return no JSON body on error; handle empty
  const contentType = res.headers.get('content-type') || ''
  if (contentType.includes('application/json')) return res.json()
  return null
}

export async function login(username, password) {
  return request('/login', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ username, password }),
  })
}

export async function register(username, password) {
  return request('/register', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ username, password }),
  })
}

export async function submitSolution({ user_id, problem_id, code }) {
  return request('/submit', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ user_id, problem_id, code }),
  })
}

export async function fetchDashboard(user_id) {
  return request(`/dashboard/${user_id}`)
}

export default { login, register, submitSolution, fetchDashboard }
