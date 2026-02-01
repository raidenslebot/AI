#!/usr/bin/env node
"use strict";
const path = require("path");
const fs = require("fs");
const { projectDir, appendLog, setSessionState } = require("./lib.js");
const { saveSnapshot, registerExpectedPath } = require("./rollback.js");
const { runReview } = require("./review.js");
let input = "";
process.stdin.setEncoding("utf8");
process.stdin.on("data", (chunk) => { input += chunk; });
process.stdin.on("end", () => {
  try {
    const data = JSON.parse(input);
    const filePath = data.file_path || "";
    const edits = data.edits || [];
    const pd = projectDir();
    const abs = path.isAbsolute(filePath) ? filePath : path.join(pd, filePath);
    const rel = path.relative(pd, abs).replace(/\\/g, "/");
    if (edits.length === 0) {
      process.stdout.write(JSON.stringify({}));
      return;
    }
    registerExpectedPath(rel);
    const isCore = rel.includes("Core") || rel.includes("Include");
    if (isCore) setSessionState(data.conversation_id, { core_edited: true });
    let postContent = "";
    try {
      postContent = fs.readFileSync(abs, "utf8");
    } catch (_) {}
    let preContent = postContent;
    for (let i = edits.length - 1; i >= 0; i--) {
      const e = edits[i];
      preContent = preContent.split(e.new_string).join(e.old_string);
    }
    const entries = require("./rollback.js").listSnapshotsForFile(rel);
    const nextStage = entries.length > 0 ? Math.max(...entries.map((x) => x.stage || 0)) + 1 : 1;
    saveSnapshot(rel, preContent, nextStage - 1, { source: "afterFileEdit_pre" });
    saveSnapshot(rel, postContent, nextStage, { source: "afterFileEdit_post" });
    const review = runReview(preContent, postContent, filePath);
    if (review.degraded) {
      setSessionState(data.conversation_id, {
        edit_degradation: true,
        degradation_file: rel,
        degradation_issues: review.issues,
        placeholder_matches: review.placeholderMatches || [],
      });
      appendLog("hooks", {
        event: "afterFileEdit_degradation",
        file: rel,
        issues: review.issues,
        placeholder_count: (review.placeholderMatches || []).length,
        metrics: review.metrics,
      });
    }
    appendLog("hooks", {
      event: "afterFileEdit",
      file: rel,
      edit_count: edits.length,
      core: isCore,
      degraded: review.degraded,
    });
    process.stdout.write(JSON.stringify({}));
  } catch (e) {
    process.stdout.write(JSON.stringify({}));
  }
});
