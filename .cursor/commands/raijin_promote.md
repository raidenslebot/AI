# Raijin promote

Summarize promotion gates and report exactly what is missing. Do not merge or commit; only report.

## Promotion pipeline (dominance gate)

**Promote only if all four are true:**

1. **Implementer pass** — Implementer ran canonical build + canonical test and both passed; provenance JSON written; PR-like summary produced.
2. **Verifier pass** — Verifier independently re-ran canonical build + canonical test and both passed; no banned patterns in the diff; all acceptance criteria MET.
3. **Red Team pass** — Red Team ran at least 2 adversarial checks and the implementation survived (or failures are documented and accepted).
4. **Librarian artifact** — Librarian wrote or updated at least one durable artifact (docs/decisions/, docs/plans/, or docs/provenance/ index).

If any step fails: reject or iterate inside the worktree; do not promote to main.

## What you must do

1. **Gather evidence:** From the current conversation or from repo state, determine whether each gate has been satisfied. Use:
   - Implementer output (build PASS, test PASS, provenance path, PR summary).
   - Verifier output (build PASS, test PASS, banned-pattern CLEAN, acceptance criteria MET).
   - Red Team output (at least 2 adversarial checks, RED TEAM PASS).
   - Librarian output (list of artifacts updated; at least one in docs/decisions, docs/plans, or docs/provenance).
2. **Report in fixed format:**
   - **Implementer:** PASS or MISSING. If MISSING: "Implementer gate missing: &lt;what&gt;" (e.g. "build failed", "no provenance", "no PR summary").
   - **Verifier:** PASS or MISSING. If MISSING: "Verifier gate missing: &lt;what&gt;" (e.g. "Verifier did not re-run", "banned pattern in diff", "acceptance criteria not MET").
   - **Red Team:** PASS or MISSING. If MISSING: "Red Team gate missing: &lt;what&gt;" (e.g. "fewer than 2 adversarial checks", "RED TEAM FAIL").
   - **Librarian:** PASS or MISSING. If MISSING: "Librarian gate missing: &lt;what&gt;" (e.g. "no durable artifact updated").
3. **Verdict:** **PROMOTE** (all four PASS) or **DO NOT PROMOTE** (any MISSING).
4. **If DO NOT PROMOTE:** Print "Next required:" and exactly one line listing the missing gate(s) (e.g. "Next required: Verifier re-run and Red Team 2 checks.").

## Output format

```
Implementer: PASS | MISSING (reason)
Verifier:    PASS | MISSING (reason)
Red Team:    PASS | MISSING (reason)
Librarian:   PASS | MISSING (reason)
---
Verdict: PROMOTE | DO NOT PROMOTE
[If DO NOT PROMOTE] Next required: <one line>
```

Do not run build/test or spawn subagents; only summarize from available evidence and repo state.
