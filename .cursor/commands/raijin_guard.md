# Raijin guard

Drift + integrity check. Output is **PASS** or **FAIL** with the next required fix.

## Objective

1. **Git status:** Run `git status` (and optionally `git diff --stat`) in the project root. Report: branch, clean/dirty, list of modified/untracked/deleted files. Flag if uncommitted changes look like unfinished work (e.g. many modified Core/ files with no commit).

2. **Banned-pattern scan:** Scan the repository for **banned patterns** over tracked source (Core/, Include/, RaijinUI/, *.py, *.js under project; exclude node_modules, Bin, obj). Search for (case-insensitive where sensible): `TODO`, `FIXME`, `XXX`, `HACK`, `mock`, `demo`, `simulated`, `placeholder`, `stub`, `"for now"`, `to be implemented`, `not yet implemented`, `example` (in comment context suggesting placeholder), ellipsis-in-code (`...`). Report: file path, line number (or count per file), and matched snippet or tag. If none found, report "No banned patterns in scanned paths."

3. **Canonical build/test:** Identify the **canonical build** command and **canonical test** (or minimal verification) command. Check for: `scripts/build.ps1`, `build_raijin.bat`, `dotnet build`, `npm run build`, or a single documented build; and for `Bin\raijin.exe --self-test`, `test_gauntlet.bat`, `pytest`, `npm test`, or a single documented test. If **either** is missing or not clearly defined, **declare missing** and state what must be added (e.g. "Missing: test entrypoint; add scripts/test.ps1 or document Bin\raijin.exe --self-test").

4. **Verdict and next fix:**
   - If git is clean (or only expected changes), no banned patterns in scanned paths, and both build and test commands are identified (or created): output **PASS** and "No fix required."
   - Otherwise output **FAIL** and exactly **one** "Next required fix:" line: the single highest-priority action (e.g. "Remove or replace banned pattern in Core/X.cpp:L42", "Add scripts/test.ps1 and document in README", "Commit or revert uncommitted Core/ changes").

## Output format

- Git status summary
- Banned-pattern scan results (or "none")
- Build command and path (or "missing")
- Test command and path (or "missing")
- **PASS** or **FAIL**
- **Next required fix:** (one line, or "No fix required" if PASS)

## Rules

- Scan only tracked source and config (no node_modules, Bin, obj).
- If you create a script to satisfy build/test, keep it minimal and runnable (no placeholders).
- Canonical build: prefer `scripts/build.ps1` or root `build.ps1`.
