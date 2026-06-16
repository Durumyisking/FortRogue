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

## Terrain Map Workflow

FortRogue maps are stored as `FortRogue Terrain Map Definition` data assets and consumed by `AFortRogueDestructibleTerrain` at runtime.

To create or edit a map:

1. In the Content Browser, create a new `FortRogue Terrain Map Definition` asset.
2. Set `CellsX`, `CellsZ`, and `CellSize` for the terrain resolution and world scale.
3. Edit `SolidMask` and `TextureLayerMask` directly only for small tests, or use the Blueprint-callable helpers on the asset:
   - `Clear`, `FillRect`, `ApplyCircle`, and `ApplyCircleStroke` for solid terrain.
   - `FillTexturedRect`, `ApplyTexturedCircle`, and `ApplyTexturedCircleStroke` for solid terrain with a texture layer.
   - `ImportSolidMaskFromTexture*` for texture-driven masks.
4. Set `TextureLayers` to define the material/color used by each texture layer.
5. Set `PlayerSpawnLocal` and `EnemySpawnLocal`. X is relative to the center of the terrain, Z is local height above the terrain actor.

To use a map in game:

1. Open the active `FortRogueGameMode` Blueprint, such as `Content/FortRogue/Game/B_FortRogueGameMode`.
2. Assign the map asset to `TerrainMapDefinition`.
3. Play the level. The game mode spawns `AFortRogueDestructibleTerrain`, applies the map dimensions/masks, then resolves player and enemy spawn locations from the map asset.

Character stats are stored per `FortRogueCharacterDefinition` asset:

- `MaxMoveBudget`: movement amount per turn.
- `ShotPowerMultiplier`: projectile force multiplier applied to the charged shot power.
- `MaxHealth`: character health.

## Version Control Notes

Commit source files, config files, project files, and `.uasset` content. Do not commit generated build output or local editor caches.
