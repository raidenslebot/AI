#!/usr/bin/env node
"use strict";
const { appendLog } = require("./lib.js");
const DESTRUCTIVE_PATTERNS = [
  /\brm\s+-rf\s+\/(?!\/)/,
  /\brm\s+-rf\s+\*\s*$/,
  /\bdel\s+\/s\s+\/q\s+[cC]:/,
  /\bformat\s+[cC]:/,
  /\bmkfs\./,
  /\bdd\s+if=.*of=\/dev/,
  /\b:\(\)\s*\{\s*:\s*\|\s*:\s*&\s*\}/,
  /\bReg\.exe\s+add\s+HKLM\\Software\\Microsoft\\Windows\s+CurrentVersion\\Run/,
  /\btaskkill\s+\/f\s+\/im\s+csrss/,
  /\bdel\s+\/s\s+\/q\s+\\\\\?\\/,
];
let input = "";
process.stdin.setEncoding("utf8");
process.stdin.on("data", (chunk) => { input += chunk; });
process.stdin.on("end", () => {
  try {
    const data = JSON.parse(input);
    const cmd = (data.command || "").trim();
    appendLog("hooks", { event: "beforeShellExecution", cmd_preview: cmd.slice(0, 500) });
    for (const re of DESTRUCTIVE_PATTERNS) {
      if (re.test(cmd)) {
        appendLog("hooks", { event: "beforeShellExecution_protect", cmd_preview: cmd.slice(0, 200) });
        process.stdout.write(JSON.stringify({
          permission: "deny",
          user_message: "Blocked: command would destroy project or system.",
          agent_message: "Command blocked to protect the workspace. Use safe alternatives.",
        }));
        process.exit(2);
      }
    }
    process.stdout.write(JSON.stringify({ permission: "allow" }));
  } catch (e) {
    process.stdout.write(JSON.stringify({ permission: "allow" }));
  }
});
