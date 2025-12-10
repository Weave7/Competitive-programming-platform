export function saveSession({ user_id, username }) {
  localStorage.setItem('user_id', user_id)
  localStorage.setItem('username', username)
}

export function clearSession() {
  localStorage.removeItem('user_id')
  localStorage.removeItem('username')
}

export function getSession() {
  const user_id = localStorage.getItem('user_id')
  const username = localStorage.getItem('username')
  return user_id ? { user_id, username } : null
}

