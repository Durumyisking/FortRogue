# FortRogue UI CommonUI Refactor Plan

## Assumptions

- FortRogue UI should be authored as editor-editable UMG assets wherever possible.
- CommonUI should own screen routing, modal/menu layering, focus, and input actions.
- C++ should provide gameplay state and ViewModel updates, not construct final HUD layout by hand.
- The current `WBP_BattleHUD_MVVM` and `VM_BattleHUD` assets are prototypes and should be replaced or heavily reworked before becoming production UI.

## Reference UI Direction

### Researched References

- ShellShock Live: artillery tank combat with bottom armor/fuel/items/weapons HUD, compact shot flow, and hundreds of weapons.
  - Source: https://store.steampowered.com/app/326460/ShellShock_Live/
- Worms W.M.D: turn-based 2D artillery with in-world health labels, bottom-left turn timer, and a large weapon/crafting grid opened only when needed.
  - Source: https://store.steampowered.com/app/327030/Worms_WMD/
- Fortress 2 BLUE: classic fortress-style angle/power/wind readability with a dense bottom command strip and clear turn countdown.
  - Source: https://store.steampowered.com/app/3152590/Fortress_2_BLUE/

### Applied FortRogue Direction

- Keep the battlefield visually dominant. Permanent HUD should avoid the center of the screen.
- Use a top turn banner for current turn, run progress, and one short instruction line only.
- Put aiming essentials together: wind, angle, power, movement budget, and fire readiness should read as one control cluster.
- Make the bottom bar the main tactical interaction area: current weapon, weapon slots, item slots, and selection state.
- Keep right-side shot detail collapsible or visually secondary. Damage/blast/projectile count are primary; gravity, terrain, fill, traits, and modifiers are secondary.
- Use world-space UMG for unit HP/name/selection markers, not C++ Slate or debug drawing.
- Replace large empty slots with compact disabled, locked, or unavailable states.
- Main menu, options, pause, confirmation, reward, and battle HUD should all enter through the CommonUI root layer model.

## Current State

- `UFRBattleHUDWidget` and `UFRRewardScreenWidget` already inherit from `UCommonActivatableWidget`.
- The active battle HUD is still mostly generated in C++ through `WidgetTree` in `UFRBattleHUDWidget::BuildDefaultHUD`.
- `AFRPlayerController` creates widgets directly with `CreateWidget`, adds them to the viewport, and refreshes the HUD every tick.
- Character health bars and floating combat text are `UUserWidget` classes, but their widget trees are created in C++ rather than from reusable UMG assets.
- `Content/FortRogue/Widget` now has CommonUI root, menu, HUD, world health, floating text, style, and component assets.
- `AFRPlayerController`, `AFRBattleCharacter`, and `AFRFloatingCombatText` now default to the authored HUD/world/floating WBP assets while keeping editable class overrides.
- `WBP_BattleHUD` now has a `UFRBattleHUDViewModel` MVVM context, and `UFRBattleHUDWidget` creates, updates, and injects the same ViewModel instance at runtime.

## Key Problems

### Architecture

- UI creation is split between C++ generated widgets and UMG assets, so designers cannot reliably edit the final UI in the editor.
- CommonUI is present but not really used as the UI flow architecture.
- There is no root UI layout/layer system for HUD, menus, prompts, and modals.
- UI refresh is polling-based instead of state/event driven.

### MVVM

- `VM_BattleHUD` is a flat prototype bag of fields: text and percent values only.
- It does not model the game domains: battle state, player status, enemy status, loadout, shot preview, rewards, or menu state.
- It is not fed by runtime gameplay state and should not become the production path.
- `UFRBattleHUDViewModel` is the current runtime-fed ViewModel surface for the production HUD.
- The current prototype WBP has MVVM bindings, but `UFRBattleHUDWidget::RefreshDefaultHUD` exits early because C++ private widget pointers are not populated by that WBP.
- Direct parent HUD bindings into nested module widget internals, such as `TurnBanner.TurnText.Text`, currently fail WBP compilation. Bindings should move into each module widget or into module-specific C++ adapter widgets instead.

### HUD Layout

