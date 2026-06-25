# AGENTS.md

## Build Testing

When you need to build or verify Unreal code changes, use the project build batch file:

```bat
.\Build-UE58.bat
```

Use `--print` when you only need to verify the generated Unreal Build Tool command:

```bat
.\Build-UE58.bat --print
```

The batch file is configured for this project's UE 5.8 install:

```text
D:\Program Files\Epic Games\UE_5.8
```

Do not call another project's build script for FortRogue.
---

## Coding
- 언리얼 기능을 적극 활용하려고 노력하고, 언리얼 기능으로만 구현이 불가능할때 새로운 기능을 만든다.
- 하드코딩 방식이 아니라 가능한 에디터에서 추가 및 수정이 가능한 형태로 작성한다.

## Unreal Editor MCP

When a request involves modifying Unreal Editor-owned assets or editor state, use the MCP ToolSet before treating the work as complete.

- This includes Blueprint, UMG/WBP, level/map, data asset, project setting, and other editor-side changes.
- First check `FortRogue.uproject` for the relevant enabled plugins instead of assuming the ToolSet is unavailable. For this project, expected plugins include `ModelContextProtocol`, `MCPClientToolset`, `EditorToolset`, `UMGToolSet`, `MVVMToolset`, and other `*Toolset` plugins.
- If the current Codex tool list does not show Unreal MCP tools, do not stop there. Verify whether Unreal's local MCP server is available from the project/editor settings.
- The usual FortRogue endpoint is `http://127.0.0.1:8000/mcp`, based on `Saved/Config/WindowsEditor/EditorPerProjectUserSettings.ini` under `/Script/ModelContextProtocolEngine.ModelContextProtocolSettings` (`ServerPortNumber=8000`, `ServerUrlPath=/mcp`, `bAutoStartServer=True`).
- If the endpoint is not reachable, check whether `UnrealEditor.exe` is running and whether the engine plugins exist under `D:\Program Files\Epic Games\UE_5.8\Engine\Plugins\Experimental\Toolsets`.
- Use `EditorToolset` for editor state, assets, selection, PIE, and screenshots. Use `UMGToolSet` for WBP/UMG structure and widget changes. Use `MVVMToolset` for MVVM/view model related editor work.
- Do not claim editor-side work is done from source search, asset path checks, or a successful build alone.
- Use MCP ToolSet to inspect the relevant editor state, perform the requested editor changes when possible, and verify the result.
- Only say MCP ToolSet is unavailable after checking the project plugins, the local MCP endpoint, and the running editor state. If it is still unavailable, explain exactly which check failed before using a fallback such as a commandlet or source-only change.

## Work Checklist

Checklist work is opt-in only.

- Do not create, read, or update `WORK_CHECKLIST.md` unless the user explicitly asks for checklist work.
- Do not add or update checklist entries as a prerequisite for ordinary coding, asset, build, or verification work.
- When the user explicitly asks to create or update checklist entries, only make the requested checklist changes.
- Create `WORK_CHECKLIST.md` only when the user explicitly asks to create a checklist file.
- When the user asks to perform checklist work, inspect the existing `WORK_CHECKLIST.md` and work only from entries already present there. Do not create additional checklist entries during that work.

When explicitly creating or updating checklist entries, group entries by feature name first, then by date.

- Dates use `YY MM DD` format.
- Use this exact entry style:

```md
UI 체크리스트

26 06 21
- 캐릭터 체력바 [v]
- 캐릭터 이동 게이지 [ ]
```

Status meanings:

- `[ ]`: planned or not completed.
- `[v]`: completed and protected from incidental edits.
- `[수정]`: the user explicitly requested changes to a completed item.

Work rules:

- These rules apply only during explicit checklist work.
- If requested checklist work touches an item already marked `[v]`, do not modify it unless the user explicitly asked for that item to be changed.
- When the user explicitly asks to modify a completed checklist item, change that checklist entry to `[수정]` before editing.
- After an explicitly requested checklist item is implemented and verified, mark that existing checklist entry as `[v]`.
- If it is unclear whether a file or code path belongs to a completed checklist item during checklist work, treat it as protected and ask the user before editing.
- Do not create speculative checklist items for work that was not requested.

## Parallel Codex CLI Operations

FortRogue uses a one-agent, one-branch, one-worktree model for parallel Codex CLI work.

- Each Codex CLI session owns exactly one worktree and one branch.
- Shared project rules live in this `AGENTS.md`.
- Role-specific rules live under `.agents/roles/`.
- Runtime task state, command queues, and handoff notes live outside git in `D:\Project\FortRogue-ops`.
- Launch role sessions with `--add-dir D:\Project\FortRogue-ops` so they can read and update the shared operations folder.
- A worker handles one assigned task at a time, records the result, then returns to idle.
- Only the integration/director role merges branches or edits shared operational state broadly.
- Do not let feature workers merge other branches into their worktree unless the director explicitly asks.

Editor-owned assets are single-owner:

- Only the editor role may modify `Content/**/*.uasset`, `Content/**/*.umap`, UMG/WBP, MVVM assets, Niagara assets, DataAssets, levels, or editor/project settings.
- The texture role may create source images or prompts, but Unreal import and `.uasset` creation belong to the editor role.
- When using Unreal MCP, the running Unreal Editor must have opened the same worktree that the editor role owns, normally `D:\Project\FortRogue-editor\FortRogue.uproject`.
- If the editor is opened from another worktree, stop and reopen the correct `.uproject` before making MCP changes.
