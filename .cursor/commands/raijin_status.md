# Raijin status

Print the following in chat. Use read-only tools and shell only as needed. Output a single structured report.

## 1) Git

- Branch name and clean/dirty (e.g. `main` clean or `main` dirty).
- Last 5 commits (oneline: hash + subject).
- Remote: name and URL (e.g. `origin` https://github.com/raidenslebot/AI.git).

Use: `git status`, `git branch -v`, `git log -5 --oneline`, `git remote -v` in the project root.

## 2) Rules

List each file under `.cursor/rules/*.mdc` with:
- File name
- `alwaysApply`: true/false (from frontmatter)
- `globs`: value if present, else "—"

Read each `.mdc` and extract the YAML frontmatter.

## 3) Hooks

List hooks from `.cursor/hooks.json`: hook name and the command (e.g. `node .cursor/hooks/…`). Show hook key and first command only.

## 4) MCP health summary

- If you can read MCP Output logs (e.g. from a file or MCP tool), summarize: for each enabled server in `.cursor/mcp.json`, report started/failed, tool count, and last error line if any.
- If you cannot read the MCP Output channel (View → Output → MCP): state "MCP status: read from View → Output → MCP. Enabled servers: [list server names from mcp.json]." Then list each server from `.cursor/mcp.json` and note that status/tools/errors require the user to check the Output panel.

End with a short summary: git branch + clean/dirty, rule count, hook count, and MCP note (summary or "check Output").
