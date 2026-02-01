#!/usr/bin/env node
"use strict";
const { appendLog } = require("./lib.js");
let input = "";
process.stdin.setEncoding("utf8");
process.stdin.on("data", (chunk) => { input += chunk; });
process.stdin.on("end", () => {
  try {
    const data = JSON.parse(input);
    const text = data.text || "";
    appendLog("hooks", { event: "afterAgentResponse", len: text.length, preview: text.slice(0, 100) });
    process.stdout.write(JSON.stringify({}));
  } catch (e) {
    process.stdout.write(JSON.stringify({}));
  }
});
