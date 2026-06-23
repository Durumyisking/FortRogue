# Worker Start Prompt

Use this prompt after opening each Codex CLI role session.

```text
AGENTS.md 지침을 반드시 따라라.
역할 파일을 먼저 읽어라: <ROLE_FILE>
운영 상태 파일을 확인해라: D:\Project\FortRogue-ops\STATUS.md

너는 <ROLE_NAME> 담당이다.
현재 worktree와 branch만 소유한다.
다른 branch를 merge/rebase하지 마라. 총괄이 명시적으로 지시한 경우만 예외다.
작업 지시가 없으면 현재 상태를 보고하고 idle 상태로 대기해라.
작업을 완료하면 검증, 커밋, STATUS.md 갱신, handoff 작성 후 idle 상태로 돌아가라.
```
