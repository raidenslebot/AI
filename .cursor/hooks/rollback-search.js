#!/usr/bin/env node
"use strict";
const { listSnapshotsForFile, findLatestSnapshot, getExpectedPaths } = require("./rollback.js");
const path = require("path");
const args = process.argv.slice(2);
const cmd = args[0] || "list";
if (cmd === "list") {
  const fileArg = args[1];
  if (fileArg) {
    const entries = listSnapshotsForFile(fileArg);
    if (entries.length === 0) console.log(`No snapshots for ${fileArg}`);
    else entries.forEach((e, i) => console.log(`${i + 1}. ts=${e.ts} stage=${e.stage} source=${e.source || "-"}`));
  } else {
    const expected = getExpectedPaths();
    console.log("Expected paths (known to rollback):", [...expected].slice(0, 50).join("\n"));
    if (expected.size > 50) console.log("... and", expected.size - 50, "more");
  }
} else if (cmd === "latest") {
  const fileArg = args[1];
  if (!fileArg) {
    console.log("Usage: node rollback-search.js latest <file_path>");
    process.exit(1);
  }
  const latest = findLatestSnapshot(fileArg);
  if (!latest) console.log(`No snapshot for ${fileArg}`);
  else console.log(JSON.stringify({ ts: latest.ts, stage: latest.stage, contentLength: latest.content.length }));
}
