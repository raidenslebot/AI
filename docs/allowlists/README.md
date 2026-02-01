# Raijin allowlists

Allowlists let the harness treat certain findings as **WARN_ALLOWLISTED** instead of **FAIL_BLOCKER** so bootstrapping can continue while stubs are converted to real implementations.

## banned_patterns_allowlist.yml

Format per entry:

- **file:** Path relative to repo root (e.g. `Core/HAL/hal_13700k.cpp`).
- **allowed_phrases:** List of exact phrases or regex-like patterns allowed in that file (e.g. `"Simulated value"`, `"for now"`).
- **reason:** Why this is allowed (e.g. "HAL development stub until real MSR read").
- **remove_by:** Date (YYYY-MM-DD) or milestone (e.g. "v1.0", "post-bootstrap").
- **owner_role:** Agent role responsible for removing (e.g. `implementer`).
- **notes:** Optional extra context.

When /raijin_guard finds a banned pattern, it checks this file. If the match is allowlisted (file + phrase match), the finding is **WARN_ALLOWLISTED**. If not, it is **FAIL_BLOCKER**. Verdict **PASS** only if there are zero FAIL_BLOCKER.

Promotion (/raijin_promote) requires Guard PASS **or** every banned-pattern finding allowlisted. Use the allowlist for bootstrapping; remove entries as stubs are replaced.
