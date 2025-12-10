
import React, { useState } from "react";
import api from "../api";
import { getSession } from "../utils/auth";
import problems from "../data/problems"; // ← NEW IMPORT

export default function Submit() {
  const [problemId, setProblemId] = useState("");
  const [code, setCode] = useState("// write your solution here");
  const [loading, setLoading] = useState(false);

  const sess = getSession();

  async function handleSubmit(e) {
    e.preventDefault();
    if (!sess) return alert("Please sign in");
    if (!problemId) return alert("Enter a problem ID");

    setLoading(true);
    try {
      const resp = await api.submitSolution({
        user_id: sess.user_id,
        problem_id: parseInt(problemId),
        code,
      });

      alert(`Score: ${resp.score}\nStatus: ${resp.status}`);
    } catch (err) {
      alert("Submission failed: " + err.message);
    } finally {
      setLoading(false);
    }
  }

  const problem = problems[problemId]; // ← automatically loads description

  return (
    <div className="mt-6">
      <div className="bg-white p-6 rounded shadow grid grid-cols-1 lg:grid-cols-3 gap-6">

        {/* LEFT: Problem description */}
        <div className="lg:col-span-1">
          <h2 className="text-xl font-semibold mb-4">Problem Description</h2>

          {problem ? (
            <div className="bg-slate-50 p-4 rounded text-sm whitespace-pre-wrap">
              <h3 className="font-bold mb-2">{problem.title}</h3>
              {problem.description}
            </div>
          ) : (
            <div className="text-slate-500 text-sm">
              Enter Problem ID (1–4) to view challenge details.
            </div>
          )}
        </div>

        {/* MIDDLE + RIGHT: Editor and submit */}
        <div className="lg:col-span-2">
          <div className="mb-4">
            <label className="block text-sm text-slate-600">Problem ID</label>
            <input
              value={problemId}
              onChange={(e) => setProblemId(e.target.value)}
              className="w-full mt-1 p-2 border rounded"
              placeholder="Enter 1, 2, 3, or 4"
            />
          </div>

          <div>
            <label className="block text-sm text-slate-600 mb-2">Code</label>
            <textarea
              className="w-full h-96 p-4 rounded code-box"
              value={code}
              onChange={(e) => setCode(e.target.value)}
            />
          </div>

          <div className="flex gap-4 mt-6">
            <div className="bg-slate-50 p-4 rounded">
              <div className="text-sm text-slate-600">Language: C (hardcoded)</div>
              <div className="text-xs text-slate-500 mt-2">
                Time limit & memory limit handled server-side.
              </div>
            </div>

            <div className="flex-1">
              <button
                onClick={handleSubmit}
                disabled={loading}
                className="w-full px-4 py-2 bg-teal-500 text-white rounded"
              >
                {loading ? "Submitting..." : "Submit"}
              </button>
              <div className="text-sm text-slate-500 mt-2">
                Submission goes to backend <code>/submit</code>
              </div>
            </div>
          </div>
        </div>

      </div>
    </div>
  );
}
