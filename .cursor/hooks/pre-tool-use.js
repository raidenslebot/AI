#!/usr/bin/env node
"use strict";
const { appendLog } = require("./lib.js");
let input = "";
process.stdin.setEncoding("utf8");
process.stdin.on("data", (chunk) => { input += chunk; });
process.stdin.on("end", () => {
  try {
    const data = JSON.parse(input);
    appendLog("hooks", { event: "preToolUse", tool: data.tool_name });
    process.stdout.write(JSON.stringify({ decision: "allow" }));
  } catch (e) {
    process.stdout.write(JSON.stringify({ decision: "allow" }));
  }
});
