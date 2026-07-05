# FortRogue

UE 5.8 기반 턴제 포격 로그라이크 (ShellShockLive / 포트리스 라이크). 2D 게임플레이 평면(X/Z), 직교 카메라, 파괴 가능 지형.

## 빌드 / 테스트 / 데이터 생성

```powershell
# 빌드
& "D:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" FortRogueEditor Win64 Development -Project="D:\Project\FortRogue\FortRogue.uproject" -WaitMutex

# 자동화 테스트 (헤드리스, 결과 마커: Result={Success} / Result={Fail})
& "D:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "D:\Project\FortRogue\FortRogue.uproject" -ExecCmds="Automation RunTests FortRogue; Quit" -unattended -nopause -nosplash -nullrhi -NoSound -log

# 에디터 실행 (백그라운드로 띄울 것 — 블로킹 금지)
& "D:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "D:\Project\FortRogue\FortRogue.uproject"

# 전투 데이터 에셋 재생성 (데이터 구조 변경 시 필수)
& "D:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "D:\Project\FortRogue\FortRogue.uproject" -run=FRGenerateCombatData -unattended -nullrhi

# 캐릭터 스프라이트/플립북 재생성 (시트 추가·변경 시)
& "D:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "D:\Project\FortRogue\FortRogue.uproject" -run=FRGenerateSpriteFlipbooks -unattended -nullrhi
```

캐릭터 스프라이트 시트 규칙: `Content/GeneratedSprites/<캐릭터>/<애니>_<열>x<행>.png` (셀 512px, 알파 채널). 커맨드릿이 `<캐릭터소문자>_<애니소문자>_flipbook`을 생성하고, `FRGenerateCombatData`가 `LoadAnimationSet()`으로 캐릭터 정의의 `AnimationSet`(Idle/Move/Shoot/Special/Hurt)에 연결한다. 상태 전환은 `Combat/FRCharacterSpriteAnimator`가 담당 (이동/발사/피격 훅은 FRBattleCharacter에서 호출).

## 아키텍처 (2026-07 리팩토링 이후)

책임 분리 원칙: **런 상태는 서브시스템, 전투 연출은 GameMode, 물리·연출 디테일은 컴포넌트/전용 액터.**

- `Game/FRRunSubsystem` — 런 상태의 단일 소유자: 시드 + FRandomStream, 스테이지 진행, 선택 보상 태그, 획득 퍽, 보상 추첨, 적 풀 로테이션. **런 중 무작위 판정은 반드시 이 스트림에서 뽑을 것** (시드 재현성 유지).
- `FRGameMode` — 전투 스테이지 연출과 턴 흐름만. 카메라/런 상태를 직접 들지 않음.
- `Combat/FRBattleCamera` (ACameraActor) — 자동 추적/수동 팬/지형 클램프. GameMode는 `RequestAutoFocus`/`HandleManualInput`만 호출.
- `Combat/FRBattleCharacter` — 전투 캐릭터 파사드. 지형 물리는 `FRTerrainMovementComponent`(델리게이트로 통지), AI 조준은 `FRShotAimSolver`(순수 탐색)에 위임.
- 기본공격/특수공격은 `BasicAttackIndex`/`SpecialAttackIndex`로 명시 관리 — 매직 인덱스 금지.
- 퍽은 획득 시 `AcquiredPerkEntries`에 기록되고 샷 modifier에 출처 퍽이 남음 → `RemovePerkDefinition`으로 되돌리기 가능.

### 데이터 주도 확장 패턴 (새 콘텐츠 추가 시 따를 것)
- 투사체 효과: `UFRProjectileEffectBase` 클래스 + 파라미터 구조체(FInstancedStruct) — ProjectileEffects/
- 보상: `UFRRewardGrant` 인스턴스 객체 (Weapon/Item/Perk 서브클래스) — Rewards/FRRewardGrant.h. 새 보상 종류 = 서브클래스 추가.
- 아이템 즉발 효과: `UFRItemEffect` 인스턴스 객체 — Items/FRItemEffect.h
- 수치 성장/조건부 효과: `FFRShotModifierSpec` (퍽/아이템/보상에 조립)
- 데이터 에셋은 `GetDataValidationSummary()` + `IsDataValid` 오버라이드 쌍을 구현할 것 (에디터 Validate Assets 연동).

### 입력
Enhanced Input 단일 경로. Input Action 에셋이 비어 있으면 `AFRPlayerController::BuildDefaultInputBindings()`가 C++ 기본 매핑(A/D, W/S, Space, 1~5, J/H, 방향키 카메라)을 생성. 키보드 폴링 fallback을 다시 추가하지 말 것.

### GAS 사용 방침
`UFRCombatSet` 어트리뷰트를 직접 setter로 조작하는 경량 방식 유지 (데미지 GE 파이프라인 미사용 — 의도된 결정). 어트리뷰트 태그 매핑은 FRBattleCharacter.cpp의 `GetCombatAttributeHandlers()` 테이블에 한 줄 추가.

## 에디터 MCP (unreal-mcp)

- 에디터 상태를 조회·수정하는 작업(액터/에셋/블루프린트/위젯/레벨/PIE/로그 등)은 **반드시 `unreal-engine-skills-for-claude-code:unreal-mcp` 스킬을 먼저 로드**하고 MCP 툴로 수행할 것. 서버 주소: `.mcp.json`의 `http://127.0.0.1:8000/mcp` (에디터 실행 중일 때만 응답).
- C++ 함수 본문 수정은 `LiveCodingToolset`으로 컴파일. 단 **Live Coding이 못 다루는 변경(UFUNCTION/UPROPERTY 추가·삭제, 헤더 구조 변경, 파일 추가·삭제)** 은 에디터를 종료 → 빌드 → 재실행해야 하며, 이 재시작은 사용자 확인 없이 자율적으로 수행해도 됨 (단, 에디터에 저장 안 된 작업이 있을 수 있으니 종료 전 더티 패키지 저장 시도).

## 주의사항

- 콘텐츠 데이터 에셋(무기/캐릭터/런/모드)은 `FRGenerateCombatData` 커맨드릿이 원본. 에디터에서 손으로 수정하면 재생성 시 덮어써짐.
- 자동화 테스트 중(`GIsAutomationTesting`)에는 GameMode가 GameFlow(메뉴/레벨 이동)를 건너뛰고 바로 전투 시작.
- 테스트 파일 `Tests/FRTerrainAutomationTests.cpp`의 일부 테스트는 AFRGameMode의 friend — GameMode 내부 변경 시 함께 갱신.
- 위젯 기본 클래스는 Project Settings → FortRogue Game Flow → Combat UI (DefaultGame.ini). C++에 에셋 경로 하드코딩 금지.

## 컨벤션

- 커밋 메시지: 한국어, `Feat : ...` / `refactor : ...` 형식
- 클래스 접두사 `FR`, 카테고리 `FortRogue|...`, UPROPERTY에 한국어 ToolTip 필수
- 태그 프로퍼티에는 `meta = (Categories = "...")`로 태그 네임스페이스 제한
