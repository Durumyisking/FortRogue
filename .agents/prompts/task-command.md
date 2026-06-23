# Task Command Prompt

Use this prompt when assigning a task to a worker.

```text
D:\Project\FortRogue-ops\tasks\<role>\<task-file>.md 작업을 처리해.

반드시 AGENTS.md와 <ROLE_FILE>을 따른다.
허용 범위 밖 변경이 필요하면 구현하지 말고 BLOCKED로 기록한다.
완료 후 필요한 검증을 실행하고, 커밋한 뒤 D:\Project\FortRogue-ops\STATUS.md와 handoff를 갱신해라.
마지막에는 idle, blocked, needs-review 중 하나로 상태를 보고해라.
```
