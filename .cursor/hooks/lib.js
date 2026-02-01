"use strict";
const fs = require("fs");
const path = require("path");

const MAX_LOG_BYTES = 10 * 1024 * 1024;
const ROTATE_KEEP = 5;

function projectDir() {
  return process.env.CURSOR_PROJECT_DIR || process.env.CLAUDE_PROJECT_DIR || process.cwd();
}

function logsDir() {
  const pd = projectDir();
  const data = path.join(pd, "data");
  const logs = path.join(data, "logs");
  if (!fs.existsSync(data)) fs.mkdirSync(data, { recursive: true });
  if (!fs.existsSync(logs)) fs.mkdirSync(logs, { recursive: true });
  return logs;
}

function appendLog(name, obj) {
  try {
    const dir = logsDir();
    const logPath = path.join(dir, `${name}.jsonl`);
    const line = JSON.stringify({ ts: new Date().toISOString(), ...obj }) + "\n";
    fs.appendFileSync(logPath, line, "utf8");
    rotateIfNeeded(logPath);
  } catch (_) {}
}

function rotateIfNeeded(logPath) {
  try {
    const stat = fs.statSync(logPath);
    if (stat.size <= MAX_LOG_BYTES) return;
    const dir = path.dirname(logPath);
    const base = path.basename(logPath, ".jsonl");
    for (let i = ROTATE_KEEP - 1; i >= 0; i--) {
      const src = i === 0 ? logPath : path.join(dir, `${base}.${i}.jsonl`);
      const dst = path.join(dir, `${base}.${i + 1}.jsonl`);
      if (fs.existsSync(src)) fs.renameSync(src, dst);
    }
    fs.writeFileSync(logPath, "", "utf8");
  } catch (_) {}
}

function sessionStatePath(conversationId) {
  const dir = logsDir();
  const safe = (conversationId || "default").replace(/[^a-zA-Z0-9_-]/g, "_").slice(0, 64);
  return path.join(dir, `session_${safe}.json`);
}

function getSessionState(conversationId) {
  try {
    const p = sessionStatePath(conversationId);
    if (fs.existsSync(p)) return JSON.parse(fs.readFileSync(p, "utf8"));
  } catch (_) {}
  return {};
}

function setSessionState(conversationId, updates) {
  try {
    const p = sessionStatePath(conversationId);
    const curr = getSessionState(conversationId);
    fs.writeFileSync(p, JSON.stringify({ ...curr, ...updates }), "utf8");
  } catch (_) {}
}

module.exports = { projectDir, logsDir, appendLog, rotateIfNeeded, getSessionState, setSessionState };
