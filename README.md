# FortRogue

FortRogue is an Unreal Engine 5.7 C++ project.

## Project Layout

- `FortRogue.uproject` - Unreal project file.
- `Source/` - C++ gameplay code and build targets.
- `Config/` - Unreal project configuration.
- `Content/` - Unreal assets.

Generated folders such as `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/`, and IDE state are intentionally ignored by Git.

## Requirements

- Unreal Engine 5.7
- Visual Studio with C++ game development tools, or another Unreal-compatible C++ IDE

Enabled plugins:

- GameplayAbilities
- CommonUI
- Paper2D
- ModelingToolsEditorMode

## Getting Started

1. Clone the repository.
2. Open `FortRogue.uproject` with Unreal Engine 5.7.
3. If needed, regenerate IDE project files from the `.uproject`.
4. Build the `FortRogueEditor` target.
5. Open the project in the Unreal Editor.

## Version Control Notes

Commit source files, config files, project files, and `.uasset` content. Do not commit generated build output or local editor caches.
