"use strict";

const PLACEHOLDER_PATTERNS = [
  { re: /\bTODO\b|\bFIXME\b|\bXXX\b|\bHACK\b|\bPLACEHOLDER\b|\bplaceholder\b/gi, tag: "placeholder_marker" },
  { re: /\bstub\b|\bstub\s*\(\)|\bStub\b/gi, tag: "stub" },
  { re: /\bimplement\s*(?:me|later|this)\b|\bto\s*be\s*implemented\b|\bimplement\s*later\b|\bnot\s*yet\s*implemented\b/gi, tag: "unimplemented" },
  { re: /\b(?:for\s*now|simplified|basic)\s*(?:we'll|we\s*will|let's|we)?\s*(?:just|only)\b/gi, tag: "temporary_workaround" },
  { re: /\bin\s*a\s*real\s*system\b|\bhere'?s\s*(?:a\s*)?simplified\b|\bthis\s*is\s*(?:a\s*)?basic\s*example\b/gi, tag: "unreal_example" },
  { re: /\bdemo\b|\bexample\s*code\b|\bfor\s*demonstration\b|\bdemonstration\s*only\b/gi, tag: "demo" },
  { re: /\bsimulated\b|\bsimulation\b|\bmock\b|\bfake\b|\bdummy\b|\bfaked?\b|\bmocked?\b/gi, tag: "simulated_mock" },
  { re: /\bpseudocode\b|\bpseudo-code\b|\bpseudo\s*code\b/gi, tag: "pseudocode" },
  { re: /\/\/\s*\.\.\.|\/\*\s*\.\.\.\s*\*\/|#\s*\.\.\./, tag: "ellipsis_placeholder" },
  { re: /\bREPLACE_ME\b|\bIMPLEMENT_ME\b|\bFILL_ME\b|\bXXX\s*replace\b/gi, tag: "magic_placeholder" },
  { re: /\bassume\s+\w+\s+exists\b|\bassume\s+[^.]*exists\b/gi, tag: "assume_exists" },
  { re: /\breturn\s*(?:0|null|undefined|true|false)\s*;?\s*\/\/\s*(?:placeholder|stub|todo)/gi, tag: "fake_return" },
  { re: /\{\s*\}\s*;?\s*\/\/|\{\s*\/\/\s*todo|\{\s*\/\/\s*stub|\{\s*\/\/\s*placeholder/gi, tag: "empty_body" },
  { re: /\bconsole\.log\s*\([^)]*\)\s*;?\s*\/\/\s*(?:instead|replacing|instead\s*of)/gi, tag: "console_instead_of_logic" },
  { re: /\bprintf?\s*\([^)]*\)\s*;?\s*\/\/\s*(?:instead|replacing|debug)/gi, tag: "print_instead_of_logic" },
];

function scanPlaceholderLike(content) {
  if (!content || typeof content !== "string") return [];
  const matches = [];
  const lines = content.split(/\r?\n/);
  for (let i = 0; i < lines.length; i++) {
    const line = lines[i];
    for (const { re, tag } of PLACEHOLDER_PATTERNS) {
      const r = new RegExp(re.source, re.flags);
      let m;
      while ((m = r.exec(line)) !== null) {
        matches.push({ line: i + 1, tag, text: m[0].slice(0, 60), col: m.index });
      }
    }
  }
  const emptyFn = /\{\s*\}/g;
  let em;
  while ((em = emptyFn.exec(content)) !== null) {
    const lineNum = content.slice(0, em.index).split(/\r?\n/).length;
    const ctx = content.slice(Math.max(0, em.index - 100), em.index + 10);
    const isStructEnum = /\b(?:struct|union|enum)\s*[^{]*\{\s*\}/.test(ctx) || /\:\s*\{\s*\}/.test(ctx);
    const isControlFlow = /\b(?:if|for|while|else)\s*[\s\(]/.test(ctx) || /\}\s*else\s*\{\s*\}/.test(ctx);
    const isFunctionBody = /\)\s*\{\s*\}/.test(ctx) && !isControlFlow;
    if (!isStructEnum && isFunctionBody && !matches.some((x) => x.line === lineNum && x.tag === "empty_body")) {
      matches.push({ line: lineNum, tag: "empty_body", text: "{}", col: em.index });
    }
  }
  return matches;
}

function extractSymbols(content, ext) {
  const symbols = new Set();
  if (!content) return symbols;
  if (/\.(cpp|c|h|hpp|cc|cxx)$/i.test(ext) || !ext) {
    const fn = /(?:^|\n)\s*(?:[\w:]+\s+)+(\w+)\s*\([^)]*\)\s*(?:const)?\s*[{\s]/gm;
    let m;
    while ((m = fn.exec(content)) !== null) symbols.add(m[1]);
    const cls = /(?:^|\n)\s*class\s+(\w+)/gm;
    while ((m = cls.exec(content)) !== null) symbols.add(m[1]);
    const inc = /#include\s*[<"]([^>"]+)[>"]/g;
    while ((m = inc.exec(content)) !== null) symbols.add("#" + m[1]);
  }
  if (/\.(cs)$/i.test(ext)) {
    const fn = /(?:^|\n)\s*(?:[\w\s]+\s+)(\w+)\s*\([^)]*\)/gm;
    let m;
    while ((m = fn.exec(content)) !== null) symbols.add(m[1]);
    const cls = /(?:^|\n)\s*(?:public\s+)?class\s+(\w+)/gm;
    while ((m = cls.exec(content)) !== null) symbols.add(m[1]);
  }
  if (/\.(py)$/i.test(ext)) {
    const fn = /(?:^|\n)\s*def\s+(\w+)\s*\(/gm;
    let m;
    while ((m = fn.exec(content)) !== null) symbols.add(m[1]);
    const cls = /(?:^|\n)\s*class\s+(\w+)/gm;
    while ((m = cls.exec(content)) !== null) symbols.add(m[1]);
  }
  return symbols;
}

function bracketBalance(content) {
  if (!content) return true;
  let round = 0, curly = 0, square = 0;
  const skip = /"[^"]*"|'[^']*'|\/\/[^\n]*|\/\*[\s\S]*?\*\//g;
  const cleaned = content.replace(skip, "");
  for (const c of cleaned) {
    if (c === "(") round++; else if (c === ")") round--;
    if (c === "{") curly++; else if (c === "}") curly--;
    if (c === "[") square++; else if (c === "]") square--;
    if (round < 0 || curly < 0 || square < 0) return false;
  }
  return round === 0 && curly === 0 && square === 0;
}

function lineCount(content) {
  return (content || "").split(/\r?\n/).length;
}

function runReview(beforeContent, afterContent, filePath) {
  const issues = [];
  const ext = (filePath || "").split(".").pop() || "";
  const beforeLines = lineCount(beforeContent);
  const afterLines = lineCount(afterContent);
  const lineDelta = beforeLines - afterLines;
  const lineDropPct = beforeLines > 0 ? (lineDelta / beforeLines) * 100 : 0;
  if (lineDropPct > 30 && (lineDelta > 5 || (beforeLines < 100 && lineDelta > 2))) {
    issues.push(`Line count dropped ${lineDelta} (${lineDropPct.toFixed(0)}%). Possible functionality loss.`);
  }
  if (afterLines === 0 && beforeLines > 0) {
    issues.push("File emptied. Severe functionality loss.");
  }
  if (!bracketBalance(afterContent)) {
    issues.push("Bracket/brace imbalance. Syntax likely broken.");
  }
  const beforeSyms = extractSymbols(beforeContent, ext);
  const afterSyms = extractSymbols(afterContent, ext);
  const lost = [...beforeSyms].filter((s) => !afterSyms.has(s));
  if (lost.length > 0 && lost.length >= beforeSyms.size * 0.2) {
    issues.push(`Symbols removed: ${lost.slice(0, 10).join(", ")}${lost.length > 10 ? "..." : ""}. Possible functionality loss.`);
  }
  const emptyBefore = !beforeContent || beforeContent.trim().length === 0;
  const emptyAfter = !afterContent || afterContent.trim().length === 0;
  if (!emptyBefore && emptyAfter) {
    issues.push("Content removed entirely.");
  }
  const placeholderMatches = scanPlaceholderLike(afterContent);
  const beforePlaceholders = scanPlaceholderLike(beforeContent);
  if (placeholderMatches.length > 0) {
    const byTag = {};
    for (const m of placeholderMatches) {
      byTag[m.tag] = (byTag[m.tag] || 0) + 1;
    }
    const introduced = placeholderMatches.length - beforePlaceholders.length;
    const tagSummary = Object.entries(byTag).map(([k, v]) => `${k}:${v}`).join(", ");
    issues.push(`Placeholder/demo/simulated/mock detected: ${tagSummary}. Line(s): ${[...new Set(placeholderMatches.map((m) => m.line))].slice(0, 8).join(", ")}${placeholderMatches.length > 8 ? "..." : ""}.`);
    if (introduced > 0) {
      issues.push(`Edit introduced ${introduced} new placeholder-like pattern(s). Replace with real implementation.`);
    }
  }
  return {
    degraded: issues.length > 0,
    issues,
    placeholderMatches,
    metrics: { beforeLines, afterLines, lineDelta, lineDropPct, symbolsLost: lost.length, symbolCountBefore: beforeSyms.size, symbolCountAfter: afterSyms.size, placeholderCount: placeholderMatches.length },
  };
}

module.exports = { runReview, extractSymbols, bracketBalance, lineCount, scanPlaceholderLike, PLACEHOLDER_PATTERNS };
