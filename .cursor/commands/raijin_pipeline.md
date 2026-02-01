# Raijin pipeline

Run the full multi-agent pipeline for one objective. Usage: **/raijin_pipeline &lt;objective&gt;**

Enforces order: Scout → Implementer (in worktree) → Verifier → Red Team → Librarian → Guard (on main) → Promote. Saves all outputs to docs/run_evidence/ with run_id. If any gate fails, stop with exactly one "Next required:" line.

## Objective

1. **Generate run_id:** Format `YYYYMMDD_HHMMSS_<slug>` where slug is the objective normalized (lowercase, underscores, no slashes). Example: `20260131_143000_bootstrapping_policy`. Use current date/time when the pipeline starts.
2. **Ensure evidence dir:** `docs/run_evidence/` must exist (create if missing).
3. **Run in order; save output; stop on failure.**

   **A) Scout:** Run `/raijin_spawn scout "<objective>"` (follow docs/agent_roles/scout.md). Save the full output to `docs/run_evidence/<run_id>_scout.md`. If Scout fails (e.g. cannot read repo), stop and output "Next required: &lt;one line&gt;."

   **B) Implementer (in worktree):** Create or reuse worktree via `/raijin_worktree <objective_slug>` (slug from objective). Then run `/raijin_spawn implementer "<objective>"` **in that worktree context** (so edits land in the worktree, not main). Save the full output to `docs/run_evidence/<run_id>_implementer.md`. If Implementer fails (build/test FAIL, or no provenance), stop and output "Next required: &lt;one line&gt;."

   **C) Verifier:** Run `/raijin_spawn verifier "Verify <objective> in worktree + main diffs"`. Use the worktree branch diff vs main (or the saved Implementer output) as the change under verification. Save the full output to `docs/run_evidence/<run_id>_verifier.md`. If Verifier verdict is REJECT, stop and output "Next required: &lt;blocking item from Verifier&gt;."

   **D) Red Team:** Run `/raijin_spawn redteam "Adversarial checks for <objective>"`. Save the full output to `docs/run_evidence/<run_id>_redteam.md`. If Red Team verdict is RED TEAM FAIL, stop and output "Next required: &lt;one line from Red Team&gt;."

   **E) Librarian:** Run `/raijin_spawn librarian "Record decision + provenance index for <objective>"`. Save the full output to `docs/run_evidence/<run_id>_librarian.md`. If Librarian did not update at least one durable artifact, stop and output "Next required: Librarian must write or update one artifact in docs/decisions/, docs/plans/, or docs/provenance/."

   **F) Guard on main:** Run `/raijin_guard` from **main** (repo root). Save the full output to `docs/run_evidence/latest_guard.md`. If Guard is FAIL, stop and output "Next required: Guard FAIL; remove all banned patterns until Guard PASS, then re-run Guard."

   **G) Promote:** Run `/raijin_promote`. Use all saved evidence (latest_guard.md, _verifier, _implementer, _redteam, _librarian). Save the full output to `docs/run_evidence/<run_id>_promote.md`. If Verdict is DO NOT PROMOTE, output "Next required: &lt;one line from promote output&gt;."

4. **If all gates passed:** Output "Pipeline complete. run_id: &lt;run_id&gt;. Evidence: docs/run_evidence/&lt;run_id&gt;_*.md, latest_guard.md, &lt;run_id&gt;_promote.md. Merge worktree branch to main when ready."

## Output format

- run_id
- Per-step: saved path (e.g. docs/run_evidence/&lt;run_id&gt;_scout.md)
- On failure: "Next required: &lt;exactly one line&gt;"
- On success: "Pipeline complete. run_id: ... Evidence: ... Merge worktree branch to main when ready."

## Rules

- Implementer must run in the worktree (not main). Use the worktree path from /raijin_worktree when running Implementer (e.g. open or cd to that path so edits land there).
- Guard and Promote are evaluated from main (repo root); evidence files are written to main's docs/run_evidence/.
- Do not merge the worktree branch to main inside this command; only report that the user can merge when ready.
