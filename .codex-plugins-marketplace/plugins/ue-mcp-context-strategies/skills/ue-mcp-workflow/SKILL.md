---
name: ue-mcp-workflow
description: Use when driving Unreal Engine editor via the ue-mcp MCP server. Covers the required order of operations (status check first), editor lifecycle, project scoping, and what to do when the bridge says "still initializing". Pulls in automatically any time the user asks to use an Unreal project or references ue-mcp tools.
---

# ue-mcp workflow

The `ue-mcp` MCP drives a live Unreal Engine editor through a C++ bridge plugin.
This project uses the `micro` context strategy. Call category actions through:

```text
tools(action="call", category="<category>", method="<action>", args={...})
```

Use `tools(action="describe", category="<category>")` before guessing an action
or its parameters. In `full` or `lean` mode, the equivalent direct form is
`<category>(action="<action>", ...)`.

## Start every session with a status check

**Always** call
`tools(action="call", category="project", method="get_status", args={})`
before anything else. It tells you:

- Whether the bridge is connected
- Which project is loaded
- Whether the editor is responsive

If status reports `not connected` / `editor not running`, call
`tools(action="call", category="editor", method="start_editor", args={})` to
launch UE. Wait for the status check to succeed before issuing other calls —
handlers that need the editor world will return `"Editor is still initializing.
Please wait and retry."` if you call them too early.

## Orient yourself before authoring

Before writing, read. Common orientation calls:

- `tools(action="call", category="level", method="get_outliner", args={})` —
  what's in the current level
- `tools(action="call", category="asset", method="list", args={directory:
  "/Game/...", recursive: true})` — what assets exist
- `tools(action="call", category="asset", method="search_fts", args={query:
  "..."})` — ranked search across names/classes/paths; call `reindex_fts` first
- `tools(action="call", category="reflection", method="reflect_class",
  args={className: "StaticMeshActor"})` — inspect any UE class
- `tools(action="call", category="project", method="list_project_modules",
  args={})` — native C++ modules in the project

## Mutation recipe

For any write action:

1. Call the read/list variant first to confirm the target exists and capture the current shape (e.g. `tools(action="call", category="blueprint", method="read", args={assetPath: ...})` before `add_node`).
2. Issue the write. Most write handlers return `{ success, existed, created, updated, rollback? }` — honor `existed` as "idempotent no-op" and `updated` as "real change made".
3. If a rollback record came back and you hit a later failure in the same logical unit, call the rollback method yourself (the TS flow runner does this automatically when tasks are composed via `ue-mcp.yml`).

## Common pitfalls

- **Action typos silently fail** — each tool validates `action` against an enum; a typo returns `Unknown action '<x>'. Available: <list>`. Read the `Available:` list rather than guessing.
- **Asset paths use `/Game/...`**, not filesystem paths. Package paths (folder) vs object paths (`Folder/Name.Name`) matter — most create handlers take `packagePath` (folder) plus `name`; most read handlers take `assetPath` (full object path).
- **`execute_python` is an escape hatch, not a shortcut** — if a native tool
  exists for the job, use it. Call it in micro mode with
  `tools(action="call", category="editor", method="execute_python",
  args={...})`. The feedback hook will offer to report the native-tool gap.
- **Editor must be fully loaded** for asset registry queries. If a `list_*` action retries with "still initializing", the editor is still starting — wait a few seconds and retry.

## When something truly can't be done

If no native action covers your case:

1. Use `tools(action="call", category="reflection",
   method="search_classes", args={query: ...})` or `search_functions` to
   confirm no UFUNCTION exists.
2. Use `tools(action="call", category="editor", method="execute_python",
   args={...})` as a last resort.
3. If the user agrees to report the gap, call `tools(action="call",
   category="feedback", method="submit", args={...})`.
