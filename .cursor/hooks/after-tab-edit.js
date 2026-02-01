#!/usr/bin/env node
"use strict";
const path = require("path");
const fs = require("fs");
const { projectDir, appendLog } = require("./lib.js");
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
    if (edits.length === 0) {
      process.stdout.write(JSON.stringify({}));
      return;
    }
    const pd = projectDir();
    const abs = path.isAbsolute(filePath) ? filePath : path.join(pd, filePath);
    const rel = path.relative(pd, abs).replace(/\\/g, "/");
    registerExpectedPath(rel);
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
    saveSnapshot(rel, preContent, nextStage - 1, { source: "afterTabFileEdit_pre" });
    saveSnapshot(rel, postContent, nextStage, { source: "afterTabFileEdit_post" });
    const review = runReview(preContent, postContent, filePath);
    appendLog("hooks", { event: "afterTabFileEdit", file: rel, edit_count: edits.length, degraded: review.degraded });
    process.stdout.write(JSON.stringify({}));
  } catch (e) {
    process.stdout.write(JSON.stringify({}));
  }
});
