param(
    [string] $RawLine,

    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]] $InputArgs
)

$ErrorActionPreference = "Stop"

$DefaultModel = "gpt-5.5"
$DefaultEffort = "high"
$OpsRoot = "D:\Project\FortRogue-ops"

$Roles = @{
    "director"    = @{ Name = "Director"; Worktree = "D:\Project\FortRogue-integration"; RoleFile = ".agents\roles\00-director.md" }
    "integration" = @{ Name = "Director"; Worktree = "D:\Project\FortRogue-integration"; RoleFile = ".agents\roles\00-director.md" }
    "총괄"        = @{ Name = "Director"; Worktree = "D:\Project\FortRogue-integration"; RoleFile = ".agents\roles\00-director.md" }
    "design"      = @{ Name = "Design"; Worktree = "D:\Project\FortRogue-design"; RoleFile = ".agents\roles\01-design.md" }
    "기획"        = @{ Name = "Design"; Worktree = "D:\Project\FortRogue-design"; RoleFile = ".agents\roles\01-design.md" }
    "flow"        = @{ Name = "Flow"; Worktree = "D:\Project\FortRogue-flow"; RoleFile = ".agents\roles\02-flow.md" }
    "게임플로우"  = @{ Name = "Flow"; Worktree = "D:\Project\FortRogue-flow"; RoleFile = ".agents\roles\02-flow.md" }
    "combat"      = @{ Name = "Combat"; Worktree = "D:\Project\FortRogue-combat"; RoleFile = ".agents\roles\03-combat.md" }
    "전투"        = @{ Name = "Combat"; Worktree = "D:\Project\FortRogue-combat"; RoleFile = ".agents\roles\03-combat.md" }
    "ui"          = @{ Name = "UI"; Worktree = "D:\Project\FortRogue-ui"; RoleFile = ".agents\roles\04-ui.md" }
    "editor"      = @{ Name = "Editor"; Worktree = "D:\Project\FortRogue-editor"; RoleFile = ".agents\roles\05-editor.md" }
    "에디터"      = @{ Name = "Editor"; Worktree = "D:\Project\FortRogue-editor"; RoleFile = ".agents\roles\05-editor.md" }
    "texture"     = @{ Name = "Texture"; Worktree = "D:\Project\FortRogue-texture"; RoleFile = ".agents\roles\06-texture.md" }
    "텍스쳐"      = @{ Name = "Texture"; Worktree = "D:\Project\FortRogue-texture"; RoleFile = ".agents\roles\06-texture.md" }
    "텍스처"      = @{ Name = "Texture"; Worktree = "D:\Project\FortRogue-texture"; RoleFile = ".agents\roles\06-texture.md" }
    "review"      = @{ Name = "Review"; Worktree = "D:\Project\FortRogue-review"; RoleFile = ".agents\roles\07-review.md" }
    "리뷰"        = @{ Name = "Review"; Worktree = "D:\Project\FortRogue-review"; RoleFile = ".agents\roles\07-review.md" }
}

function Show-Usage {
    Write-Host @"
Usage:
  .\Agent.bat
  agent> @Flow 너 게임 플로우 점검해
  agent> @Flow -5.4 -xhigh 너 게임 플로우 점검해

  .\Agent.bat Flow 너 게임 플로우 점검해
  .\Agent.bat '@Flow' -5.4 -xhigh 너 게임 플로우 점검해
  .\Agent.bat Combat -dry-run 플레이어 기본 공격 구현 점검해

Defaults:
  model:  $DefaultModel
  effort: $DefaultEffort

Roles:
  @Director, @Design, @Flow, @Combat, @UI, @Editor, @Texture, @Review
  Korean aliases: @총괄, @기획, @게임플로우, @전투, @에디터, @텍스쳐, @리뷰

Options:
  -5.5              Uses model gpt-5.5
  -5.4              Uses model gpt-5.4
  -m <model>        Uses an explicit Codex model
  -model <model>    Uses an explicit Codex model
  -low              Uses low reasoning effort
  -medium           Uses medium reasoning effort
  -high             Uses high reasoning effort
  -xhigh            Uses xhigh reasoning effort
  -dry-run          Prints the Codex command without executing it
"@
}

