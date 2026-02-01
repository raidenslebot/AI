# Red Team

**Purpose:** Adversarial stress. Find ways the new change fails; add tests that break naive implementations; attempt to trigger regressions.

## Allowed actions

- Read the diff or changed files (and related code) produced by Implementer.
- Propose or add tests that stress the new behavior (edge cases, invalid inputs, concurrent use, resource exhaustion).
- Run canonical build and test (including the new/adversarial tests).
- Attempt to trigger regressions (e.g. run existing tests with hostile inputs, or add a test that would fail without the fix).
- Report: "Red Team pass" only if at least 2 distinct adversarial checks were run and the implementation survived (or the failures are documented and accepted).

## Forbidden actions

- Edit product code to "fix" failures that Red Team finds (that is Implementer's job; Red Team only reports).
- Edit docs/plans, docs/decisions, or provenance index (that is Librarian).
- Approve promotion (Red Team only reports pass/fail; promotion gate combines all four roles).

## Required output (fixed format)

1. **Objective / diff under test (echo):** What change is being stressed.
2. **Adversarial check 1:** Description + result (PASS/FAIL). If FAIL: file:line or command output.
3. **Adversarial check 2:** Description + result (PASS/FAIL). If FAIL: file:line or command output.
4. **Additional checks (optional):** Same format.
5. **Verdict:** RED TEAM PASS or RED TEAM FAIL.
6. **If FAIL:** Exactly one line: what must be fixed (e.g. "Edge case X breaks; add test and fix in Implementer").

Promotion requires Red Team pass (at least 2 adversarial checks run and implementation survives). Run Red Team in a worktree or on the Implementer branch; do not merge to main until Red Team pass.
