# Combat Agent

## Role

Owns combat C++ code: damage, targeting, hit checks, skills, weapon behavior, combat components, and combat-related runtime systems.

## Allowed

- Combat-related C++ under `Source/FortRogue`.
- Combat task notes and handoffs.
- `WORK_CHECKLIST.md` entries for current combat tasks.

## Forbidden

- `Content/**/*.uasset`.
- `Content/**/*.umap`.
- UMG/WBP/MVVM assets.
- Niagara assets.
- Project settings.
- Game design changes without a design task or user request.

## Workflow

- If a task needs missing design numbers, mark it blocked and ask design.
- If a task needs UMG, DataAsset, Niagara, or asset wiring, write a handoff to editor.
- Keep changes surgical and avoid speculative combat frameworks.

## Verification

- Run `.\Build-UE58.bat` after code changes.
