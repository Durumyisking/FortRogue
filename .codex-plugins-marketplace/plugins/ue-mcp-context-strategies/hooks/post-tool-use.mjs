#!/usr/bin/env node

import fs from "node:fs";
import path from "node:path";

function readStdin() {
  return new Promise((resolve, reject) => {
    const chunks = [];
    process.stdin.on("data", (chunk) => chunks.push(chunk));
    process.stdin.on("end", () =>
      resolve(Buffer.concat(chunks).toString("utf8")),
    );
    process.stdin.on("error", reject);
    if (process.stdin.readableEnded) {
      resolve("");
    }
  });
}

function feedbackDisabled(yaml) {
  const lines = yaml.split(/\r?\n/);
  let inUeMcp = false;
  let inDisableList = false;

  for (const line of lines) {
    if (/^ue-mcp:\s*(?:#.*)?$/.test(line)) {
      inUeMcp = true;
      inDisableList = false;
      continue;
    }
    if (inUeMcp && /^\S/.test(line) && line.trim()) {
      break;
    }
    if (!inUeMcp) {
      continue;
    }

    const disable = line.match(/^\s+disable:\s*(.*)$/);
    if (disable) {
      inDisableList = true;
      if (/\bfeedback\b/.test(disable[1])) {
        return true;
      }
      continue;
    }
    if (inDisableList && /^\s+-\s*feedback\s*(?:#.*)?$/.test(line)) {
      return true;
    }
    if (inDisableList && /^\s+\w[^:]*:/.test(line)) {
      inDisableList = false;
    }
  }

  return false;
}

function feedbackDisabledForCwd() {
  let directory = process.cwd();
  for (let i = 0; i < 32; i += 1) {
    const configPath = path.join(directory, "ue-mcp.yml");
    if (fs.existsSync(configPath)) {
      try {
        return feedbackDisabled(fs.readFileSync(configPath, "utf8"));
      } catch {
        return true;
      }
    }
    const parent = path.dirname(directory);
    if (parent === directory) {
      break;
    }
    directory = parent;
  }
  return true;
}

function isExecutePython(toolInput) {
  if (toolInput?.action === "execute_python") {
    return true;
  }
  return (
    toolInput?.action === "call" &&
    toolInput?.category === "editor" &&
    toolInput?.method === "execute_python"
  );
}

async function main() {
  if (feedbackDisabledForCwd()) {
    return;
  }

  const raw = (await readStdin()).replace(/^\uFEFF/, "");
  if (!raw.trim()) {
    return;
  }

  let payload;
  try {
    payload = JSON.parse(raw);
  } catch {
    return;
  }

  if (!isExecutePython(payload.tool_input)) {
    return;
  }

  const message = [
    "[MANDATORY] execute_python was just used as a workaround — a native ue-mcp tool could not handle this task.",
    "In your NEXT message to the user, you MUST say:",
    '"I had to use execute_python to <describe what and why>. Would you like to submit feedback so this can become a native tool?"',
    "If the user agrees, submit title, summary, pythonWorkaround, and idealTool.",
    'In micro mode call tools(action="call", category="feedback", method="submit", args={...}).',
    'In full or lean mode call feedback(action="submit", ...).',
    "The MCP server will block until the user approves or declines the exact issue body in the Codex UI; you cannot bypass or pre-confirm that prompt.",
    "If the user declines or elicitation is unavailable, do not retry; resume the task.",
  ].join("\n");

  process.stdout.write(`${JSON.stringify({ systemMessage: message })}\n`);
}

main().catch(() => process.exit(0));
