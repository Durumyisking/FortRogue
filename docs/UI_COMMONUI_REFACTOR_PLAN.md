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
- `DefaultEngine.ini` now uses `/Script/CommonUI.CommonGameViewportClient` so CommonUI input routing can run through the expected viewport client.
- The battle HUD no longer builds a C++ fallback widget tree; `UFRBattleHUDWidget` now expects authored UMG modules and only owns ViewModel creation, injection, and refresh.
- `AFRPlayerController` now creates one UI root and routes battle HUD/reward screens through CommonUI layer stacks when the authored root WBP is based on `UFRUIRootWidget`.
- `AFRPlayerController` can route main menu, options, pause, and confirmation dialogs through the root `MenuLayer` and `ModalLayer`; the authored WBPs now use their CommonUI adapter base classes.
- Character health bars and floating combat text use authored UMG assets; their C++ classes only find named widgets and push runtime values.
- Character health bars now expose a runtime `UFRCharacterHealthBarViewModel` and optional CommonTextBlock/CommonNumericTextBlock display fields.
- World status markers now have a `UFRWorldStatusMarkerWidget` CommonUI adapter with a runtime `UFRWorldStatusMarkerViewModel`, optional `CommonTextBlock`, and optional `CommonBorder`.
- Trajectory preview points now have a `UFRTrajectoryPreviewPointWidget` CommonUI adapter with a runtime `UFRTrajectoryPreviewPointViewModel`, optional `CommonBorder`, `CommonTextBlock`, and `CommonNumericTextBlock`.
- `WBP_WorldStatusMarker` and `WBP_TrajectoryPreviewPoint` now exist as authored UMG component assets under `Content/FortRogue/Widget/MainGame/Components`.
- The editor module exposes `FRGenerateCommonUIWidgets` plus a Tools menu action for regenerating missing world marker and trajectory preview CommonUI component WBPs.
- Floating combat text now expects a `CommonTextBlock` named `DamageText` and can apply an editor-selected `CommonTextStyle`.
- `Content/FortRogue/Widget` now has CommonUI root, menu, HUD, world health, floating text, style, and component assets.
- `AFRPlayerController`, `AFRBattleCharacter`, and `AFRFloatingCombatText` now default to the authored HUD/world/floating WBP assets while keeping editable class overrides.
- `WBP_BattleHUD` no longer carries the broken parent-level prototype MVVM Blueprint extension; `UFRBattleHUDWidget` owns runtime ViewModel creation, update, and module injection.
- Production `WBP_BattleHUD` now includes `EnemyStatusPanel`, and `UFRBattleHUDWidget` owns a separate runtime enemy/target status ViewModel for it.
- Battle HUD module adapter widget classes now exist so module WBPs can own their display updates without parent-to-child internal bindings.
- Battle HUD module WBPs are now parented to their CommonUI adapter classes, and the adapters support the existing authored child widget names as compatibility aliases.
- Battle HUD module WBPs now carry module-owned MVVM ViewModel contexts so future Blueprint bindings can bind at the module level instead of reaching into nested widgets from the parent HUD.
- Generated CommonUI HUD/button WBPs have had empty prototype MVVM Blueprint extensions removed so they no longer produce `/Script/ModelViewViewModelBlueprint` import warnings on commandlet load.
- Loadout weapon/item slots now have per-slot ViewModels and a CommonButton-based slot adapter for selected, enabled, empty, and locked states.
- Reward screen choices now have per-choice ViewModels and a CommonButton-based choice adapter for title, summary, condition feedback, enabled state, and selection.
- Main menu, options, pause, and confirmation dialog now have CommonActivatableWidget adapter classes with CommonButton request events.
- `WBP_UIRoot` is based on `UFRUIRootWidget` and owns CommonUI stacks named `HUDLayer`, `MenuLayer`, and `ModalLayer`.
- `WBP_MainMenu`, `WBP_OptionsMenu`, `WBP_PauseMenu`, and `WBP_ConfirmDialog` are based on their CommonUI adapter classes and have minimal authored CommonUI layouts.
- `WBP_RewardScreen` and `WBP_RewardChoiceButton` now exist under `Content/FortRogue/Widget/Reward`, are based on their CommonUI adapter classes, and carry matching MVVM ViewModel contexts.
- Options menu now has a runtime `UFROptionsMenuViewModel` and optional CommonTextBlock/CommonNumericTextBlock display fields for editor-authored option rows.
- UI root now has a runtime `UFRUIRootViewModel` that exposes active HUD, menu, and modal layer state.
- Main menu, pause menu, and confirmation dialog now share a runtime `UFRMenuScreenViewModel` for title/body/status display.
- Menu adapters expose `FFRMenuStyleSet` so authored screens can apply editor-selected CommonTextStyle and CommonButtonStyle classes to their optional CommonUI children.
- HUD module adapters expose `FFRHUDModuleStyleSet` so authored modules can apply editor-selected CommonTextStyle, numeric text style, and button style classes.
- Reward screen adapters expose `FFRRewardStyleSet` so authored reward screens and choice cards can apply editor-selected CommonTextStyle and CommonButtonStyle classes.

