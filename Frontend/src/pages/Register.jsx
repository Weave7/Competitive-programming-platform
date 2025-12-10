
import React, { useState } from 'react'
import { useNavigate } from 'react-router-dom'
import api from '../api'

export default function Register() {
  const [username, setUsername] = useState('')
  const [password, setPassword] = useState('')
  const [loading, setLoading] = useState(false)
  const navigate = useNavigate()

  async function handleRegister(e) {
    e.preventDefault()
    setLoading(true)
    try {
      await api.register(username, password)
      alert('Registered successfully! Please sign in.')
      navigate('/')
    } catch (err) {
      alert('Registration failed: ' + err.message)
    } finally {
      setLoading(false)
    }
  }

  return (
    <div className="max-w-xl mx-auto mt-12">
      <div className="bg-white p-8 rounded shadow">
        <h1 className="text-2xl font-semibold mb-4">Sign Up</h1>
        <form onSubmit={handleRegister} className="space-y-4">
          <div>
            <label className="block text-sm text-slate-600">Username</label>
            <input value={username} onChange={e => setUsername(e.target.value)} className="w-full mt-1 p-2 border rounded" />
          </div>
          <div>
            <label className="block text-sm text-slate-600">Password</label>
            <input type="password" value={password} onChange={e => setPassword(e.target.value)} className="w-full mt-1 p-2 border rounded" />
          </div>
          <div className="flex items-center justify-between">
            <button type="submit" disabled={loading} className="px-4 py-2 bg-slate-700 text-white rounded">{loading ? 'Creating...' : 'Create Account'}</button>
            <a href="/" className="text-sm text-slate-500">Already have an account?</a>
          </div>
        </form>
      </div>
    </div>
  )
}
