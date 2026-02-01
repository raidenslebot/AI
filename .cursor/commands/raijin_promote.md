# Raijin promote

Summarize promotion gates and report exactly what is missing. Do not merge or commit; only report.

## Promotion pipeline (dominance gate)

**Gate 0 — Guard PASS (required):** Promotion is blocked unless /raijin_guard is PASS. There is no allowlist escape. If Guard is FAIL, Verdict must be DO NOT PROMOTE.

**Gates 1–4 — Promote only if all are true:**

1. **Implementer pass** — Implementer ran canonical build + canonical test and both passed; provenance JSON written (mandatory, even if no files changed); PR-like summary produced.
2. **Verifier pass** — Verifier independently re-ran canonical build + canonical test and both passed; full-repo banned-pattern scan (or /raijin_guard output cited) shows PASS; GuardStatus PASS; all acceptance criteria MET.
3. **Red Team pass** — Red Team ran at least 2 adversarial checks and the implementation survived (or failures are documented and accepted).
4. **Librarian artifact** — Librarian wrote or updated at least one durable artifact (docs/decisions/, docs/plans/, or docs/provenance/ index).

If any step fails: reject or iterate inside the worktree; do not promote to main.

## Evidence inputs

- **Guard:** Read `docs/run_evidence/latest_guard.md` if it exists (pasted or recorded Guard output). If missing or stale, instruct the user to run /raijin_guard and paste the results (or save to docs/run_evidence/latest_guard.md). Use Guard output to determine: Guard PASS/FAIL. Promotion requires Guard PASS.
- **Implementer:** Build PASS, test PASS, provenance path (must exist even for no-op runs).
- **Verifier:** Build PASS, test PASS, GuardStatus PASS, acceptance criteria MET.
- **Red Team:** At least 2 adversarial checks, RED TEAM PASS.
- **Librarian:** At least one artifact in docs/decisions, docs/plans, or docs/provenance.

## What you must do

1. **Gather evidence:** From the current conversation, docs/run_evidence/latest_guard.md, or repo state, determine whether each gate is satisfied.
2. **Gate 0 first:** If Guard is FAIL, Verdict is DO NOT PROMOTE; state "Guard FAIL; run /raijin_guard and remove all banned patterns until Guard PASS."
3. **Report in fixed format:**
   - **Guard (Gate 0):** PASS or FAIL.
   - **Implementer:** PASS or MISSING. If MISSING: "Implementer gate missing: <what>."
   - **Verifier:** PASS or MISSING. If MISSING: "Verifier gate missing: <what>."
   - **Red Team:** PASS or MISSING. If MISSING: "Red Team gate missing: <what>."
   - **Librarian:** PASS or MISSING. If MISSING: "Librarian gate missing: <what>."
4. **Verdict:** **PROMOTE** (Gate 0 + all four PASS) or **DO NOT PROMOTE** (any MISSING or Guard FAIL).
5. **If DO NOT PROMOTE:** Print "Next required:" and exactly one line.

## Output format

```
Guard (Gate 0): PASS | FAIL
Implementer:    PASS | MISSING (reason)
Verifier:       PASS | MISSING (reason)
Red Team:       PASS | MISSING (reason)
Librarian:      PASS | MISSING (reason)
---
Verdict: PROMOTE | DO NOT PROMOTE
[If DO NOT PROMOTE] Next required: <one line>
```

Do not run build/test or spawn subagents; only summarize from available evidence and repo state.
