# Raijin worktree

Create or reuse a git worktree for isolated work. Usage: **/raijin_worktree &lt;objective_slug&gt;**

Never edit main directly for objective work; use a worktree so main stays clean until promotion.

## Objective

1. **Resolve slug:** The argument is &lt;objective_slug&gt; (e.g. `bootstrapping_policy`, `hal_real_impl`). Normalize to a valid path segment: lowercase, underscores for spaces, no slashes.
2. **Worktree path:** `.worktrees/<objective_slug>` relative to repo root (e.g. `c:\Raijin\.worktrees\bootstrapping_policy`). Branch name: `raijin/<objective_slug>` (e.g. `raijin/bootstrapping_policy`).
3. **Create or reuse:**
   - If `.worktrees/<objective_slug>` already exists and is a valid git worktree: report "Worktree exists." and skip creation.
   - Else: run `git worktree add .worktrees/<objective_slug> -b raijin/<objective_slug>` from repo root (creates branch and worktree). If the branch already exists, use `git worktree add .worktrees/<objective_slug> raijin/<objective_slug>`.
4. **Minimal bootstrap:** In the worktree directory, run `npm install` only if `package.json` exists and `node_modules` is missing or outdated (e.g. no node_modules). Otherwise skip. Do not run full build unless the user asks.
5. **Output:** Print:
   - **Worktree path:** (absolute path to .worktrees/&lt;objective_slug&gt;)
   - **Branch:** raijin/&lt;objective_slug&gt;
   - **Commands to enter worktree:** `cd <absolute_path>` (or `Set-Location <path>` on PowerShell)
   - **To run Implementer in this worktree:** Open Cursor in that path, or run `/raijin_spawn implementer "<objective>"` from that path so edits land in the worktree.

## Rules

- Do not run build/test in this command unless the user requests it (bootstrap = worktree + optional npm install only).
- Ensure `.worktrees/` is in `.gitignore` so worktree contents are not tracked by main.
- If `git worktree add` fails (e.g. path in use, branch exists), report the error and suggest: remove existing `.worktrees/<slug>` or use a different slug.
