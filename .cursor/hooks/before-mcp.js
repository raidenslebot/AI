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
      event: "beforeMCPExecution",
      tool_name: data.tool_name,
      input_preview: (typeof data.tool_input === "string" ? data.tool_input : JSON.stringify(data.tool_input || {})).slice(0, 500),
    });
    process.stdout.write(JSON.stringify({ permission: "allow" }));
  } catch (e) {
    process.stdout.write(JSON.stringify({ permission: "allow" }));
  }
});
