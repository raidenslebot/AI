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
      event: "sessionEnd",
      session_id: data.session_id,
      reason: data.reason,
      duration_ms: data.duration_ms,
      is_bg: data.is_background_agent,
      final_status: data.final_status,
      error: (data.error_message || "").slice(0, 200),
    });
    process.stdout.write(JSON.stringify({}));
  } catch (e) {
    process.stdout.write(JSON.stringify({}));
  }
});
