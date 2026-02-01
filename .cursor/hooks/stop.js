#!/usr/bin/env node
"use strict";
const { appendLog, getSessionState, setSessionState, projectDir } = require("./lib.js");
const fs = require("fs");
const path = require("path");
let input = "";
process.stdin.setEncoding("utf8");
process.stdin.on("data", (chunk) => { input += chunk; });
process.stdin.on("end", () => {
  try {
    const data = JSON.parse(input);
    const status = data.status || "";
    const loopCount = data.loop_count || 0;
    const cid = data.conversation_id;
    const state = getSessionState(cid);
    appendLog("hooks", {
      event: "stop",
      status,
      loop_count: loopCount,
      core_edited: state.core_edited,
      build_failed: state.build_failed,
      edit_degradation: state.edit_degradation,
    });
    let followup = "";
    const buildPath = path.join(projectDir(), "build_raijin_mingw.bat");
    const hasGit = fs.existsSync(path.join(projectDir(), ".git"));
    if (status === "completed" && loopCount < 3) {
      if (state.edit_degradation && state.degradation_file) {
        const issues = state.degradation_issues || [];
        const hasPlaceholder = issues.some((i) => /placeholder|demo|simulated|mock|stub|unimplemented/i.test(i));
        const scanCmd = hasPlaceholder ? ` Scan: node .cursor/hooks/placeholder-scan.js ${state.degradation_file}.` : "";
        followup = `AUTO-REVIEW: Edit may have lost functionality, introduced placeholders/demo/simulated code, or made things worse. Issues: ${issues.join("; ")}. Rollback and reassessment recommended. Restore: node .cursor/hooks/rollback-restore.js ${state.degradation_file}.${scanCmd}`;
        setSessionState(cid, { edit_degradation: false, degradation_file: null, degradation_issues: null });
      } else if (state.missing_file_suggest_restore) {
        followup = `File ${state.missing_file_suggest_restore} was missing. Restore from rollback: node .cursor/hooks/rollback-restore.js ${state.missing_file_suggest_restore}`;
        setSessionState(cid, { missing_file_suggest_restore: null });
      } else if (state.missing_file_restored) {
        followup = `File ${state.missing_file_restored} was restored from rollback. Verify and continue.`;
        setSessionState(cid, { missing_file_restored: null });
      } else if (state.build_failed && state.core_edited && hasGit) {
        followup = "Damage detected (build/test failed). Rollback: git checkout -- Core/ Include/. Or restore specific file: node .cursor/hooks/rollback-restore.js <path>. Then fix and run build_raijin_mingw.bat.";
        setSessionState(cid, { core_edited: false, build_failed: false });
      } else if (state.core_edited && fs.existsSync(buildPath)) {
        followup = "Execute build_raijin_mingw.bat. Then Bin\\raijin.exe --self-test. Report results. Continue evolution.";
        setSessionState(cid, { core_edited: false });
      }
    }
    process.stdout.write(JSON.stringify(followup ? { followup_message: followup } : {}));
  } catch (e) {
    process.stdout.write(JSON.stringify({}));
  }
});
