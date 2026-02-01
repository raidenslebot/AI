# Guard Gauntlet: full-spectrum weakness scanner

**Status:** Accepted  
**Date:** 2026-01-31

## Purpose

Under Purist policy, /raijin_guard is the **weakness detector**, not a style cop. It flags anything that represents incompleteness, fake implementation, uncertainty, drift, or unverifiable behavior. Every match in any category is **FAIL_BLOCKER**. PASS only when zero matches across all categories.

## Scan scope

- **Paths:** Core/, Include/, RaijinUI/, scripts/, .cursor/commands/, .cursor/rules/, .cursor/hooks/
- **File types:** .c, .cpp, .h, .hpp, .cs, .xaml, .py, .js, .ts, .ps1, .bat, .md, .json, .yml, .yaml
- **Exclude:** node_modules, Bin, obj, .worktrees
- **Tracked only:** Scan only files tracked by git in the above paths.

## Categories and Purist semantics

### A. Banned-language patterns

Incomplete or deferred work signals: TODO, FIXME, HACK, XXX; placeholder, stub, mock, simulated, demo, example (as placeholder); "for now", "temporary", "not implemented", "not yet", "to be implemented", "later", "eventually"; NOTE implying future work. **Any match = FAIL_BLOCKER.**

### B. Empty-body heuristics

Functions or methods that are non-trivial by name but have only a trivial return or empty body (e.g. `return 0;`, `return true;`, `{}`, or Python `pass` in a method that suggests real work). Conservative: skip obvious getters/setters; flag the rest. **Any match = FAIL_BLOCKER.**

### C. Silent error absorption

Empty or comment-only catch/except blocks; `except: pass`; patterns like `if (!ok) return;` with no logging or propagation. **Any match = FAIL_BLOCKER.**

### D. Debug residue

Raw printf, cout, console.log, debugger in Core/ or Include/ unless routed through a telemetry/log abstraction. **Any match = FAIL_BLOCKER.**

### E. Nondeterminism

rand(), unseeded RNG, time(), system_clock in Core/ without a controlled Time or RNG abstraction. Flag occurrences. **Any match = FAIL_BLOCKER.**

### F. Harness integrity

hooks.json references a script that does not exist; mcp.json references a path that does not exist; .cursor/rules frontmatter missing (alwaysApply/globs) where expected. **Any match = FAIL_BLOCKER.**

### G. Provenance integrity

Tracked code change in the last commit (Core/, Include/, RaijinUI/, scripts/) without a corresponding new or updated file under docs/provenance/ (or commit message referencing run_id/evidence). **Any match = FAIL_BLOCKER.**

## Output and next-fix rule

- **Output:** git summary → scan results grouped by category (each match: Category | file:line | snippet) → build path → test path → PASS/FAIL → Next required fix.
- **Next required fix:** Exactly one line. Selection order: (1) highest-impact cluster by match count within **Core/**; (2) harness integrity (F); (3) provenance (G).

## Relation to other decisions

- **Bootstrapping (Purist):** docs/decisions/bootstrapping_policy.md — Guard PASS only when zero banned-pattern matches; no allowlist. The Gauntlet extends "banned patterns" to the full set of categories above.
- **Promotion:** /raijin_promote requires Guard PASS (Gate 0). No promotion when Guard FAILs.
- **Pipeline:** docs/decisions/agent_pipeline.md — Guard runs on main; evidence in docs/run_evidence/latest_guard.md.
