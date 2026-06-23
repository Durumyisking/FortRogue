@echo off
setlocal
set "CODEX_AGENT_RAW=%*"
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Scripts\Invoke-CodexAgent.ps1"
exit /b %ERRORLEVEL%
