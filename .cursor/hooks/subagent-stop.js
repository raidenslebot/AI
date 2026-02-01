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
      event: "subagentStop",
      subagent_type: data.subagent_type,
      status: data.status,
      duration: data.duration,
      result_preview: (data.result || "").slice(0, 500),
    });
    process.stdout.write(JSON.stringify({}));
  } catch (e) {
    process.stdout.write(JSON.stringify({}));
  }
});
