---
name: ue-mcp-niagara
description: Use when authoring Niagara VFX systems via ue-mcp — creating systems and emitters, adding renderers, setting module inputs and static switches, building HLSL modules, and batching operations. Pulls in any time the user asks for a particle system, VFX, Niagara emitter, or motion matching cost visualization.
---

# ue-mcp Niagara authoring

The Niagara category covers systems, emitters, modules, renderers, and HLSL
authoring. This project uses micro mode, so call each action as
`tools(action="call", category="niagara", method="<action>", args={...})`.
In full/lean mode, the equivalent direct form is `niagara(action="<action>", ...)`.

## Create and inspect

- Call `create` with `args={name: ..., packagePath: ...}` for a blank system.
- Call `create_system_from_spec` with
  `args={name: ..., packagePath: ..., emitters: [{path: ...}]}`.
- Call `list_emitters` with `args={systemPath: ...}` or `get_info` with
  `args={assetPath: ...}` to inspect shape.
- Call `inspect_data_interfaces` and `list_system_parameters` with
  `args={systemPath: ...}`.

## Renderers

- Call `list_renderers` with `args={systemPath: ..., emitterName: ...}`.
- Call `add_renderer` with `args={systemPath: ..., rendererType:
  "sprite"|"mesh"|"ribbon", emitterName: ...}`.
- Call `set_renderer_property` with `args={systemPath: ..., rendererIndex: ...,
  propertyName: ..., value: ..., emitterName: ...}`.
- Call `remove_renderer` with `args={systemPath: ..., rendererIndex: ...,
  emitterName: ...}`.

## Module inputs + static switches (v0.7.14)

- Call `list_module_inputs` with `args={systemPath: ..., emitterName: ...,
  stackContext: ...}`.
- Call `set_module_input` with `args={systemPath: ..., moduleName: ...,
  inputName: ..., value: ..., emitterName: ..., stackContext: ...}`.
  Inputs already overridden through the stack editor's override-map node are
  not changed; clear that override in-editor if needed.
- Call `list_static_switches` and `set_static_switch` through the same gateway.
  Format the value as `"true"`/`"false"` for bool or an integer literal for
  int/byte/enum.

## HLSL module authoring (v0.7.14)

- Call `create_module_from_hlsl` with `args={name: ..., hlsl: ...,
  packagePath: ...}`.
- Call `get_compiled_hlsl` with `args={systemPath: ..., emitterName: ...}`.

## Batching (v0.7.14)

- Call `batch` with `args={ops: [{action: ..., params: {...}}]}`. It runs
  Niagara sub-actions fail-fast and returns `stoppedAt`; nested batches are
  rejected.

## Typical authoring flow

1. Create the system + emitters (`create_system_from_spec` or step-by-step).
2. Inspect the stack: `list_module_inputs` to see what you're working with.
3. Tune: `set_module_input` / `set_static_switch` for literal values; `set_renderer_property` for visual output.
4. For custom HLSL behavior: `create_module_from_hlsl` to produce a reusable module, then `add_emitter` / reference it from your stack.
5. Optional: wrap a long sequence in `batch` so a mid-sequence failure gets reported as a single result.

## Pitfalls

- **Emitter references are version-pinned.** When using `create_system_from_spec`, the handler uses the source emitter's current exposed version — save the emitter first if you've been editing it.
- **`set_emitter_property` uses reflection on `FVersionedNiagaraEmitterData`.** Not all properties are writable this way (some require the stack editor's validators). If `success: false`, the response lists available properties so you can pick the right one.
- **GPU emitters** expose `get_compiled_hlsl` results only when `simTarget == GPU`. CPU emitters return a note explaining no HLSL is available.
