# Bootstrapping policy: Purist

**Status:** Accepted  
**Date:** 2026-01-31

## Decision

Raijin uses **Purist** Guard policy. No allowlist-based bootstrapping. Promotion is blocked unless Guard PASS.

- **Purist:** Guard PASS only when **zero** banned-pattern matches exist in the repo. Every banned-pattern match is **FAIL_BLOCKER**. No allowlist. No WARN_ALLOWLISTED. No Gate 0 escape hatch.
- Promotion requires Guard PASS. Verifier requires Guard PASS. Pipeline can run worktrees/subagents, but nothing promotes if Guard FAILs.

## Guard behavior (canonical)

1. **Banned-pattern scan:** Full-repo (Core/, Include/, RaijinUI/, tracked source). Patterns: TODO, FIXME, mock, demo, simulated, placeholder, stub, "for now", etc.
2. **Classification:** Every match is **FAIL_BLOCKER**. There is no allowlist. Guard does not consult any allowlist file.
3. **Verdict:** **PASS** only if there are **zero** banned-pattern matches. **FAIL** if any match exists.
4. **Promotion:** /raijin_promote requires Guard PASS (Gate 0). No promotion when Guard FAILs.

## Implementation

- Guard: .cursor/commands/raijin_guard.md — no allowlist logic; every match = FAIL_BLOCKER; PASS only when zero matches.
- Promote: Gate 0 = Guard PASS REQUIRED.
- Verifier: GuardStatus must be PASS to PROMOTE; no AllowlistCoverage.
- docs/allowlists/ is irrelevant (deleted or unused); Guard must not reference it.

## Purge order (after Purist is enforced)

Guard will FAIL while stubs exist. The only valid next move is to purge banned patterns cluster by cluster (e.g. /raijin_bestof N hal_purge "…"). High-leverage order: (1) Core/HAL/hal_13700k.cpp, (2) Core/InternetAcquisition/internet_acquisition.cpp, (3) Core/ProgrammingDomination/programming_domination.cpp, (4) Core/Hypervisor/hypervisor_layer.c, then Autonomous/Evolution/Neural.

## Summary

- **Policy:** Purist. Guard PASS only when zero banned-pattern matches. No allowlist.
- **Promotion:** Blocked unless Guard PASS.
- **Next:** Purge banned patterns file-by-file via best-of worktrees until Guard PASS.
