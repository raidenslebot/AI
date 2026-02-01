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
      event: "preCompact",
      trigger: data.trigger,
      context_usage_percent: data.context_usage_percent,
      context_tokens: data.context_tokens,
      context_window_size: data.context_window_size,
      message_count: data.message_count,
      messages_to_compact: data.messages_to_compact,
      is_first: data.is_first_compaction,
    });
    process.stdout.write(JSON.stringify({}));
  } catch (e) {
    process.stdout.write(JSON.stringify({}));
  }
});
