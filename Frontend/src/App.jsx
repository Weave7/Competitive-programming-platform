
import React from 'react'
import { Routes, Route, Navigate } from 'react-router-dom'
import Navbar from './components/Navbar'
import Login from './pages/Login'
import Register from './pages/Register'
import Dashboard from './pages/Dashboard'
import Submit from './pages/Submit'
import { getSession } from './utils/auth'

function RequireAuth({ children }) {
  const sess = getSession()
  if (!sess) return <Navigate to="/" replace />
  return children
}

export default function App() {
  return (
    <div className="min-h-screen">
      <Navbar />
      <main className="max-w-6xl mx-auto p-4">
        <Routes>
          <Route path="/" element={<Login />} />
          <Route path="/register" element={<Register />} />
          <Route path="/dashboard" element={<RequireAuth><Dashboard /></RequireAuth>} />
          <Route path="/submit" element={<RequireAuth><Submit /></RequireAuth>} />
        </Routes>
      </main>
    </div>
  )
}
