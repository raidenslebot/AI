#!/usr/bin/env node
"use strict";
const { restoreFile, listSnapshotsForFile, projectDir } = require("./rollback.js");
const path = require("path");
const args = process.argv.slice(2);
const fileArg = args[0];
if (!fileArg) {
  console.log("Usage: node rollback-restore.js <file_path>");
  process.exit(1);
}
const rel = path.relative(projectDir(), path.isAbsolute(fileArg) ? fileArg : path.join(projectDir(), fileArg)).replace(/\\/g, "/");
const entries = listSnapshotsForFile(rel);
if (entries.length === 0) {
  console.log(`No rollback snapshots for ${rel}`);
  process.exit(1);
}
const ok = restoreFile(rel);
if (ok) {
  console.log(`Restored ${rel} from snapshot (ts=${entries[0].ts})`);
} else {
  console.log(`Restore failed for ${rel}`);
  process.exit(1);
}