function Invoke-AgentCommand {
    param(
        [string[]] $CommandArgs
    )

    if (-not $CommandArgs -or $CommandArgs.Count -eq 0) {
        Show-Usage
        return 1
    }

    if ($CommandArgs[0] -in @("-h", "--help", "help", "/?")) {
        Show-Usage
        return 0
    }

    $roleToken = $CommandArgs[0].TrimStart("@")
    $roleKey = $roleToken.ToLowerInvariant()
    if (-not $Roles.ContainsKey($roleKey)) {
        throw "Unknown role '$($CommandArgs[0])'. Run .\Agent.bat --help for supported roles."
    }

    $role = $Roles[$roleKey]
    $model = $DefaultModel
    $effort = $DefaultEffort
    $dryRun = $false
    $promptParts = New-Object System.Collections.Generic.List[string]

    for ($i = 1; $i -lt $CommandArgs.Count; $i++) {
        $arg = $CommandArgs[$i]

        switch -Regex ($arg) {
            "^-([0-9]+(\.[0-9]+)?)$" {
                $model = "gpt-$($Matches[1])"
                continue
            }
            "^-m$|^-model$|^--model$" {
                if ($i + 1 -ge $CommandArgs.Count) {
                    throw "$arg requires a model value."
                }
                $i++
                $model = $CommandArgs[$i]
                continue
            }
            "^-m=(.+)$|^-model=(.+)$|^--model=(.+)$" {
                $model = ($arg -replace "^-m=|^-model=|^--model=", "")
                continue
            }
            "^-low$|^--low$" {
                $effort = "low"
                continue
            }
            "^-medium$|^--medium$" {
                $effort = "medium"
                continue
            }
            "^-high$|^--high$" {
                $effort = "high"
                continue
            }
            "^-xhigh$|^--xhigh$" {
                $effort = "xhigh"
                continue
            }
            "^-dry-run$|^--dry-run$" {
                $dryRun = $true
                continue
            }
            default {
                $promptParts.Add($arg)
            }
        }
    }

    if ($promptParts.Count -eq 0) {
        throw "Prompt is empty. Add the task after the role and options."
    }

    $worktree = $role.Worktree
    $roleFile = Join-Path $worktree $role.RoleFile

    if (-not (Test-Path $worktree)) {
        throw "Worktree not found: $worktree"
    }

    if (-not (Test-Path $roleFile)) {
        throw "Role file not found: $roleFile"
    }

    if (-not (Test-Path $OpsRoot)) {
        throw "Ops folder not found: $OpsRoot"
    }

    $userPrompt = ($promptParts -join " ")
    $prompt = @"
AGENTS.md 지침을 반드시 따라라.
역할 파일을 먼저 읽어라: $roleFile
운영 상태 파일을 확인해라: $OpsRoot\STATUS.md

너는 $($role.Name) 담당이다.
현재 worktree와 branch만 소유한다.
다른 branch를 merge/rebase하지 마라. 총괄이 명시적으로 지시한 경우만 예외다.
작업 완료 후 필요한 검증을 실행하고, 변경이 있다면 커밋까지 수행해라.
STATUS.md와 handoff가 필요하면 갱신하고 마지막 상태를 idle, blocked, needs-review 중 하나로 보고해라.

사용자 지시:
$userPrompt
"@

    $codexArgs = @(
        "exec",
        "-C", $worktree,
        "--full-auto",
        "--add-dir", $OpsRoot,
        "-m", $model,
        "-c", "model_reasoning_effort=`"$effort`"",
        $prompt
    )

    Write-Host "Role:    $($role.Name)"
    Write-Host "Model:   $model"
    Write-Host "Effort:  $effort"
    Write-Host "Root:    $worktree"
    Write-Host ""

    if ($dryRun) {
        Write-Host "Dry run. Command:"
        Write-Host "codex $($codexArgs -join ' ')"
        return 0
    }

    & codex @codexArgs
    return $LASTEXITCODE
}

if (-not $RawLine -and $env:CODEX_AGENT_RAW) {
    $RawLine = $env:CODEX_AGENT_RAW
}

if ($RawLine) {
    $InputArgs = $RawLine -split "\s+"
}

if ($InputArgs -and $InputArgs.Count -gt 0) {
    $exitCode = Invoke-AgentCommand -CommandArgs $InputArgs
    exit $exitCode
}

Show-Usage
Write-Host ""
Write-Host "Interactive mode. Type 'exit' to quit."

while ($true) {
    $line = Read-Host "agent"
    if (-not $line) {
        continue
    }

    if ($line -in @("exit", "quit")) {
        exit 0
    }

    try {
        $lineArgs = $line -split "\s+"
        $code = Invoke-AgentCommand -CommandArgs $lineArgs
        if ($code -ne 0) {
            Write-Host "Command exited with code $code"
        }
    }
    catch {
        Write-Host "Error: $($_.Exception.Message)"
    }
}
