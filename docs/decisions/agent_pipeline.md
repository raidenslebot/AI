# Agent pipeline: worktrees and evidence

**Status:** Accepted  
**Date:** 2026-01-31

## Why worktrees are mandatory

- **No direct work on main:** Objective work (Implementer, Red Team edits) must happen in a **worktree** so main stays clean until promotion. This prevents drift and makes "merge when gates pass" deterministic.
- **Isolation:** Each objective gets a branch and working tree (e.g. `.worktrees/<slug>`, branch `raijin/<slug>`). Build/test run in that tree; main is only updated via merge after /raijin_promote passes.
- **Parallelism:** Best-of-N runs use separate worktrees (`.worktrees/<slug>_1` … `_N`) so multiple Implementer attempts do not conflict.

## Evidence files (docs/run_evidence/)

All pipeline outputs are written under `docs/run_evidence/` so promotion and audit have a single source of truth.

| File | Purpose |
|------|--------|
| `<run_id>_scout.md` | Scout output: relevant files, touch set, risks. |
| `<run_id>_implementer.md` | Implementer output: files changed, build/test, provenance path, PR summary. |
| `<run_id>_verifier.md` | Verifier output: build/test, GuardStatus, AllowlistCoverage, verdict. |
| `<run_id>_redteam.md` | Red Team output: adversarial checks, verdict. |
| `<run_id>_librarian.md` | Librarian output: artifacts updated, invariants. |
| `latest_guard.md` | Most recent /raijin_guard output (used by /raijin_promote as Gate 0 evidence). |
| `<run_id>_promote.md` | /raijin_promote output: Guard, Implementer, Verifier, Red Team, Librarian gates and verdict. |

If any gate fails, the pipeline stops and writes "Next required:"; the user fixes and re-runs or continues from the failed step.

## Run_id naming convention

**Format:** `YYYYMMDD_HHMMSS_<slug>`

- **YYYYMMDD:** Date when the pipeline run started (e.g. 20260131).
- **HHMMSS:** Time when the run started (e.g. 143000).
- **slug:** Objective normalized to a path-safe segment (lowercase, underscores for spaces, no slashes). Example: `bootstrapping_policy`, `refactor_internet_acquisition`.

Example: `20260131_143000_bootstrapping_policy`.

This keeps evidence files unique and sortable by time.

## Commands that use this

- **/raijin_worktree &lt;slug&gt;** — Creates or reuses `.worktrees/<slug>`, branch `raijin/<slug>`. Never edits main.
- **/raijin_pipeline &lt;objective&gt;** — Scout → Implementer (in worktree) → Verifier → Red Team → Librarian → Guard (on main) → Promote; saves all outputs to docs/run_evidence/ with run_id.
- **/raijin_bestof &lt;N&gt; &lt;objective&gt;** — N parallel Implementer attempts in `.worktrees/<slug>_1` … `_N`; Verifier picks best; merge instructions printed.