## Key Problems

### Architecture

- UI creation is split between C++ generated widgets and UMG assets, so designers cannot reliably edit the final UI in the editor.
- CommonUI root routing exists in C++ and the authored `WBP_UIRoot` now has the expected root base class and named layer stacks.
- Menu, prompt, and modal flows are routed through the root layer model, but still need interaction and navigation polish in authored assets.
- UI refresh is polling-based instead of state/event driven.

### MVVM

- `VM_BattleHUD` is a flat prototype bag of fields: text and percent values only.
- It does not model the game domains: battle state, player status, enemy status, loadout, shot preview, rewards, or menu state.
- It is not fed by runtime gameplay state and should not become the production path.
- `UFRBattleHUDViewModel` is the current runtime-fed ViewModel surface for the production HUD.
- The current prototype WBP has MVVM bindings, but production bindings should move into module widgets because the C++ fallback path has been removed.
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
- CommonUI viewport routing is enabled through `GameViewportClientClassName=/Script/CommonUI.CommonGameViewportClient`.
- `UFRCharacterHealthBarWidget` and `UFRFloatingCombatTextWidget` require named widgets from authored WBP assets and no longer construct fallback widget trees.
- `UFRCharacterHealthBarWidget` updates `UFRCharacterHealthBarViewModel` and can drive optional `HealthText`, `CurrentHealthValueText`, and `MaxHealthValueText` CommonUI widgets.
- `AFRBattleCharacter` now owns a `StatusMarkerComponent` that points to `/Game/FortRogue/Widget/MainGame/Components/WBP_WorldStatusMarker` when that authored WBP exists, then falls back to the C++ adapter class.
- `AFRBattleCharacter` resolves the default world health, status marker, and trajectory preview WBP classes during `BeginPlay` instead of constructor-loading them, so MVVM Blueprint extensions are not loaded before the editor MVVM module is ready.
- `UFRWorldStatusMarkerWidget` can drive optional `MarkerText` and `MarkerBorder` CommonUI widgets and exposes CommonTextStyle/CommonBorderStyle overrides.
- `AFRBattleCharacter` now samples trajectory preview points into screen-space WidgetComponents using `/Game/FortRogue/Widget/MainGame/Components/WBP_TrajectoryPreviewPoint` when that authored WBP exists instead of drawing debug lines and spheres.
- `UFRTrajectoryPreviewPointWidget` can drive optional `PointText`, `PointIndexText`, and `PointBorder` CommonUI widgets and exposes text, number, trajectory border, and impact border style overrides.
- `FRGenerateCommonUIWidgets` creates `WBP_WorldStatusMarker` and `WBP_TrajectoryPreviewPoint` with CommonUI primitives and the current shared text/border styles when either asset is missing.
- `UFRFloatingCombatTextWidget` uses `UCommonTextBlock` for authored damage text and exposes a `DamageTextStyle` override.
- `UFRBattleHUDWidget` no longer constructs a fallback HUD layout in C++; missing authored HUD modules will surface as missing UI instead of silently showing generated panels.
- `UFRBattleHUDWidget` now injects module-specific runtime ViewModels into known child module widgets so modules can own their own MVVM bindings.
- `EnemyStatusPanel` and `TargetStatusPanel` receive the runtime enemy status ViewModel, and the combatant status panel hides movement budget fields when no move budget text is present.
- `UFRBattleHUDModuleWidgetBase` and derived adapter widgets can receive injected module ViewModels and push values into optional named CommonUI widgets.
- `UFRBattleHUDModuleWidgetBase` and `UFRLoadoutSlotWidget` now forward injected ViewModels into their own MVVM view extension when the authored WBP has a matching manual ViewModel context.
- `WBP_TurnBanner`, `WBP_CombatantStatusPanel`, `WBP_AimWindIndicator`, `WBP_ShotPowerMeter`, `WBP_LoadoutBar`, `WBP_WeaponSlot`, `WBP_ItemSlot`, `WBP_ShotInfoPanel`, and `WBP_ModifierSummary` now use the matching Battle HUD adapter parent classes.
- Those Battle HUD module WBPs now have matching manual MVVM ViewModel contexts for `UFRBattleStateViewModel`, `UFRCombatantStatusViewModel`, `UFRAimWindViewModel`, `UFRShotPowerViewModel`, `UFRLoadoutViewModel`, `UFRLoadoutSlotViewModel`, `UFRShotPreviewViewModel`, and `UFRModifierSummaryViewModel`.
- `UFRLoadoutSlotWidget` uses `UCommonButtonBase`; `WBP_LoadoutBar` can expose `WeaponSlotPanel` and `ItemSlotPanel` containing slot widgets in editor-defined counts.
- `UFRRewardScreenWidget` creates a runtime `UFRRewardScreenViewModel`; `UFRRewardChoiceButtonWidget` uses `UCommonButtonBase` for editor-authored reward choice cards.
- `AFRPlayerController` now lazy-loads `/Game/FortRogue/Widget/Reward/WBP_RewardScreen` as the default reward screen when no editable override is set.
- `UFRUIRootWidget` expects authored CommonUI stacks named `HUDLayer`, `MenuLayer`, and `ModalLayer`; `WBP_UIRoot` now provides those stacks.
- `UFRMainMenuWidget`, `UFROptionsMenuWidget`, `UFRPauseMenuWidget`, and `UFRConfirmDialogWidget` expose optional named CommonButton children and BlueprintAssignable request events.
- `AFRPlayerController` binds menu adapter request events and uses `Escape` as a fallback for pause/back/cancel while Enhanced Input actions are not yet authored.
- `UFROptionsMenuWidget` creates and injects `UFROptionsMenuViewModel`; default option labels and numeric values are editable on the widget class.
- `UFRUIRootWidget` creates and injects `UFRUIRootViewModel`; layer push, remove, and clear operations refresh the active layer state.
- `UFRMainMenuWidget`, `UFRPauseMenuWidget`, and `UFRConfirmDialogWidget` create and inject `UFRMenuScreenViewModel`; optional CommonTextBlock fields can display the same values without Blueprint scripting.
- `FFRMenuStyleSet` lets main menu, options, pause, and confirmation adapters apply title/body/status text styles and primary/secondary button styles in C++.
- `FFRHUDModuleStyleSet` lets battle HUD modules apply CommonTextStyle/CommonNumericTextBlock styling by walking their authored widget trees; loadout slots can also apply a CommonButtonStyle override.
- `FFRRewardStyleSet` lets reward screen titles and reward choice cards apply CommonTextStyle/CommonButtonStyle overrides in C++.
- Next implementation step: decide whether the untracked prototype `WBP_BattleHUD_MVVM` / `VM_BattleHUD` assets should be deleted, regenerated, or kept only as reference, then add formal module-owned Blueprint MVVM bindings only where the C++ adapters are not sufficient.

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
- `WBP_WorldStatusMarker`
- `WBP_TrajectoryPreviewPoint`
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
	- Keep the removed `BuildDefaultHUD` path out of the production HUD.

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
- [x] Expose fallback HUD CommonUI style classes for editor control.
- [x] Expose world health bar presentation colors for editor control.
- [x] Create modular battle HUD widgets.
- [x] Create missing main menu and options screen widgets.
- [x] Convert generated TextBlock/Border widgets to CommonUI primitives.
- [x] Convert C++ fallback HUD TextBlock/Border construction to CommonUI primitives.
- [x] Convert C++ fallback HUD numeric value displays to CommonNumericTextBlock.
- [x] Remove C++ battle HUD fallback layout construction.
- [x] Convert floating combat text authored widget path to CommonTextBlock.
- [x] Add floating combat text CommonTextStyle override.
- [x] Remove C++ world health/floating combat fallback layout construction.
- [x] Convert remaining C++ gameplay widget bases to CommonUserWidget/CommonActivatableWidget.
- [x] Rebuild remaining menu/dialog buttons as CommonButtonBase widgets.
- [x] Identify remaining C++ generated runtime UI entry points.
- [x] Wire runtime HUD, world health, and floating combat text to authored UMG assets.
- [x] Convert world health/floating combat UI to authored UMG assets.
- [x] Add runtime-fed `UFRBattleHUDViewModel` and attach it to `WBP_BattleHUD`.
- [x] Add module/domain ViewModel classes for battle HUD modules.
- [x] Add numeric binding fields for CommonNumericTextBlock-ready battle HUD modules.
- [x] Inject module/domain ViewModels into battle HUD module widgets.
- [x] Add battle HUD module adapter widget classes for injected ViewModels.
- [x] Add CommonButton-based loadout slot ViewModels and adapter widgets.
- [x] Add CommonButton-based reward choice ViewModels and adapter widgets.
- [x] Route Battle HUD and Reward screen through a CommonUI root layer adapter.
- [x] Configure CommonUI GameViewportClient for input routing.
- [x] Add CommonUI adapter classes for menu and confirmation dialog screens.
- [x] Connect menu, options, pause, and confirmation adapters to the CommonUI root layers.
- [x] Add Options menu ViewModel and CommonUI display adapter fields.
- [x] Add UI root layer state ViewModel.
- [x] Add Main/Pause/Confirm menu screen ViewModel display surfaces.
- [x] Add menu CommonUI style override surface.
- [x] Add HUD module CommonUI style override surface.
- [x] Add Reward screen CommonUI style override surface.
- [x] Add world health bar ViewModel and CommonNumericTextBlock display surface.
- [x] Add world status marker CommonUI/MVVM adapter and runtime widget component.
- [x] Add trajectory preview CommonUI/MVVM point adapter and runtime widget components.
- [x] Create authored UMG assets for world status marker and trajectory preview point.
- [x] Add an editor/commandlet generator for missing world marker and trajectory CommonUI component WBPs.
- [x] Defer world UI WBP default loads until runtime so MVVM extension assets are not loaded too early.
- [x] Parent root, menu, and confirmation WBPs to their CommonUI adapter classes.
- [x] Parent Battle HUD module WBPs to adapter classes and support legacy authored child widget names.
- [x] Remove prototype MVVM Blueprint extensions/import warnings from generated CommonUI HUD/button WBPs.
- [x] Add module-owned MVVM ViewModel contexts to Battle HUD module WBPs.
- [x] Connect enemy/target status panel to the production HUD ViewModel path.
- [x] Create authored Reward screen and Reward choice CommonUI WBP assets.
- [ ] Replace prototype MVVM with module/domain ViewModels and real module-level bindings.
- [x] Compile and save created UMG assets.

## Immediate Next Task

Decide the fate of the untracked prototype `WBP_BattleHUD_MVVM` and `VM_BattleHUD` assets: delete them, regenerate them as module-level examples, or keep them only as reference. Production `WBP_BattleHUD` now uses module adapter injection and module-owned MVVM contexts instead of parent-to-child prototype bindings.

Do not bind from the parent HUD into nested widget internals; that path failed compilation and should stay replaced with module-owned bindings or the adapter widgets. The remaining production gap is authoring real module-level Blueprint MVVM bindings where the C++ adapter refresh path is not enough.
