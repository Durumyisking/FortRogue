---
name: ue-mcp-native-cpp
description: Use when writing or modifying native C++ UCLASSes in an Unreal project via ue-mcp. Covers create_cpp_class → write_cpp_file → live_coding_compile loop, when to use build vs Live Coding, and the add_module_dependency workflow. Pulls in any time the user asks to write a new native class, add a UPROPERTY, or implement a UFUNCTION.
---

# ue-mcp native C++ authoring

The project category wraps `GameProjectUtils::AddCodeToProject` and
`ILiveCodingModule`. This project uses micro mode, so call every project action
as `tools(action="call", category="project", method="<action>", args={...})`.
In full/lean mode, the equivalent direct form is
`project(action="<action>", ...)`.

## The authoring loop

1. **Pick a module.** Call `list_project_modules` with empty `args`.
2. **Create the class.** Call `create_cpp_class` with `args={className: ...,
   parentClass: ..., moduleName: ..., classDomain: ..., subPath: ...}`. The
   class-name prefix is derived from `parentClass`; pass `"BotSpawner"`, not
   `"ABotSpawner"`.
3. **Inspect the generated files.** Call `read_cpp_header` with
   `args={headerPath: ...}` and `read_cpp_source` with
   `args={sourcePath: ...}`.
4. **Append declarations and bodies.** Call `write_cpp_file` with
   `args={path: ..., content: ...}`. Read first, then write the full amended
   contents.
5. **Compile.**
   - **New UCLASS** → call `build`. Live Coding doesn't reliably register
     brand-new UCLASSes; use a full build and editor restart.
   - **Existing UCLASS** → call `live_coding_status`, then
     `live_coding_compile` with `args={wait: true}`.

## Adding module dependencies

When you need a new module in a project's `Build.cs`:

- Call `add_module_dependency` with `args={moduleName: <target Build.cs>,
  dependency: <module to add>, access: "public"|"private"}`.
- Deduplicates automatically. Creates the `AddRange` block if missing.
- Call `build` afterwards.

## Reading the engine source

When you need to know *how* UE does something, reach for engine source lookup:

- Call `read_engine_header` with `args={headerPath: ...}`.
- Call `find_engine_symbol` with `args={symbol: ..., maxResults: ...}`.
- Call `search_engine_cpp` with `args={query: ..., tree: ...,
  subdirectory: ..., maxResults: ...}`.
- Call `list_engine_modules` with empty `args`.

## Pitfalls

- **UCLASSes need UnrealBuildTool + MSVC/Clang** on the host. Same requirement as the editor's own Build menu.
- **`write_cpp_file` refuses writes outside `Source/`** and rejects extensions other than `.h` / `.cpp` / `.inl` / `.cs`.
- **Live Coding ≠ full build.** If behavior doesn't reflect your code changes, check whether you added a new UCLASS, UFUNCTION reflection macro, or UPROPERTY annotation — those usually need a full build.
- **Don't edit Intermediate/ or Binaries/.** These are regenerated.

## Testing the new class

Once a class is compiled and registered, you can typically:

- Call `asset.read_properties` or `reflection.reflect_class` through the
  gateway to verify UE sees the UCLASS.
- Call `blueprint.create` with
  `args={parentClass: "/Script/<Module>.<Class>", ...}` to subclass it.
- Call `level.place_actor` with `args={className: ...}` to place an instance.
