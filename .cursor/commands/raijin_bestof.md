# Raijin bestof

Run N parallel Implementer attempts in separate worktrees; Verifier picks the best. Usage: **/raijin_bestof &lt;N&gt; &lt;objective&gt;**

Example: `/raijin_bestof 3 "Refactor InternetAcquisition to real implementation and tests"` runs 3 attempts in .worktrees/&lt;slug&gt;_1, _2, _3; then Verifier compares and selects one.

## Objective

1. **Resolve N and objective:** N is a positive integer (e.g. 2 or 3). Objective is the implementation goal.
2. **Slug:** Normalize objective to a path-safe slug (e.g. `refactor_internet_acquisition`).
3. **Create N worktrees:** For i in 1..N: create or reuse `.worktrees/<slug>_<i>` with branch `raijin/<slug>_<i>`. Use `/raijin_worktree <slug>_<i>` or equivalent (git worktree add .worktrees/&lt;slug&gt;_i -b raijin/&lt;slug&gt;_i).
4. **Run N Implementer attempts (parallel if possible, else sequential):** In each worktree i, run `/raijin_spawn implementer "<objective>"` so that implementation lands in that worktree. Each attempt must produce:
   - Provenance JSON (docs/provenance/&lt;run_id&gt;_i.json in that worktree or copy to main with attempt id)
   - Build PASS/FAIL
   - Test PASS/FAIL
   - PR-like summary and list of files changed.
   Save each Implementer output to `docs/run_evidence/<run_id>_implementer_<i>.md` (run_id = YYYYMMDD_HHMMSS_&lt;slug&gt;).
5. **Verifier comparison:** Run Verifier (or equivalent comparison) over all N attempts. Compare by:
   - **Build/test pass:** Prefer attempts that passed both.
   - **Minimal diff footprint:** Prefer smaller file-change count or smaller line delta vs main.
   - **No banned patterns introduced:** Prefer attempts with no new FAIL_BLOCKER in full-repo scan (or allowlisted).
   Select the **best** attempt (e.g. first that passes build+test and has no new banned patterns; tie-break by minimal diff). If none pass, select the one with fewest failures or report "No winning attempt; fix and re-run."
6. **Output:** Print:
   - **Selected attempt:** &lt;i&gt; (worktree .worktrees/&lt;slug&gt;_i, branch raijin/&lt;slug&gt;_i).
   - **Why:** One-line reason (e.g. "Only attempt with build+test pass and no new banned patterns; minimal diff.").
   - **How to apply/merge to main:** `git checkout main`, `git merge raijin/<slug>_<i>` (or `git worktree remove .worktrees/<slug>_<i>` after merge), and run Guard + Promote from main.

## Rules

- Each attempt must write provenance (mandatory); otherwise that attempt is disqualified from "best."
- Verifier does not edit code; only compares outputs and selects by the criteria above.
- Do not merge to main inside this command; only report the selected attempt and merge instructions.
