# Raijin guard (Gauntlet)

Full-spectrum weakness scanner. Output is **PASS** or **FAIL** with the next required fix. Every match in any category is **FAIL_BLOCKER**. PASS only when zero matches across all categories. Purist: no allowlist.

## Objective

1. **Git status:** Run `git status` (and optionally `git diff --stat`) in the project root. Report: branch, clean/dirty, ahead/behind origin, list of modified/untracked/deleted files. Flag if uncommitted changes look like unfinished work (e.g. many modified Core/ files with no commit).

2. **Gauntlet scan:** Run all scan categories below over **tracked source only**. Exclude: node_modules, Bin, obj, .worktrees. For each category, report every match as **FAIL_BLOCKER** with format: `Category | file:line | short snippet`. If a category has no matches, report "Category X: none."

3. **Canonical build/test:** Identify the **canonical build** command and **canonical test** (or minimal verification) command. Check for: `scripts/build.ps1`, `build_raijin.bat`, `dotnet build`, `npm run build`, or a single documented build; and for `Bin\raijin.exe --self-test`, `test_gauntlet.bat`, `pytest`, `npm test`, or a single documented test. If **either** is missing or not clearly defined, **declare missing** and state what must be added.

4. **Verdict and next fix:**
   - **PASS** only if: **zero** matches in every category, git is clean or only expected changes, and both build and test commands are identified (or created). Output **PASS** and "No fix required."
   - **FAIL** if any match exists in any category, or build/test missing, or git state requires action. Output **FAIL** and exactly **one** "Next required fix:" line. **Next-fix selection rule:** Choose the **highest-impact cluster** (by match count) within **Core/** first; if none, then **harness integrity** (categories F); then **provenance** (category G). Example: "Remove weakness cluster in Core/HAL/hal_13700k.cpp (N matches); implement real logic and tests."

## Scan scope

- **Paths:** Core/, Include/, RaijinUI/, scripts/, .cursor/commands/, .cursor/rules/, .cursor/hooks/
- **File types:** .c, .cpp, .h, .hpp, .cs, .xaml, .py, .js, .ts, .ps1, .bat, .md, .json, .yml, .yaml
- **Exclude:** node_modules, Bin, obj, .worktrees (and their contents)
- **Tracked only:** Scan only files that are tracked by git (or explicitly listed in scope). Do not report on untracked files except as part of git status.

## Scan categories (each match = FAIL_BLOCKER)

### A. Banned-language patterns

Search (case-insensitive where sensible) for:

- **Tags:** TODO, FIXME, HACK, XXX
- **Placeholder/stub/mock:** placeholder, stub, mock, simulat(e|ed|ion), demo(nstration)?, example (when used as placeholder context)
- **Deferred work:** "for now", "temporary", "not implemented", "not yet", "to be implemented", "later", "eventually", "NOTE" (when implying future work)

Report: `A | path:line | snippet`.

### B. Empty-body heuristics

- **C/C++/C#:** Function bodies that contain only: `return 0;`, `return false;`, `return true;`, `{}`, or a single `;`. Flag only when the function name does not suggest a trivial getter/setter (e.g. GetX, IsY, HasZ). Be conservative but aggressive: skip obvious accessors; flag non-trivial-looking functions with empty or trivial bodies.
- **Python:** `pass` as the only statement in a non-trivial def or class method (e.g. not `def __init__(self): pass` when it is intentionally empty; flag when the method name suggests real work).

Report: `B | path:line | function/method name or "empty body"`.

### C. Silent error absorption

- **C/C++/C#:** catch blocks that are empty or contain only comments. `catch (...) {}`, `catch (Exception) { }`, etc.
- **Python:** `except:` or `except Exception:` followed only by `pass` (or only comments).
- **Heuristic:** `if (!ok) return;` (or similar) with no logging or error propagation in the same block. Flag when the pattern suggests swallowed errors.

Report: `C | path:line | snippet or "empty catch" / "except: pass"`.

### D. Debug residue

- **Core/ and Include/ only:** Raw `printf`, `cout`, `cerr`, `console.log`, `debugger` statements. Exclude if the output is clearly routed through a telemetry or logging abstraction (e.g. a function named Log*, Telemetry*, WriteLog* that is defined in the repo and used consistently).

Report: `D | path:line | snippet`.

### E. Nondeterminism

- **Core/ only:** Uses of `rand()`, `random()` (stdlib without seed), `time(NULL)`, `std::chrono::system_clock`, or similar wall-clock/time-based logic that is not clearly sourced from a controlled Time or RNG abstraction (e.g. injectable clock, seeded RNG). Flag occurrences; per-case decisions may follow.

Report: `E | path:line | snippet`.

### F. Harness integrity

- **hooks.json:** For each entry in `.cursor/hooks.json` that references a script (e.g. `node .cursor/hooks/foo.js`), verify that the file `.cursor/hooks/foo.js` exists. If missing: FAIL_BLOCKER.
- **mcp.json:** For each MCP server entry in `.cursor/mcp.json`, if the server uses a local path (e.g. `node path/to/index.js`), verify that path exists under the repo. If missing: FAIL_BLOCKER.
- **Rules frontmatter:** Each file under `.cursor/rules/*.mdc` that is intended to be conditional should have `alwaysApply` or `globs` (or equivalent) in frontmatter where expected. If a rule file has no frontmatter and is not the single global rule, flag it. (Conservative: only flag clearly missing frontmatter on language/scope rules.)

Report: `F | path:line or "hooks"|"mcp"|"rules" | snippet or "missing script" / "missing path"`.

### G. Provenance integrity

- If there are **tracked code changes** (files under Core/, Include/, RaijinUI/, scripts/) in the **last commit** (HEAD vs HEAD~1), then there must be at least one **new or updated** file under `docs/provenance/` in that same commit (or the commit message must reference a run_id and evidence path). Otherwise: one FAIL_BLOCKER for provenance gap.
- "Code changes" means modified/added .c, .cpp, .h, .hpp, .cs, .xaml, .py, .js, .ts, .ps1, .bat under those paths. Config-only or doc-only commits may be exempt (Guard may treat as no code change).

Report: `G | docs/provenance/ or "last commit" | "Code change in HEAD without provenance update"`.

## Output format

1. **Git status summary** (branch, clean/dirty, ahead/behind, modified/untracked/deleted).
2. **Scan results by category:**
   - **A. Banned-language:** list each match as `A | file:line | snippet` (or "none").
   - **B. Empty-body:** list each match as `B | file:line | snippet` (or "none").
   - **C. Silent error absorption:** list each match as `C | file:line | snippet` (or "none").
   - **D. Debug residue:** list each match as `D | file:line | snippet` (or "none").
   - **E. Nondeterminism:** list each match as `E | file:line | snippet` (or "none").
   - **F. Harness integrity:** list each match as `F | target | snippet` (or "none").
   - **G. Provenance integrity:** list each match as `G | target | snippet` (or "none").
3. **Build path:** command and path (or "missing").
4. **Test path:** command and path (or "missing").
5. **PASS** or **FAIL**.
6. **Next required fix:** exactly one line (or "No fix required" if PASS).

## Rules

- Scan only tracked source in the listed paths and file types. Exclude node_modules, Bin, obj, .worktrees.
- No allowlist. Every match in every category is FAIL_BLOCKER.
- If you create a script to satisfy build/test, keep it minimal and runnable (no placeholders).
- Canonical build: prefer `scripts/build.ps1` or root `build.ps1`.
- See docs/decisions/guard_gauntlet.md for category semantics and Purist policy.
