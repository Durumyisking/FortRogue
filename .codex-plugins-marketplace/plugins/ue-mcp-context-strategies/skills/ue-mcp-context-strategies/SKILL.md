---
name: ue-mcp-context-strategies
description: Manage and use ue-mcp full, lean, and micro context strategies in Codex. Use when inspecting or changing `ue-mcp.yml`, reducing Unreal MCP context/token cost, choosing a strategy, restarting after a strategy change, or translating direct category calls into the micro `tools` gateway.
---

# ue-mcp Context Strategies

Use ue-mcp's native Codex support. Do not reimplement the MCP server or its
strategy logic.

## Inspect or switch strategy

Run from the Unreal project directory:

```powershell
npx -y ue-mcp@1.1.6 context status
npx -y ue-mcp@1.1.6 context full
npx -y ue-mcp@1.1.6 context lean
npx -y ue-mcp@1.1.6 context micro
```

The command updates `ue-mcp.context.strategy` in `ue-mcp.yml`. Start a new
Codex thread after changing it so the MCP server restarts.

`UE_MCP_CONTEXT_STRATEGY` overrides `ue-mcp.yml`. If the reported strategy does
not follow the file, inspect the `ue-mcp` server environment and remove or
change that override.

## Choose a strategy

- `full`: Advertise every category and action inline. Use for maximum immediate
  discoverability when context cost is acceptable.
- `lean`: Advertise category tools with trimmed descriptions plus `catalog`.
  Use `catalog(action="search", query="...")`,
  `catalog(action="list_categories")`, or `<category>(action="describe")`
  before direct category calls.
- `micro`: Advertise `tools` and `flow`. Use this for the smallest initial
  context and discover schemas on demand.

## Use micro correctly

Discover, describe, then call:

```text
tools(action="list_categories")
tools(action="describe", category="blueprint")
tools(
  action="call",
  category="blueprint",
  method="create",
  args={...}
)
```

`method` is the category action name. Put every action-specific parameter in
`args`. Start each session with:

```text
tools(action="call", category="project", method="get_status", args={})
```

Run a project flow directly:

```text
flow(action="run", flowName="<name>")
```

Do not call hidden category tools directly in micro mode. For example,
`blueprint(action="compile", assetPath=path)` becomes:

```text
tools(
  action="call",
  category="blueprint",
  method="compile",
  args={assetPath: path}
)
```
