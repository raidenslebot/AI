#!/usr/bin/env node
"use strict";
const path = require("path");
const { projectDir, appendLog, setSessionState } = require("./lib.js");
const { findLatestSnapshot, restoreFile } = require("./rollback.js");
let input = "";
process.stdin.setEncoding("utf8");
process.stdin.on("data", (chunk) => { input += chunk; });
process.stdin.on("end", () => {
  try {
    const data = JSON.parse(input);
    const tool = data.tool_name || "";
    appendLog("hooks", {
      event: "postToolUseFailure",
      tool,
      failure_type: data.failure_type,
      error: (data.error_message || "").slice(0, 300),
    });
    if (tool === "Read") {
      let rawPath = "";
      const ti = data.tool_input;
      if (typeof ti === "object" && ti !== null) {
        rawPath = ti.path || ti.file_path || ti.filePath || ti.target_file || "";
      } else if (typeof ti === "string") {
        try {
          const p = JSON.parse(ti);
          rawPath = p.path || p.file_path || p.filePath || "";
        } catch (_) {
          rawPath = ti;
        }
      }
      if (rawPath) {
        const pd = projectDir();
        const abs = path.isAbsolute(rawPath) ? rawPath : path.join(pd, rawPath);
        const rel = path.relative(pd, abs).replace(/\\/g, "/");
        const fs = require("fs");
        if (!fs.existsSync(abs)) {
          const latest = findLatestSnapshot(rel);
          if (latest) {
            const restored = restoreFile(rel);
            if (restored) {
              setSessionState(data.conversation_id, { missing_file_restored: rel });
              appendLog("hooks", { event: "postToolUseFailure_restored", file: rel });
            } else {
              setSessionState(data.conversation_id, { missing_file_suggest_restore: rel });
            }
          }
        }
      }
    }
    process.stdout.write(JSON.stringify({}));
  } catch (e) {
    process.stdout.write(JSON.stringify({}));
  }
});
