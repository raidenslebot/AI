# Raijin guard

Drift + integrity check. Output is **PASS** or **FAIL** with the next required fix. Banned-pattern findings are **FAIL_BLOCKER** unless allowlisted, then **WARN_ALLOWLISTED**.

## Objective

1. **Git status:** Run `git status` (and optionally `git diff --stat`) in the project root. Report: branch, clean/dirty, list of modified/untracked/deleted files. Flag if uncommitted changes look like unfinished work (e.g. many modified Core/ files with no commit).

2. **Banned-pattern scan:** Scan the repository for **banned patterns** over tracked source (Core/, Include/, RaijinUI/, *.py, *.js under project; exclude node_modules, Bin, obj). Search for (case-insensitive where sensible): `TODO`, `FIXME`, `XXX`, `HACK`, `mock`, `demo`, `simulated`, `placeholder`, `stub`, `"for now"`, `to be implemented`, `not yet implemented`, `example` (in comment context suggesting placeholder), ellipsis-in-code (`...`). For each match: report file path, line number, and matched snippet or tag.

3. **Allowlist check:** Read `docs/allowlists/banned_patterns_allowlist.yml`. For each banned-pattern match:
   - If the match is **allowlisted** (file path and phrase match an entry in the allowlist): mark as **WARN_ALLOWLISTED** and note "ALLOWLISTED" beside it.
   - If the match is **not allowlisted**: mark as **FAIL_BLOCKER**.
   Report: list each match with status **FAIL_BLOCKER** or **WARN_ALLOWLISTED**. If no matches, report "No banned patterns in scanned paths."

4. **Canonical build/test:** Identify the **canonical build** command and **canonical test** (or minimal verification) command. Check for: `scripts/build.ps1`, `build_raijin.bat`, `dotnet build`, `npm run build`, or a single documented build; and for `Bin\raijin.exe --self-test`, `test_gauntlet.bat`, `pytest`, `npm test`, or a single documented test. If **either** is missing or not clearly defined, **declare missing** and state what must be added.

5. **Verdict and next fix:**
   - **PASS** only if: zero **FAIL_BLOCKER** (all banned-pattern matches are allowlisted or none found), git is clean or only expected changes, and both build and test commands are identified (or created). Output **PASS** and "No fix required."
   - **FAIL** if any **FAIL_BLOCKER** exists, or build/test missing, or git state requires action. Output **FAIL** and exactly **one** "Next required fix:" line: the single highest-priority action (e.g. "Remove or allowlist banned pattern in Core/X.cpp:L42", "Add missing entry to docs/allowlists/banned_patterns_allowlist.yml", "Add scripts/test.ps1 and document in README").

## Output format

- Git status summary
- Banned-pattern scan results: each match with **FAIL_BLOCKER** or **WARN_ALLOWLISTED** (or "none")
- Build command and path (or "missing")
- Test command and path (or "missing")
- **PASS** or **FAIL**
- **Next required fix:** (one line, or "No fix required" if PASS)

## Rules

- Scan only tracked source and config (no node_modules, Bin, obj).
- Allowlist path: `docs/allowlists/banned_patterns_allowlist.yml`. Format described in `docs/allowlists/README.md`.
- If you create a script to satisfy build/test, keep it minimal and runnable (no placeholders).
- Canonical build: prefer `scripts/build.ps1` or root `build.ps1`.
