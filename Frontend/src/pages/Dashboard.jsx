
import React, { useEffect, useState } from 'react'
import api from '../api'
import { getSession } from '../utils/auth'

function SubmissionsTable({ items = [] }) {
  if (!items.length) return <div className="text-sm text-slate-500">No submissions yet.</div>
  return (
    <div className="overflow-x-auto">
      <table className="w-full table-auto border-collapse">
        <thead>
          <tr className="text-left">
            <th className="p-2">Problem</th>
            <th className="p-2">Score</th>
            <th className="p-2">Status</th>
          </tr>
        </thead>
        <tbody>
          {items.map(row => (
            <tr key={row.id} className="border-t">
              <td className="p-2 align-top">{row.fields.problem_id}</td>
              <td className="p-2 align-top">{row.fields.score}</td>
              <td className="p-2 align-top">{row.fields.status}</td>
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  )
}

export default function Dashboard() {
  const [mySubs, setMySubs] = useState([])
  const [topScores, setTopScores] = useState([])
  const [loading, setLoading] = useState(true)

  useEffect(() => {
    const sess = getSession()
    if (!sess) return
    setLoading(true)
    api.fetchDashboard(sess.user_id)
      .then(data => {
        setMySubs(data.my_subs || [])
        setTopScores(data.top_scores || [])
      })
      .catch(err => alert('Failed to load dashboard: ' + err.message))
      .finally(() => setLoading(false))
  }, [])

  return (
    <div className="grid grid-cols-1 lg:grid-cols-3 gap-6 mt-6">
      <div className="lg:col-span-2">
        <div className="bg-white p-6 rounded shadow">
          <h2 className="text-xl font-semibold mb-4">My Submissions</h2>
          {loading ? <div>Loading...</div> : <SubmissionsTable items={mySubs} />}
        </div>
      </div>

      <aside className="">
        <div className="bg-white p-6 rounded shadow">
          <h2 className="text-xl font-semibold mb-4">Top Scores</h2>
          <ol className="space-y-3">
            {topScores.length === 0 && !loading && <div className="text-sm text-slate-500">No scores yet.</div>}
            {topScores.map(item => (
              <li key={item.id} className="flex items-center justify-between">
                <div>
                  <div className="text-sm font-medium">{item.fields.username || 'unknown'}</div>
                  <div className="text-xs text-slate-500">{item.fields.problem_id}</div>
                </div>
                <div className="text-lg font-semibold">{item.fields.score}</div>
              </li>
            ))}
          </ol>
        </div>
      </aside>
    </div>
  )
}
