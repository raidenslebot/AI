# Raijin spawn

Launch a subagent by role. Usage: **/raijin_spawn &lt;role&gt; &lt;objective&gt;**

## Roles

- **verifier** — Read-only verification (build, test, banned-pattern scan, acceptance criteria). Never edits product code.
- **scout** — Read-only exploration (find files, dependencies, invariants, touch set, risks). Never edits.
- **implementer** — Executes one objective in a worktree: edits, build/test, provenance, PR-like summary.
- **redteam** — Adversarial stress: tests that break naive implementations, trigger regressions. At least 2 adversarial checks.
- **librarian** — Memory anchoring: updates docs/plans, docs/decisions, docs/provenance index; records invariants.

## What you must do

1. **Resolve role:** The first argument is the role (verifier | scout | implementer | redteam | librarian). The second argument (and rest of input) is the objective or question.
2. **Load the role contract:** Read the file `docs/agent_roles/<role>.md`. If the file does not exist, report "Unknown role: &lt;role&gt;. Allowed: verifier, scout, implementer, redteam, librarian" and stop.
3. **Follow the contract:** Execute as that role only. Adhere to:
   - Allowed and forbidden actions in the role file.
   - Required output format in the role file.
   - Promotion gate checklist in the role file (for context; promotion is enforced by /raijin_promote).
4. **Worktree / read-only recommendation:**
   - **Implementer** and **Red Team:** Recommend running in a **worktree** (or branch) so main stays clean until promotion. If the user did not specify a worktree, say: "Recommendation: run Implementer/Red Team in a worktree; create with git worktree add ../Raijin-worktrees/&lt;branch&gt; &lt;branch&gt;."
   - **Verifier** and **Scout:** Stay **read-only**; no worktree required. Do not edit product code or docs (Scout/Verifier never write product code; Verifier only reports pass/fail).
   - **Librarian:** May edit only docs (plans, decisions, provenance index). Worktree optional.

## Output

- Echo: "Role: &lt;role&gt;. Objective: &lt;objective&gt;."
- Then produce the **required output** from `docs/agent_roles/<role>.md` in the exact format specified there.
- If the role is implementer or redteam, append one line: "Worktree: recommended (keep main clean until /raijin_promote)."
