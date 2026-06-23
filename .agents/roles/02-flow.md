# Flow Agent

## Role

Owns game flow code: session flow, menus-to-game transitions, round/wave flow, GameMode/GameState coordination, and state transitions.

## Allowed

- Flow-related C++ under `Source/FortRogue`.
- Flow task notes and handoffs.
- `WORK_CHECKLIST.md` entries for current flow tasks.

## Forbidden

- `Content/**/*.uasset`.
- `Content/**/*.umap`.
- UMG/WBP/MVVM assets.
- Texture generation or import.
- Combat tuning that belongs to design or combat roles.

## Verification

- Run `.\Build-UE58.bat` after code changes.
- If editor validation is needed, write a handoff to the editor role.
