
import React, { useState } from 'react'
import { useNavigate } from 'react-router-dom'
import api from '../api'
import { saveSession } from '../utils/auth'

export default function Login() {
  const [username, setUsername] = useState('')
  const [password, setPassword] = useState('')
  const [loading, setLoading] = useState(false)
  const navigate = useNavigate()

  async function handleLogin(e) {
    e.preventDefault()
    setLoading(true)
    try {
      const resp = await api.login(username, password)
      // backend now returns user_id and username
      saveSession({ user_id: resp.user_id, username: resp.username })
      navigate('/dashboard')
    } catch (err) {
      alert('Login failed: ' + err.message)
    } finally {
      setLoading(false)
    }
  }

  return (
    <div className="max-w-xl mx-auto mt-12">
      <div className="bg-white p-8 rounded shadow">
        <h1 className="text-2xl font-semibold mb-4">Sign In</h1>
        <form onSubmit={handleLogin} className="space-y-4">
          <div>
            <label className="block text-sm text-slate-600">Username</label>
            <input value={username} onChange={e => setUsername(e.target.value)} className="w-full mt-1 p-2 border rounded" />
          </div>
          <div>
            <label className="block text-sm text-slate-600">Password</label>
            <input type="password" value={password} onChange={e => setPassword(e.target.value)} className="w-full mt-1 p-2 border rounded" />
          </div>
          <div className="flex items-center justify-between">
            <button type="submit" disabled={loading} className="px-4 py-2 bg-teal-500 text-white rounded">{loading ? 'Signing...' : 'Sign In'}</button>
            <a href="/register" className="text-sm text-slate-500">Create account</a>
          </div>
        </form>
      </div>
    </div>
  )
}
