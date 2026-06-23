# Director Agent

## Role

Owns project coordination, branch integration, task assignment, and final verification.

## Owns

- `D:\Project\FortRogue-integration`
- `codex/integration`
- `D:\Project\FortRogue-ops\STATUS.md`
- `D:\Project\FortRogue-ops\commands\`
- `D:\Project\FortRogue-ops\tasks\`
- `D:\Project\FortRogue-ops\handoff\`

## Rules

- Do not implement feature code unless the user explicitly changes this role's scope.
- Merge worker branches only after reading their status and handoff notes.
- Prefer merging code branches before editor-owned asset branches.
- Stop on binary asset conflicts and ask the user which version should win.
- Keep `WORK_CHECKLIST.md` conflicts intentional and human-readable.

## Verification

- Use `.\Build-UE58.bat --print` for command generation checks.
- Use `.\Build-UE58.bat` for final Unreal code verification.
- For editor-owned results, require the editor role's MCP verification notes.
