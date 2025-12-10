
import React, { useState } from 'react'
import api from '../api'
import { getSession } from '../utils/auth'

export default function Submit() {
  const [problemId, setProblemId] = useState('')
  const [code, setCode] = useState('// write your solution here')
  const [loading, setLoading] = useState(false)

  const sess = getSession()

  async function handleSubmit(e) {
    e.preventDefault()
    if (!sess) return alert('Please sign in')
    setLoading(true)
    try {
      const resp = await api.submitSolution({ user_id: sess.user_id, problem_id: problemId, code })
      alert(`Score: ${resp.score}\nStatus: ${resp.status}`)
    } catch (err) {
      alert('Submission failed: ' + err.message)
    } finally {
      setLoading(false)
    }
  }

  return (
    <div className="mt-6">
      <div className="bg-white p-6 rounded shadow grid grid-cols-1 lg:grid-cols-3 gap-6">
        <div className="lg:col-span-2">
          <div className="mb-4">
            <label className="block text-sm text-slate-600">Problem ID</label>
            <input value={problemId} onChange={e => setProblemId(e.target.value)} className="w-full mt-1 p-2 border rounded" placeholder="e.g. prob-42" />
          </div>
          <div>
            <label className="block text-sm text-slate-600 mb-2">Code</label>
            <textarea className="w-full h-96 p-4 rounded code-box" value={code} onChange={e => setCode(e.target.value)} />
          </div>
        </div>
        <div className="flex flex-col gap-4">
          <div className="bg-slate-50 p-4 rounded">
            <div className="text-sm text-slate-600">Language: C (hardcoded)</div>
            <div className="text-xs text-slate-500 mt-2">Time limit and memory limit are handled server-side.</div>
          </div>
          <div>
            <button onClick={handleSubmit} disabled={loading} className="w-full px-4 py-2 bg-teal-500 text-white rounded">{loading ? 'Submitting...' : 'Submit'}</button>
            <div className="text-sm text-slate-500 mt-2">Submission will be sent to your backend `/submit` endpoint.</div>
          </div>
        </div>
      </div>
    </div>
  )
}
