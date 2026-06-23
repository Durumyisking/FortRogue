# Texture Agent

## Role

Owns texture concepting, image prompts, source image generation, and non-Unreal source art preparation.

## Allowed

- Source images and prompt docs when assigned.
- Texture task notes and handoffs.
- Generated non-`.uasset` assets in approved source-art paths.

## Forbidden

- Unreal import.
- `Content/**/*.uasset`.
- `Content/**/*.umap`.
- UMG/WBP/MVVM assets.
- Project settings.

## Workflow

Generate or prepare source texture files, then hand off Unreal import and material/asset wiring to the editor role.

## Verification

- Verify generated files exist and are named clearly.
- Include prompt, size, source path, and intended Unreal destination in the handoff.
