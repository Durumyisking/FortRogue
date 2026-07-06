---
name: ue-mcp-blueprint
description: Use when authoring or modifying Unreal Blueprint assets through ue-mcp. Covers the read-then-write discipline, node/pin wiring, component (SCS) hierarchy, variables, interfaces, compilation, and the difference between pin defaults and linked inputs. Pulls in any time the user asks to create, edit, or inspect a Blueprint.
---

# ue-mcp blueprint authoring

The Blueprint category covers reading, authoring, and compiling Blueprints. This
project uses micro mode, so every example calls the `tools` gateway. In
full/lean mode, the equivalent direct form is
`blueprint(action="<method>", ...)`. The default workflow is
**read → mutate → compile**, never fire-and-forget.

## Discovery before authoring

For any existing Blueprint:

- `tools(action="call", category="blueprint", method="read",
  args={assetPath: ...})` — structure (parent, components, graphs)
- `tools(action="call", category="blueprint", method="read_graph_summary",
  args={assetPath: ..., graphName: ...})` — lightweight node+edge summary;
  use before `read_graph`
- `tools(action="call", category="blueprint", method="list_graphs",
  args={assetPath: ...})` — every graph in the Blueprint
- Call `list_variables`, `list_functions`, or `list_local_variables` through
  the same gateway to inspect the member surface
- `tools(action="call", category="blueprint", method="get_execution_flow",
  args={assetPath: ..., entryPoint: ...})` — trace exec pins
- `tools(action="call", category="blueprint", method="get_dependencies",
  args={assetPath: ..., reverse: false})` — dependencies; set `reverse: true`
  for callers

## Mutation recipe

1. **Create the skeleton** — `tools(action="call", category="blueprint",
   method="create", args={assetPath: ..., parentClass: ...})`.
2. **Add variables + components** — `add_variable`, `add_component` (pass `parentComponent` for SCS hierarchy).
3. **Build graphs** — `add_node` each K2Node, `set_node_property` for pin defaults, `connect_pins` to wire exec + data.
4. **Compile** — `tools(action="call", category="blueprint",
   method="compile", args={assetPath: ...})`. Compilation errors come back in
   the result; fix them before proceeding.

## Node wiring fundamentals

- `add_node` takes `nodeClass` as a K2Node class short name (`K2Node_CallFunction`, `K2Node_VariableGet`, `K2Node_DynamicCast`, `K2Node_IfThenElse`, etc.) plus `nodeParams` for class-specific fields (`FunctionReference`, `VariableReference`, `TargetType`).
- Pin defaults vs linked values: `set_node_property` writes a literal default onto a pin; `connect_pins` wires a pin to another node's output. A literal default is ignored once a pin is linked.
- `read_node_property` reads either a pin default or a reflected node UPROPERTY — use this to verify the pin was actually set before compiling.
- Graphs with duplicate names (rare but possible after rename) can be disambiguated by passing `graphIndex` alongside `graphName`.

## Components (SCS)

- `add_component` creates a node in the Simple Construction Script. Pass `parentComponent` to put the new component under an existing parent — otherwise it becomes a top-level child of the scene root.
- `set_component_property` writes on the child BP's InheritableComponentHandler override template, not on the parent — the parent stays untouched. This matters when editing inherited components.
- `read_component_properties` dumps every UPROPERTY on the template, including array contents.
- `reparent_component` moves an SCS node to a new parent.

## CDO (class defaults)

- `set_class_default` writes a UPROPERTY on the Blueprint CDO (the class default object). For actor tick settings specifically, use `set_actor_tick_settings` — it handles `bCanEverTick`, `bStartWithTickEnabled`, `TickInterval` in one call.

## Interfaces + event dispatchers

- `create_interface` + `add_interface` — the implement-side.
- `add_event_dispatcher` — fires a multicast delegate from the BP.

## Verify before compile

- Call `validate` through the Blueprint category for compiler diagnostics
  without saving.
- Call `read_graph_summary` after mutations to confirm the graph shape.