- Top status repeats stage information.
- Combat data is scattered across left, right, top, bottom, and world-space UI without a clear priority hierarchy.
- The shot info panel shows too many raw numbers at the same visual weight.
- Empty weapon/item slots occupy too much space and do not clearly communicate locked, empty, or unavailable states.
- Player/enemy HP is duplicated between side HUD panels and world health bars without a clear role split.

### Widgets

- HUD modules are not separate widgets.
- Health bar, floating combat text, weapon slot, item slot, shot info, turn banner, and reward card should be reusable component widgets.
- Current visuals are hard-coded in C++ and cannot be themed consistently.
- Main menu and options screens do not exist.

## Required UI

### Main Flow

- Main menu: start run, options, quit.
- Options: audio, display/window mode, resolution or scale, input hints, accessibility basics.
- Pause menu: resume, options, restart/return to menu, quit.
- Confirmation dialog: destructive actions such as quit/restart.

### Battle HUD

- Turn/status banner.
- Run progress summary.
- Player status panel.
- Enemy/target status panel.
- Movement budget indicator.
- Aim and wind indicator.
- Shot charge/power indicator.
- Current weapon summary.
- Weapon loadout bar.
- Item/consumable bar.
- Shot preview panel.
- Active and pending modifier summary.

### World UI

- Character health bar.
- Selected unit/target marker.
- Floating combat text.
- Projectile impact/trajectory preview indicators.

### Reward UI

- Reward screen root.
- Reward card list.
- Chosen reward/condition feedback.
- Continue/confirm action.

## UI To Remove Or Demote

- Remove duplicate stage text in the top status bar.
- Demote constant raw shot stats; keep core stats visible and move secondary details into expanded info or tooltip-style panels.
- Demote full side-panel HP if world health bars are visible and readable; side panel can show player HP and target HP only when tactically useful.
- Replace large empty weapon/item blocks with compact disabled/locked slots.
- Remove C++ fallback layout once production UMG assets exist.

## Runtime Integration Remaining

- Runtime HUD, world health, and floating combat text now default to authored WBP assets.
- `UFRCharacterHealthBarWidget` and `UFRFloatingCombatTextWidget` now prefer named widgets from authored WBP assets and only construct fallback widget trees if no authored widget exists.
- `UFRBattleHUDWidget::BuildDefaultHUD` still exists as a fallback for missing authored layouts.
- `UFRBattleHUDWidget` now injects module-specific runtime ViewModels into known child module widgets so modules can own their own MVVM bindings.
- Next implementation step: restart the editor/MCP session, then bind and save each module widget directly to the injected ViewModel.

## Recommended Widget Modules

### Global

- `WBP_UIRoot`
- `WBP_CommonButton`
- `WBP_CommonPanelFrame`
- `WBP_ConfirmDialog`
- `WBP_InputActionHint`

### MainGame

- `WBP_BattleHUD`
- `WBP_TurnBanner`
- `WBP_RunProgress`
- `WBP_CombatantStatusPanel`
- `WBP_HealthBar`
- `WBP_WorldHealthBar`
- `WBP_AimWindIndicator`
- `WBP_ShotPowerMeter`
- `WBP_LoadoutBar`
- `WBP_WeaponSlot`
- `WBP_ItemSlot`
- `WBP_ShotInfoPanel`
- `WBP_ModifierSummary`
- `WBP_FloatingCombatText`

### Reward

- `WBP_RewardScreen`
- `WBP_RewardCard`
- `WBP_RewardChoiceList`

### MainMenu

- `WBP_MainMenu`
- `WBP_OptionsMenu`
- `WBP_PauseMenu`

## Recommended ViewModels

- `VM_UIRoot`: active screen, modal state, input mode.
- `VM_BattleHUD`: top-level composition only.
- `VM_BattleState`: turn state, run progress, status message.
- `VM_CombatantStatus`: name, HP, HP percent, enemy/player/team color, defeated state.
- `VM_AimWind`: aim angle, wind direction, wind strength.
- `VM_ShotPower`: charge percent, can fire, charging state.
- `VM_Loadout`: selected weapon index, weapon slot list, item slot list.
- `VM_WeaponSlot`: display name, icon, ammo/count if needed, selected, enabled, unavailable reason.
- `VM_ItemSlot`: display name, icon, charges, enabled, unavailable reason.
- `VM_ShotPreview`: damage, blast, projectile count, primary effect, secondary stats.
- `VM_RewardScreen`: reward choices, selected index, can choose, failure reason.
- `VM_Options`: settings state and pending changes.

