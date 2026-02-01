# Raijin execute

Closed-loop objective execution. Accept an **objective** (user-provided text) and run: proof-of-read → plan-if-large → implement → verify → provenance → commit.

## Objective

1. **Read the objective:** The user supplies an objective (e.g. "Add retry logic to HTTP client" or "Fix regression in resource_governor"). Treat that as the single goal for this run.

2. **Proof-of-read (if new chat):** If this is a new chat or the relevant code has not been read in this session, perform **proof-of-read**: list the files you will touch or depend on and read them (or their key sections) so the agent has grounded context. Output a one-line summary: "Proof-of-read: [N] files read: path1, path2, ...". If context is already present, you may skip and say "Proof-of-read: reusing existing context."

3. **Scope and plan:**
   - If the objective is **larger than ~30 minutes** of work: Create a **plan file** at `docs/plans/<short_slug>.md` with: objective, steps in order, acceptance criteria, and estimated time. Do not implement the full plan in one go; implement the first step(s) that fit in one turn, then verify and update the plan. If the full objective fits in one turn, skip the plan file and go to implementation.
   - If the objective is **~30 minutes or less**: Proceed directly to implementation.

4. **Implementation:** Implement the objective (or the current plan step). Change only what is needed; preserve existing behavior elsewhere.

5. **Verification:** Run the **canonical build** (e.g. `scripts/build.ps1` or `build_raijin.bat`) and the **canonical test** (e.g. `Bin\raijin.exe --self-test` or `test_gauntlet.bat`). If either fails, fix the failure or report the blocker and stop. Do not commit until verification passes (unless the user explicitly asks to commit anyway).

6. **Provenance:** Write or update a **provenance snapshot** at `docs/provenance/<run_id>.json` (e.g. run_id = date-time or objective slug). Include: objective, toolchain or env summary (e.g. compiler, script paths), git revision (e.g. `git rev-parse HEAD`), build result (pass/fail), test result (pass/fail), and list of files changed. Keep the format machine-readable (JSON).

7. **Commit:** Create a single commit with a clear message that references the objective (e.g. "Add retry logic to HTTP client" or "Fix resource_governor regression"). Commit only the files that were changed for this objective (no unrelated changes).

## Output

- Objective (echo)
- Proof-of-read summary
- Plan path (if created) or "No plan file (scope &lt; 30 min)"
- Implementation summary (what was changed)
- Verification result (build + test pass/fail)
- Provenance path
- Commit hash and message

## Rules

- Do not commit without verification pass unless the user explicitly requests it.
- If verification fails, fix first or report the blocker; do not mark the run "done" without passing verification or explicit user override.
- Provenance must be written even if verification fails (record the failure).
- Prefer canonical build/test: `scripts/build.ps1`, `Bin\raijin.exe --self-test` (or project-defined equivalents).
