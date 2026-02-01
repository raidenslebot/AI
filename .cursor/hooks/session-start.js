#!/usr/bin/env node
"use strict";
const { logsDir, appendLog, setSessionState } = require("./lib.js");
let input = "";
process.stdin.setEncoding("utf8");
process.stdin.on("data", (chunk) => { input += chunk; });
process.stdin.on("end", () => {
  try {
    logsDir();
    const data = JSON.parse(input);
    const cid = data.conversation_id || data.session_id;
    setSessionState(cid, { core_edited: false, build_failed: false, started_at: new Date().toISOString() });
    appendLog("hooks", { event: "sessionStart", cid });
    const ctx = [
      "RAIJIN: Evolution organism. Neural, HAL, Programming Domination. No limits. Supremacy through coherence and self-evolution.",
      "CURSOR AGENT: orchestration, scaffolding, evaluation, ops. RAIJIN: cognition, learning, adaptation. Role separation invariant.",
      "Stopping at it works is FAILURE. Dominant under sustained adversarial pressure. Ambition without apology. Evolution without endpoint.",
      "Build: build_raijin_mingw.bat. Test: Bin\\raijin.exe --self-test. Bootstrap: bootstrap_raijin.bat. Verify every change.",
      "Placeholder/demo/stub/simulated/mock detection enabled. Edits introducing these trigger AUTO-REVIEW. Scan: node .cursor/hooks/placeholder-scan.js <path>.",
    ].join(" ");
    process.stdout.write(JSON.stringify({ continue: true, additional_context: ctx }));
  } catch (e) {
    process.stdout.write(JSON.stringify({ continue: true }));
  }
});
