#!/usr/bin/env node
"use strict";
const fs = require("fs");
const path = require("path");
const { projectDir } = require("./lib.js");
const { scanPlaceholderLike } = require("./review.js");
const args = process.argv.slice(2);
const fileArg = args[0];
if (!fileArg) {
  console.log("Usage: node placeholder-scan.js <file_path>");
  process.exit(0);
}
const pd = projectDir();
const abs = path.isAbsolute(fileArg) ? fileArg : path.join(pd, fileArg);
if (!fs.existsSync(abs)) {
  console.log("File not found:", abs);
  process.exit(1);
}
const content = fs.readFileSync(abs, "utf8");
const matches = scanPlaceholderLike(content);
if (matches.length === 0) {
  console.log("No placeholder/demo/simulated/mock patterns detected.");
  process.exit(0);
}
const byTag = {};
for (const m of matches) {
  byTag[m.tag] = (byTag[m.tag] || 0) + 1;
}
console.log("PLACEHOLDER/DEMO/SIMULATED DETECTION:");
console.log(JSON.stringify(byTag, null, 2));
console.log("");
for (const m of matches.slice(0, 20)) {
  console.log(`  L${m.line}: [${m.tag}] ${m.text}`);
}
if (matches.length > 20) console.log(`  ... and ${matches.length - 20} more`);
process.exit(2);
