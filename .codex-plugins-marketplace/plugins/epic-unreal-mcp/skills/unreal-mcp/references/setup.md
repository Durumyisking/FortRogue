# First-Time MCP Server Setup

Read this when the `unreal-mcp` MCP server is not yet wired up to a project.
Skip otherwise: once the three steps below are done, the editor starts the
server on every launch and Codex keeps using the configured connection.

The goal is three things:
1. Enable the `ModelContextProtocol` and `AllToolsets` plugins in the project.
2. Make the editor auto-start the MCP server on launch.
3. Generate the `.codex/config.toml` entry Codex reads to connect.

Walk the user through them in order. Do not skip the user's `.uproject` edit silently. Confirm the file path first.

## 1. Enable the plugins in the `.uproject`

Two plugins are required. `ModelContextProtocol` is the server and transport; `AllToolsets` provides the tools. With only `ModelContextProtocol` enabled the server starts but exposes no tools.

Open the project's `.uproject` file. In the `Plugins` array, ensure both entries:

```json
{
  "Name": "ModelContextProtocol",
  "Enabled": true
},
{
  "Name": "AllToolsets",
  "Enabled": true
}
```

If the array doesn't exist, create it. If either entry exists with `"Enabled": false`, flip it to `true`.

`AllToolsets` is an editor-only aggregator with `EnabledByDefault` off, so it must be enabled explicitly. To expose only a subset of tools, enable the specific toolset plugins you want instead of `AllToolsets`.

## 2. Enable auto-start

The default is for the MCP server to stay stopped. To start it manually in a session, run `ModelContextProtocol.StartServer` from the editor console.

To start it automatically on every editor launch, add the snippet below to the per-user editor config file:

`<Project>/Saved/Config/<Platform>Editor/EditorPerProjectUserSettings.ini`

This is the file the editor writes when you toggle the setting in Editor Preferences. It is per-user and not source-controlled.

```ini
[/Script/ModelContextProtocolEngine.ModelContextProtocolSettings]
bAutoStartServer=True
```

Optional overrides if the defaults conflict with another local service:

```ini
ServerPortNumber=8000
ServerUrlPath=/mcp
```

A command-line alternative also works: pass `-ModelContextProtocolStartServer` (and optionally `-ModelContextProtocolPort=<port>`) to the editor. Prefer the `.ini` because it's persistent.

## 3. Generate Codex MCP config

Either run a console command from inside the editor, or add the Codex MCP
connection with the Codex CLI.

**From a running editor (preferred):** run
`ModelContextProtocol.GenerateClientConfig Codex` in the console (or `All` to
write configs for every supported client: `ClaudeCode`, `Cursor`, `VSCode`,
`Gemini`, `Codex`). The Codex writer uses TOML and refuses to overwrite an
existing `.codex/config.toml`; use the CLI or edit that file if it already
exists.

The destination depends on the build kind:

- **Source build** (your repo contains `Engine/`): `.codex/config.toml` is
  created under the workspace root alongside `Engine/`.
- **Installed/launcher build**: `.codex/config.toml` is created under the
  project directory next to the `.uproject`.

**Without launching the editor first**, register the default endpoint:

```powershell
codex mcp add unreal-mcp --url http://127.0.0.1:8000/mcp
```

Adjust the URL if the port or path was overridden in step 2.

## Verifying

After the editor is running with the plugin enabled and auto-start on:

- The Output Log shows MCP server startup messages.
- `list_toolsets` (one of the three tool-search meta-tools) returns successfully.
- `/mcp` in Codex lists `unreal-mcp` as connected.

If any of these fail, see `operations.md` for recovery commands.
