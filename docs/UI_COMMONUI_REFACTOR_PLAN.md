# FortRogue UI CommonUI Refactor Plan

## Assumptions

- FortRogue UI should be authored as editor-editable UMG assets wherever possible.
- CommonUI should own screen routing, modal/menu layering, focus, and input actions.
- C++ should provide gameplay state and ViewModel updates, not construct final HUD layout by hand.

## Reference UI Direction

### Researched References

- ShellShock Live: artillery tank combat with bottom armor/fuel/items/weapons HUD, compact shot flow, and hundreds of weapons.
  - Source: https://store.steampowered.com/app/326460/ShellShock_Live/
- Worms W.M.D: turn-based 2D artillery with in-world health labels, bottom-left turn timer, and a large weapon/crafting grid opened only when needed.
  - Source: https://store.steampowered.com/app/327030/Worms_WMD/
- Fortress 2 BLUE: classic fortress-style angle/power/wind readability with a dense bottom command strip and clear turn countdown.
  - Source: https://store.steampowered.com/app/3152590/Fortress_2_BLUE/

## 

1. UI 기본 방향은 CommonUI + UMG + MVVM이다.

2. 최종 화면 구성과 레이아웃은 WBP가 소유한다.
C++은 최종 HUD나 화면을 직접 조립하지 않고, 상태 제공, 이벤트 중계, meta = (BindWidget) 기반 어댑터 역할만 한다.

3. CommonUI로 구현 가능한 UI는 반드시 CommonUI를 우선 사용한다.
예를 들어 ListView가 필요하면 일반 ListView보다 CommonListView를 우선 검토하고 사용한다. 버튼, 텍스트, 보더도 가능한 경우 CommonButton, CommonTextBlock, CommonBorder 계열을 사용한다.

4. CommonUI 위젯에 사용할 Style 에셋은 반드시 만든다.
단, 기능별로 무분별하게 만들지 않고 디자인/용도 단위로 나눈다.
예: MainMenu 버튼 스타일, HUD 텍스트 스타일, 패널 보더 스타일.

4. 버튼 WBP는 기능별로 만들지 않는다.
GameStartButton, OptionButton, ExitButton처럼 이벤트 이름 기준으로 버튼 에셋을 늘리지 않는다.
MainMenu에서 공용으로 쓰는 버튼은 하나의 MainMenu 버튼 위젯/스타일을 만들고, GameStart, LoadGame, Option, Exit 동작은 MainMenu WBP나 C++ 어댑터에서 클릭 이벤트로 바인딩한다.

5. 한 번만 쓰이는 UI는 별도 공용 WBP로 분리하지 않는다.
예를 들어 발사 버튼처럼 특정 화면에서만 쓰이는 버튼은 MainGame WBP 안에 직접 배치한다. 스타일 적용은 필요에 따라 하되, 단일 사용 위젯을 억지로 공용화하지 않는다.

6. 시각 차이만 있는 위젯은 WBP를 복제하지 않는다.
적 체력바와 아군 체력바처럼 색상만 다른 경우 WBP_HPBar 하나를 만들고, PreConstruct 변수, 스타일, 머티리얼 파라미터 등으로 색상만 바꾼다.

7. MVVM은 반복 사용되는 데이터 표현에 우선 적용한다.
Perk처럼 선택지, 툴팁, 상세 설명 등 여러 곳에서 같은 데이터를 보여주는 경우 FRPerkViewModel 같은 ViewModel을 만든다.
반대로 MainGame의 발사 힘 게이지처럼 특정 화면에서만 쓰이는 단순 상태는 MVVM 필수 대상이 아니다.

8.C++ 어댑터는 WBP 내부 구조를 과하게 소유하지 않는다.
필요한 BindWidget 참조, 상태 갱신 함수, 이벤트 delegate 정도만 제공하고, 레이아웃/스타일/화면 구성은 WBP에서 처리한다.

9. UI 작업은 최소한으로 한다.
   요청받지 않은 공용화, 추상화, 에셋 생성, 기능 확장을 하지 않는다.
   같은 디자인을 재사용해야 할 때만 공용 위젯을 만들고, 단순 차이는 파라미터로 해결한다.

10. 에디터 소유 UI 에셋을 수정할 때는 Unreal MCP ToolSet으로 실제 에디터 상태를 확인하고 작업한다.
   소스 검색이나 빌드 성공만으로 WBP, Style, UMG 작업 완료를 주장하지 않는다.

11. 완료 기준은 실제 WBP/Style/C++ 책임 분리가 확인되는 것이다.
   “CommonUI를 썼다”가 아니라, WBP가 화면을 소유하고 C++이 어댑터로만 남았는지, 스타일 에셋이 디자인 단위로 정리됐는지, 불필요한 WBP 증식이 없는지를 기준으로 검증한다.

12. UI제작은 모듈화와 편집 용이화를 중점으로 둔다.


## Required UI

### Main Flow

- Main menu: start run, options, quit.[]
- Options: audio, display/window mode, resolution or scale, input hints, accessibility basics.[]
- Pause menu: resume, options, restart/return to menu, quit.[]
- Confirmation dialog: destructive actions such as quit/restart.[]

### Battle HUD

- Turn/status banner. []
- Run progress summary. []
- Player status panel. []
- Movement budget indicator. []
- Aim and wind indicator. []
- Shot charge/power indicator. []
- Current weapon summary. []
- Weapon loadout bar. []

### World UI

- Character health bar.[]
- Selected unit/target marker.[]
- Floating combat text.[]
- Projectile impact/trajectory preview indicators.[]

### Reward UI

- Reward screen root.[]
- Reward card list.[]
- Chosen reward/condition feedback.[]
- Continue/confirm action.[]

## UI To Remove Or Demote

- Remove duplicate stage text in the top status bar.
- Demote constant raw shot stats; keep core stats visible and move secondary details into expanded info or tooltip-style panels.
- Demote full side-panel HP if world health bars are visible and readable; side panel can show player HP and target HP only when tactically useful.
- Replace large empty weapon/item blocks with compact disabled/locked slots.
- Remove C++ fallback layout once production UMG assets exist.

## Recommended Widget Modules

### Global

- `WBP_CommonButton`
- `WBP_CommonPanelFrame`
- `WBP_ConfirmDialog`
- `WBP_InputActionHint`

### MainGame
- `WBP_BattleHUD`

### Reward

- `WBP_RewardScreen`
- `WBP_RewardCard`
- `WBP_RewardChoiceList`

### MainMenu

- `WBP_MainMenuHUD`
- `WBP_OptionsMenu`

## Recommended ViewModels

- `VM_CombatantStatus`: name, HP, HP percent, enemy/player/team color, defeated state.
- `VM_ShotPreview`: damage, blast, projectile count, primary effect, secondary stats.
- `VM_RewardScreen`: reward choices, selected index, can choose, failure reason.
