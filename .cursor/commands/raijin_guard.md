# Raijin guard

Drift + integrity check. Output is **PASS** or **FAIL** with the next required fix. Every banned-pattern match is **FAIL_BLOCKER**. PASS only when zero matches.

## Objective

1. **Git status:** Run `git status` (and optionally `git diff --stat`) in the project root. Report: branch, clean/dirty, list of modified/untracked/deleted files. Flag if uncommitted changes look like unfinished work (e.g. many modified Core/ files with no commit).

2. **Banned-pattern scan:** Scan the repository for **banned patterns** over tracked source (Core/, Include/, RaijinUI/, *.py, *.js under project; exclude node_modules, Bin, obj). Search for (case-insensitive where sensible): `TODO`, `FIXME`, `XXX`, `HACK`, `mock`, `demo`, `simulated`, `placeholder`, `stub`, `"for now"`, `to be implemented`, `not yet implemented`, `example` (in comment context suggesting placeholder), ellipsis-in-code (`...`). For each match: report file path, line number, and matched snippet or tag. **Every match is FAIL_BLOCKER.** Do not consult any allowlist.

3. **Canonical build/test:** Identify the **canonical build** command and **canonical test** (or minimal verification) command. Check for: `scripts/build.ps1`, `build_raijin.bat`, `dotnet build`, `npm run build`, or a single documented build; and for `Bin\raijin.exe --self-test`, `test_gauntlet.bat`, `pytest`, `npm test`, or a single documented test. If **either** is missing or not clearly defined, **declare missing** and state what must be added.

4. **Verdict and next fix:**
   - **PASS** only if: **zero** banned-pattern matches, git is clean or only expected changes, and both build and test commands are identified (or created). Output **PASS** and "No fix required."
   - **FAIL** if any banned-pattern match exists, or build/test missing, or git state requires action. Output **FAIL** and exactly **one** "Next required fix:" line: the single highest-priority action (e.g. "Remove banned pattern in Core/X.cpp:L42", "Add scripts/test.ps1 and document in README").

## Output format

- Git status summary
- Banned-pattern scan results: each match as **FAIL_BLOCKER** (or "No banned patterns in scanned paths.")
- Build command and path (or "missing")
- Test command and path (or "missing")
- **PASS** or **FAIL**
- **Next required fix:** (one line, or "No fix required" if PASS)

## Rules

- Scan only tracked source and config (no node_modules, Bin, obj).
- No allowlist. Every banned-pattern match is FAIL_BLOCKER. Guard does not read or reference docs/allowlists/ or any allowlist file.
- If you create a script to satisfy build/test, keep it minimal and runnable (no placeholders).
- Canonical build: prefer `scripts/build.ps1` or root `build.ps1`.
