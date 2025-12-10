
const problems = {
  1: {
    title: "The Count is Good Challenge",
    description: `
Use numbers and + - * / to reach a target.
Each number can be used at most once.
Divisions must produce integers.
Output step-by-step operations.

Example:
133
2 3 5 7 10 25
→
25*5=125
125+10=135
135-2=133
`,
  },

  2: {
    title: "Algorithmic Mastermind Challenge",
    description: `
Guess the 4-digit code using oracle feedback:
- Well-placed digits
- Misplaced digits

Goal: Solve in minimum guesses.

Example Input:
1234

Example Output:
Guess 1: 0123 | 1 well-placed, 2 misplaced
...
Solved in 5 guesses.
`,
  },

  3: {
    title: "Robot Race Challenge",
    description: `
Find shortest path from S to T in a grid using BFS.
Output move sequence and grid with path marked.

Example:
5 5
S . . # .
...
`,
  },

  4: {
    title: "Ultimate Sort Challenge",
    description: `
Sort an array with constraints (e.g., max K swaps).
Track comparisons & swaps.
Output final sorted array and stats.

Example:
6 3
5 2 8 1 9 3
`,
  },
};

export default problems;