## Implementation Order

1. Create CommonUI root architecture.
   - Add a root layout widget with HUD, menu, modal, and prompt layers.
   - Route `AFRPlayerController` through the root instead of direct `AddToViewport` per screen.

2. Replace C++ HUD layout with modular UMG.
   - Keep C++ base classes thin.
   - Move layout to WBP modules.
   - Delete or gate `BuildDefaultHUD` after production assets are assigned.

3. Rebuild battle HUD modules.
   - Build `WBP_BattleHUD` from reusable child widgets.
   - Replace the current `WBP_BattleHUD_MVVM` prototype.
   - Use explicit slots for top banner, combat panels, loadout, and shot details.

4. Rework MVVM data ownership.
   - Replace flat `VM_BattleHUD` with domain-specific VMs.
   - Update VMs from gameplay state changes and explicit refresh events.
   - Avoid binding directly to gameplay actors from widgets.

5. Convert world UI to UMG assets.
   - Create `WBP_WorldHealthBar` and `WBP_FloatingCombatText`.
   - Keep `UWidgetComponent`, but point it to editable WBP classes.
   - Remove C++ widget-tree construction from those widget classes.

6. Build missing screens.
   - Main menu.
   - Options.
   - Pause menu.
   - Reward screen.
   - Confirmation dialog.

7. Polish HUD hierarchy.
   - Remove duplicate text.
   - Reduce always-visible numeric noise.
   - Clarify slot states.
   - Tune spacing and screen-edge placement.

8. Verify.
   - Compile all WBP assets.
   - Run `.\Build-UE58.bat`.
   - Play in editor and capture HUD screenshots at desktop and smaller viewport sizes.
	- Check that all gameplay UI is UMG/CommonUI authored or intentionally world-rendered through UMG widget components.

## Execution Checklist

- [x] Research ShellShock Live, Worms W.M.D, and Fortress-style artillery UI references.
- [x] Apply the reference direction to the FortRogue UI plan.
- [x] Create CommonUI root and layer widgets.
- [x] Create CommonUI style assets and common primitive components.
- [x] Create modular battle HUD widgets.
- [x] Create missing main menu and options screen widgets.
- [x] Convert generated TextBlock/Border widgets to CommonUI primitives.
- [x] Convert C++ fallback HUD TextBlock/Border construction to CommonUI primitives.
- [x] Rebuild remaining menu/dialog buttons as CommonButtonBase widgets.
- [x] Identify remaining C++ generated runtime UI entry points.
- [x] Wire runtime HUD, world health, and floating combat text to authored UMG assets.
- [x] Convert world health/floating combat UI to authored UMG assets.
- [x] Add runtime-fed `UFRBattleHUDViewModel` and attach it to `WBP_BattleHUD`.
- [x] Add module/domain ViewModel classes for battle HUD modules.
- [x] Inject module/domain ViewModels into battle HUD module widgets.
- [ ] Replace prototype MVVM with module/domain ViewModels and real module-level bindings.
- [x] Compile and save created UMG assets.

## Immediate Next Task

Restart the editor/MCP session before editing MVVM bindings again. The last tool session crashed while compiling `WBP_ShotInfoPanel` after Live Coding changed `UFRBattleHUDViewModel` FieldNotify members.

After restart, bind `WBP_TurnBanner` to `UFRBattleStateViewModel`, `WBP_CombatantStatusPanel` to `UFRCombatantStatusViewModel`, `WBP_AimWindIndicator` to `UFRAimWindViewModel`, `WBP_ShotPowerMeter` to `UFRShotPowerViewModel`, `WBP_LoadoutBar` to `UFRLoadoutViewModel`, `WBP_ShotInfoPanel` to `UFRShotPreviewViewModel`, and `WBP_ModifierSummary` to `UFRModifierSummaryViewModel`. Do not bind from the parent HUD into nested widget internals; that path failed compilation and should be replaced with module-owned bindings or thin C++ adapter widgets.
