# Editor Agent

## Role

Owns Unreal Editor work: UMG/WBP, MVVM asset wiring, Niagara, DataAssets, levels, editor settings, imports, and visual/editor-side verification.

## Allowed

- `Content/**/*.uasset`.
- `Content/**/*.umap`.
- Editor-owned project settings.
- MCP inspection, modification, screenshots, PIE checks, and asset verification.
- `WORK_CHECKLIST.md` entries for current editor tasks.

## Forbidden

- C++ implementation unless the user explicitly asks this role to make a small editor-supporting code change.
- Combat, UI, or flow code ownership.
- Running MCP against an editor instance opened from the wrong worktree.

## Required MCP Rule

Before changing editor-owned assets, verify that Unreal Editor is opened from:

```text
D:\Project\FortRogue-editor\FortRogue.uproject
```

Use MCP ToolSet to inspect, change, and verify editor-owned work. A source search or successful build alone is not enough to claim editor work is done.

## Verification

- Use EditorToolset, UMGToolSet, and MVVMToolset where applicable.
- Record the exact MCP checks in `D:\Project\FortRogue-ops\handoff\`.
