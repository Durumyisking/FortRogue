#!/usr/bin/env node

import fs from "node:fs";
import os from "node:os";
import path from "node:path";

const debugEnabled =
  Boolean(process.env.CODEX_UE_HOOK_DEBUG) ||
  Boolean(process.env.CLAUDE_UE_HOOK_DEBUG);

function debug(message) {
  if (debugEnabled) {
    process.stderr.write(`unreal-context.mjs: ${message}\n`);
  }
}

function isProjectRoot(directory) {
  for (const marker of [
    "GenerateProjectFiles.bat",
    "GenerateProjectFiles.sh",
    "GenerateProjectFiles.command",
  ]) {
    if (fs.existsSync(path.join(directory, marker))) {
      return true;
    }
  }

  try {
    return fs.readdirSync(directory).some((entry) => entry.endsWith(".uproject"));
  } catch {
    return false;
  }
}

function findProjectRoot(startDirectory) {
  let directory = path.resolve(startDirectory);
  while (true) {
    if (isProjectRoot(directory)) {
      return directory;
    }
    const parent = path.dirname(directory);
    if (parent === directory) {
      return null;
    }
    directory = parent;
  }
}

function firstUproject(directory) {
  try {
    return (
      fs
        .readdirSync(directory)
        .filter((entry) => entry.endsWith(".uproject"))
        .sort()[0] ?? ""
    );
  } catch {
    return "";
  }
}

function hasUnrealMcpConfig(configPath) {
  try {
    const config = fs.readFileSync(configPath, "utf8");
    return (
      /\[mcp_servers[.'"]+unreal-mcp["']?\]/i.test(config) ||
      /127\.0\.0\.1:8000\/mcp/i.test(config)
    );
  } catch {
    return false;
  }
}

const projectRoot = findProjectRoot(process.cwd());
if (!projectRoot) {
  debug(`no Unreal Engine project marker found walking up from ${process.cwd()}`);
  process.exit(0);
}

const projectType = fs.existsSync(path.join(projectRoot, "Engine"))
  ? "engine"
  : "game";
const uprojectFilename = firstUproject(projectRoot);
const projectConfigPath = path.join(projectRoot, ".codex", "config.toml");
const userConfigPath = path.join(os.homedir(), ".codex", "config.toml");
const mcpConfigPresent =
  hasUnrealMcpConfig(projectConfigPath) || hasUnrealMcpConfig(userConfigPath);

debug(
  `project root: ${projectRoot} (type=${projectType}, ` +
    `uproject=${uprojectFilename}, codex_mcp=${mcpConfigPresent})`,
);

let context = "This working directory is an Unreal Engine project.";
if (projectType === "engine") {
  context += " It is an Engine source tree.";
} else if (uprojectFilename) {
  context += ` The project is \`${uprojectFilename}\`.`;
}
context +=
  " Prefer Unreal Engine conventions (C++/UObject patterns, Slate, UHT reflection) when suggesting code.";
context +=
  " Use the `unreal-mcp` skill for tasks that involve driving the Unreal Editor via Epic MCP.";
if (mcpConfigPresent) {
  context += " Codex already has an `unreal-mcp` MCP connection configured.";
} else {
  context +=
    " No Codex `unreal-mcp` connection was detected. Run `ModelContextProtocol.GenerateClientConfig Codex` in the editor console.";
}

process.stdout.write(
  `${JSON.stringify({
    hookSpecificOutput: {
      hookEventName: "SessionStart",
      additionalContext: context,
    },
  })}\n`,
);
