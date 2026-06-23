# FortRogue Parallel Codex Operations

## Model

FortRogue runs parallel Codex CLI sessions as role workers.

- One CLI session owns one branch.
- One branch is checked out in one worktree.
- One worker processes one task at a time.
- Workers finish by committing their branch, updating operations status, and returning to idle.

## Worktrees

| Role | Worktree | Branch | Role file |
| --- | --- | --- | --- |
| Director | `D:\Project\FortRogue-integration` | `codex/integration` | `.agents/roles/00-director.md` |
| Design | `D:\Project\FortRogue-design` | `codex/design` | `.agents/roles/01-design.md` |
| Flow | `D:\Project\FortRogue-flow` | `codex/flow` | `.agents/roles/02-flow.md` |
| Combat | `D:\Project\FortRogue-combat` | `codex/combat` | `.agents/roles/03-combat.md` |
| UI | `D:\Project\FortRogue-ui` | `codex/ui` | `.agents/roles/04-ui.md` |
| Editor | `D:\Project\FortRogue-editor` | `codex/editor` | `.agents/roles/05-editor.md` |
| Texture | `D:\Project\FortRogue-texture` | `codex/texture` | `.agents/roles/06-texture.md` |
| Review | `D:\Project\FortRogue-review` | `codex/review` | `.agents/roles/07-review.md` |

## Operations Folder

Runtime coordination lives outside git:

```text
D:\Project\FortRogue-ops
  STATUS.md
  commands\
  tasks\
  handoff\
  logs\
```

Use this folder for live commands, task tickets, handoffs, and status. Do not copy runtime status into tracked project files unless the user asks for permanent documentation.

## Launch

Start each role from its own terminal:

```powershell
codex --full-auto -C D:\Project\FortRogue-combat --add-dir D:\Project\FortRogue-ops
```

After launch, give the worker the appropriate role prompt from `.agents/prompts/worker-start.md`.

## Task Lifecycle

1. Director writes or updates a task under `D:\Project\FortRogue-ops\tasks\<role>\`.
2. Director tells the target worker to process that task.
3. Worker reads `AGENTS.md`, its role file, and the task file.
4. Worker updates `WORK_CHECKLIST.md` only for the current requested task.
5. Worker implements the minimum scoped change.
6. Worker verifies with the role's required checks.
7. Worker commits only its own work.
8. Worker updates `STATUS.md` and writes handoff notes when needed.
9. Worker reports idle, blocked, or needs-review.

## Merge Order

Prefer this integration order:

1. `codex/design`
2. `codex/flow`
3. `codex/combat`
4. `codex/ui`
5. `codex/texture`
6. `codex/editor`
7. `codex/review` if it contains tracked review docs

Merge editor-owned asset branches last. If binary asset conflicts occur, stop and ask the user which version should win.
