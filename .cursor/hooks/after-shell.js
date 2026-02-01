#!/usr/bin/env node
"use strict";
const { appendLog, setSessionState } = require("./lib.js");
let input = "";
process.stdin.setEncoding("utf8");
process.stdin.on("data", (chunk) => { input += chunk; });
process.stdin.on("end", () => {
  try {
    const data = JSON.parse(input);
    const cmd = (data.command || "").trim();
    const output = (data.output || "").toLowerCase();
    const isBuild = /build_raijin|g\+\+|gcc|msbuild|dotnet\s+build/i.test(cmd);
    const isTest = /raijin\.exe\s+--self-test|--regression-replay|test_gauntlet|pytest|npm\s+test/i.test(cmd);
    const damageIndicators = /error:|failed|fatal|build failed|test failed|exit code [1-9]/;
    const damaged = damageIndicators.test(output) && (isBuild || isTest);
    if (damaged) setSessionState(data.conversation_id, { build_failed: true });
    appendLog("hooks", {
      event: "afterShellExecution",
      kind: isBuild ? "build" : isTest ? "test" : "shell",
      duration: data.duration || 0,
      damaged,
      cmd_preview: cmd.slice(0, 300),
    });
    process.stdout.write(JSON.stringify({}));
  } catch (e) {
    process.stdout.write(JSON.stringify({}));
  }
});
