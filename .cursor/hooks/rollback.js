"use strict";
const fs = require("fs");
const path = require("path");
const crypto = require("crypto");

function projectDir() {
  return process.env.CURSOR_PROJECT_DIR || process.env.CLAUDE_PROJECT_DIR || process.cwd();
}

function rollbackDir() {
  const pd = projectDir();
  const dir = path.join(pd, "data", "rollback");
  if (!fs.existsSync(path.dirname(dir))) fs.mkdirSync(path.dirname(dir), { recursive: true });
  if (!fs.existsSync(dir)) fs.mkdirSync(dir, { recursive: true });
  return dir;
}

function normalizePath(filePath) {
  const pd = projectDir();
  const abs = path.isAbsolute(filePath) ? filePath : path.join(pd, filePath);
  return path.relative(pd, abs).replace(/\\/g, "/");
}

function contentHash(content) {
  return crypto.createHash("sha256").update(content || "").digest("hex").slice(0, 16);
}

function snapshotPath(relPath, stage, ts) {
  const safe = relPath.replace(/[^a-zA-Z0-9_\-\.\/]/g, "_");
  const dir = path.join(rollbackDir(), safe);
  if (!fs.existsSync(dir)) fs.mkdirSync(dir, { recursive: true });
  return path.join(dir, `${ts}_s${stage}.snap`);
}

function indexPath() {
  return path.join(rollbackDir(), "_index.jsonl");
}

function saveSnapshot(filePath, content, stage, metadata = {}) {
  try {
    const rel = normalizePath(filePath);
    const ts = Date.now();
    const snapPath = snapshotPath(rel, stage, ts);
    const indexPath_ = indexPath();
    const entry = {
      ts,
      rel,
      stage,
      snap: path.relative(rollbackDir(), snapPath),
      ...metadata,
    };
    fs.writeFileSync(snapPath, content, "utf8");
    fs.appendFileSync(indexPath_, JSON.stringify(entry) + "\n", "utf8");
    return { ts, rel, stage };
  } catch (e) {
    return null;
  }
}

function listSnapshotsForFile(filePath) {
  try {
    const rel = normalizePath(filePath);
    const indexPath_ = indexPath();
    if (!fs.existsSync(indexPath_)) return [];
    const lines = fs.readFileSync(indexPath_, "utf8").trim().split("\n").filter(Boolean);
    const entries = lines.map((l) => {
      try {
        return JSON.parse(l);
      } catch (_) {
        return null;
      }
    }).filter(Boolean).filter((e) => e.rel === rel);
    return entries.sort((a, b) => b.ts - a.ts);
  } catch (_) {
    return [];
  }
}

function findLatestSnapshot(filePath) {
  const entries = listSnapshotsForFile(filePath);
  if (entries.length === 0) return null;
  const e = entries[0];
  const snapFullPath = path.join(rollbackDir(), e.snap);
  if (!fs.existsSync(snapFullPath)) return null;
  const content = fs.readFileSync(snapFullPath, "utf8");
  return { content, ts: e.ts, stage: e.stage };
}

function restoreFile(filePath) {
  const latest = findLatestSnapshot(filePath);
  if (!latest) return false;
  try {
    const pd = projectDir();
    const abs = path.isAbsolute(filePath) ? filePath : path.join(pd, filePath);
    const dir = path.dirname(abs);
    if (!fs.existsSync(dir)) fs.mkdirSync(dir, { recursive: true });
    fs.writeFileSync(abs, latest.content, "utf8");
    return true;
  } catch (_) {
    return false;
  }
}

function registerExpectedPath(filePath) {
  const rel = normalizePath(filePath);
  const manifestPath = path.join(rollbackDir(), "_expected.jsonl");
  try {
    fs.appendFileSync(manifestPath, JSON.stringify({ rel, ts: Date.now() }) + "\n", "utf8");
  } catch (_) {}
}

function getExpectedPaths() {
  try {
    const manifestPath = path.join(rollbackDir(), "_expected.jsonl");
    if (!fs.existsSync(manifestPath)) return new Set();
    const lines = fs.readFileSync(manifestPath, "utf8").trim().split("\n").filter(Boolean);
    const rels = new Set();
    for (const l of lines) {
      try {
        const o = JSON.parse(l);
        if (o.rel) rels.add(o.rel);
      } catch (_) {}
    }
    return rels;
  } catch (_) {
    return new Set();
  }
}

module.exports = {
  rollbackDir,
  normalizePath,
  saveSnapshot,
  listSnapshotsForFile,
  findLatestSnapshot,
  restoreFile,
  registerExpectedPath,
  getExpectedPaths,
};
