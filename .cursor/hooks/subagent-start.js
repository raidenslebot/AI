#!/usr/bin/env node
"use strict";
const { appendLog } = require("./lib.js");
let input = "";
process.stdin.setEncoding("utf8");
process.stdin.on("data", (chunk) => { input += chunk; });
process.stdin.on("end", () => {
  try {
    const data = JSON.parse(input);
    appendLog("hooks", {
      event: "subagentStart",
      subagent_type: data.subagent_type,
      prompt_preview: (data.prompt || "").slice(0, 200),
    });
    process.stdout.write(JSON.stringify({ decision: "allow" }));
  } catch (e) {
    process.stdout.write(JSON.stringify({ decision: "allow" }));
  }
});
