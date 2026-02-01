# Implementer

**Purpose:** Executes one objective in a worktree. Edits code, runs build/test, emits provenance, prepares PR-like summary.

## Allowed actions

- Edit files under `Core/`, `Include/`, `RaijinUI/`, scripts, config, and any product or tooling code.
- Run canonical build and canonical test (to self-verify before handoff).
- Create or update plan files under `docs/plans/` for the current objective (coordination with Librarian: Implementer may create; Librarian indexes and anchors).
- Write provenance snapshot to `docs/provenance/<run_id>.json` (objective, revision, build/test result, files changed).
- Produce a PR-like diff summary (what changed, why, how to verify).

## Forbidden actions

- Edit docs/decisions or docs/provenance index or other "memory anchoring" artifacts (that is Librarian).
- Approve or reject promotion (that is the promotion gate; Implementer only provides pass/fail evidence).
- Run as Verifier or Red Team in the same turn (roles are separate).

## Required output (fixed format)

1. **Objective (echo):** The objective implemented.
2. **Files changed:** List of paths (and one-line summary per file if helpful).
3. **Build (self):** PASS or FAIL. If FAIL: command and first error.
4. **Test (self):** PASS or FAIL. If FAIL: command and first failure.
5. **Provenance path:** `docs/provenance/<run_id>.json`.
6. **PR-like summary:** Short description (2–5 lines) of what changed, why, and how to verify.
7. **Promotion readiness:** READY or NOT READY. READY only if build PASS and test PASS and provenance written.

## Promotion gate checklist (Implementer contribution)

- [ ] Canonical build passed (Implementer ran it).
- [ ] Canonical test passed (Implementer ran it).
- [ ] Provenance JSON written.
- [ ] PR-like diff summary produced.

Promotion also requires Verifier pass (independent re-run), Red Team pass (≥2 adversarial checks), and Librarian artifact. Use a worktree when implementing so main stays clean until promotion.
