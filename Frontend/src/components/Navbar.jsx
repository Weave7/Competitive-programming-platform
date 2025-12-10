
import React from 'react'
import { Link, useNavigate } from 'react-router-dom'
import { clearSession, getSession } from '../utils/auth'

export default function Navbar() {
  const navigate = useNavigate()
  const sess = getSession()

  function onLogout() {
    clearSession()
    navigate('/')
  }

  return (
    <nav className="bg-white shadow-sm">
      <div className="max-w-6xl mx-auto px-4 py-3 flex items-center justify-between">
        <div className="flex items-center gap-4">
          <Link to="/dashboard" className="font-semibold text-lg text-slate-700">CP Arena</Link>
          <Link to="/dashboard" className="text-sm text-slate-500">Dashboard</Link>
          <Link to="/submit" className="text-sm text-slate-500">Submit</Link>
        </div>
        <div className="flex items-center gap-4">
          {sess ? (
            <>
              <span className="text-sm text-slate-600">{sess.username}</span>
              <button onClick={onLogout} className="px-3 py-1 rounded bg-red-500 text-white text-sm">Logout</button>
            </>
          ) : (
            <>
              <Link to="/" className="text-sm text-slate-600">Sign In</Link>
              <Link to="/register" className="px-3 py-1 rounded bg-slate-100 text-sm">Sign Up</Link>
            </>
          )}
        </div>
      </div>
    </nav>
  )
}
