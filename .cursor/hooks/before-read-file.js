#!/usr/bin/env node
"use strict";
const path = require("path");
const fs = require("fs");
const { projectDir, appendLog } = require("./lib.js");
const { saveSnapshot, findLatestSnapshot, restoreFile, registerExpectedPath } = require("./rollback.js");
let input = "";
process.stdin.setEncoding("utf8");
process.stdin.on("data", (chunk) => { input += chunk; });
process.stdin.on("end", () => {
  try {
    const data = JSON.parse(input);
    const filePath = data.file_path || "";
    const content = data.content || "";
    const pd = projectDir();
    const abs = path.isAbsolute(filePath) ? filePath : path.join(pd, filePath);
    const rel = path.relative(pd, abs).replace(/\\/g, "/");
    const exists = fs.existsSync(abs);
    if (exists && content && content.length > 0) {
      registerExpectedPath(rel);
      const stage = 0;
      saveSnapshot(rel, content, stage, { source: "beforeReadFile" });
      appendLog("hooks", { event: "beforeReadFile_capture", file: rel });
    }
    if (!exists) {
      const latest = findLatestSnapshot(rel);
      if (latest) {
        const restored = restoreFile(rel);
        if (restored) {
          appendLog("hooks", { event: "beforeReadFile_restored", file: rel });
        }
      }
    }
    process.stdout.write(JSON.stringify({ permission: "allow" }));
  } catch (e) {
    process.stdout.write(JSON.stringify({ permission: "allow" }));
  }
});
