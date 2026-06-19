# FortRogue GAS / Shot Modifier Data Guide

이 문서는 ShellShock Live식 다양한 탄과 보상을 데이터로 만들기 위한 현재 구조를 설명한다.

핵심 방향:

> 포탄 물리와 지형 처리는 C++가 맡고, 무기/보상/퍽/아이템의 효과 변화는 데이터와 GAS AbilitySet으로 붙인다.

## 1. 기본 흐름

발사 직전 `AFortRogueBattleCharacter::BuildShotSpec()`이 `FFortRogueShotSpec`을 만든다.

`ShotSpec`에는 실제 발사에 쓰이는 최종 값이 들어간다.

- `Damage`
- `BlastRadius`
- `TerrainCarveRadius`
- `TerrainFillRadius`
- `LaunchSpeed`
- `Gravity`
- `ProjectileCount`
- `EffectTags`
- `ImpactSpawns`

HUD는 `GetCurrentShotSpec()` 또는 `GetCurrentShotSummary()`를 통해 modifier가 적용된 최종 값과 충돌 후 생성될 자식 탄 수를 표시한다.
UMG 전투 HUD에서는 `UFortRogueBattleHUDWidget::GetPlayerCurrentShotSpec()`, `GetPlayerShotSummary()`, `GetPlayerCombatStatsSummary()`로 같은 정보를 바로 읽을 수 있다.
PlayerController 경유 UI에서는 `GetPlayerCurrentShotSpec()`과 `GetPlayerCurrentShotSummary()`로 현재 선택 무기에 modifier가 반영된 최종 샷을 읽을 수 있다.
조준 HUD는 `UFortRogueBattleHUDWidget::GetPlayerAimAngle()`, `GetPlayerShotPower()`, `GetPlayerShotChargeAlpha()`, `IsPlayerChargingShot()`로 현재 조준과 차지 상태를 읽을 수 있다. PlayerController 경유 UI에서는 같은 목적의 `GetPlayerAimAngle()`, `GetPlayerShotPower()`, `GetPlayerShotChargeAlpha()`, `IsPlayerChargingShot()`를 사용한다.
현재 샷 기준으로 후보 modifier가 적용 가능한지 보려면 `DoesShotModifierMeetCurrentShotConditions()` 또는 PlayerController의 `DoesPlayerShotModifierMeetCurrentShotConditions()`를 사용한다. UMG 전투 HUD에서는 `DoesPlayerShotModifierMeetCurrentShotConditions()`를 사용할 수 있다. 실패 이유는 `GetShotModifierCurrentConditionFailureSummary()`, PlayerController의 `GetPlayerShotModifierCurrentConditionFailureSummary()`, 전투 HUD의 `GetPlayerShotModifierCurrentConditionFailureSummary()`로 표시한다.
발사 버튼 활성화 여부는 캐릭터의 `CanFireSelectedWeapon()` 또는 PlayerController의 `CanFirePlayerWeapon()`으로 확인한다. UMG 전투 HUD에서는 `CanFirePlayerWeapon()`을 사용할 수 있다. 차지 시작 가능 여부만 별도로 봐야 하면 캐릭터의 `CanBeginShotCharge()`, PlayerController나 UMG 전투 HUD의 `CanBeginPlayerShotCharge()`를 사용한다.
바람 UI는 `AFortRogueGameMode::GetWindSummary()`로 현재 바람을 부호 포함 한 줄로 표시할 수 있다. UMG 전투 HUD에서는 `UFortRogueBattleHUDWidget::GetWindSummary()`를 사용할 수 있다.

modifier 적용 순서:

1. 무기 `ShotModifiers`
2. 보상/퍽으로 누적된 `GrantedShotModifiers`
3. 아이템으로 받은 다음 발 전용 `PendingShotModifiers`

`PendingShotModifiers`는 실제 발사 후 초기화된다.

## 2. 무기 데이터

`UFortRogueWeaponDefinition`의 `Weapon` 안에서 다음을 편집한다.

- `DisplayName`, `Description`: 보상/카탈로그 요약에 표시되는 이름과 설명
- `WeaponTag`: 기본 무기 식별 태그
- `ShotEffectTags`: 탄에 붙는 효과 태그
- `ShotModifiers`: 피해, 폭발 반경, 탄 수, 지형 반경 등을 바꾸는 보정 목록
- `ImpactSpawns`: 충돌 시 자식 탄을 생성하는 데이터

기존 무기는 `ShotModifiers`와 `ImpactSpawns`를 비워두면 이전처럼 동작한다.

`ShotModifier`에는 `DisplayName`과 `Description`을 적어 어떤 조건/효과를 의도했는지 남긴다. `DisplayName`은 데이터 에셋 배열 행 제목으로 쓰이고, 두 값은 보상/디버그 요약에 표시된다.
`ShotModifier`를 나중에 제거해야 하는 효과라면 `ModifierTag`에 고유 태그를 넣는다.
`ShotModifier::ImpactSpawns`를 쓰면 무기 자체가 아니라 보상, 퍽, 아이템으로 충돌 후 자식 탄 생성을 추가할 수 있다.

## 3. ShotModifier 사용 예

낮은 각도 강화:

- `bUseAimAngleRange = true`
- `MinAimAngle = 0`
- `MaxAimAngle = 25`
- `DamageMultiplier = 1.4`
- `EffectTags`에 `ShotEffect.Damage`

굴착탄:

- `TerrainCarveRadiusBonus` 또는 `TerrainCarveRadiusMultiplier`를 높인다.
- 피해를 줄이고 싶으면 `DamageMultiplier`를 낮춘다.
- `EffectTags`에 `ShotEffect.Drill`

지형 생성탄:

- `TerrainFillRadiusBonus`를 0보다 크게 설정한다.
- `TerrainFillRadius`가 있으면 projectile은 지형을 파지 않고 `FillCircle()`로 지형을 만든다.
- `EffectTags`에 `ShotEffect.TerrainCreate`

바람 연동탄:

- `bRequireWindAligned = true`
- `MinWindMagnitude`로 최소 풍속을 설정한다.
- `DamageMultiplier`, `LaunchSpeedMultiplier`, `BlastRadiusMultiplier` 중 필요한 값을 조정한다.

조건부 보상:

- `bUseAimAngleRange`는 현재 조준 각도가 범위 안일 때만 modifier를 적용한다.
- `bRequireWindAligned`는 발사 방향과 바람 방향이 같을 때만 modifier를 적용한다.
- `RequiredShotTags`가 있으면 현재 `ShotSpec.EffectTags`에 해당 태그가 하나 이상 있을 때만 적용된다.
- `BlockedShotTags`가 있으면 현재 `ShotSpec.EffectTags`에 해당 태그가 하나라도 있을 때 적용되지 않는다.
- 여러 조건을 같이 켜면 모두 만족해야 적용된다.
- 태그 조건은 modifier 자신의 `EffectTags`를 붙이기 전에 검사한다.
- 데이터 편집 UI에서 modifier 적용 가능 여부와 실패 이유를 미리 보여주려면 `DoesShotModifierMeetShotConditions()`와 `GetShotModifierConditionFailureSummary()`를 사용한다.

## 4. ImpactSpawns 사용 예

충돌 후 분열탄:

- `ImpactSpawns`에 항목을 추가한다.
- `ProjectileCount = 3`
- `SpreadDegrees = 45`
- `LaunchSpeed = 750`
- `DamageMultiplier = 0.5`
- `BlastRadiusMultiplier = 0.5`
- `TerrainCarveRadiusMultiplier = 0.5`
- `ChildEffectTags`에 `ShotEffect.SplitOnImpact`

자식 탄이 지형을 만들게 하려면 `TerrainFillRadius`를 0보다 크게 설정한다. 이 값이 0이면 자식 탄은 `TerrainCarveRadiusMultiplier` 기준으로 지형을 판다.

자식 탄은 기본적으로 다시 분열하지 않는다. 무한 분열을 막기 위해 `ImpactSpawns`를 자식에게 전달하지 않는다.

런 보상이나 다음 발 아이템으로 분열 효과를 주고 싶다면 무기 `ImpactSpawns` 대신 `ShotModifier::ImpactSpawns`에 같은 데이터를 넣는다.

## 5. 보상과 퍽

런 진행 HUD는 `AFortRogueGameMode::GetRunProgressSummary()`로 현재 스테이지와 상태를 한 줄로 표시할 수 있다. UMG 전투 HUD에서는 `GetRunProgressSummary()`, `GetBattleState()`, `GetStatusText()`로 런 진행, 상태 enum, 상태 문구를 읽을 수 있다. PlayerController 경유 UI에서는 `GetCurrentBattleState()`, `GetCurrentStatusText()`를 사용한다.

보상 카드에서 바로 탄 방식을 바꾸려면 `FFortRogueRewardChoice::ShotModifiers`를 사용한다.

직접 스탯 보상은 `DamageBonus`, `MaxHealthBonus`, `MaxMoveBudgetBonus`, `ProjectileBonus`, `ShotPowerMultiplierBonus`를 사용한다. 리스크/보상 카드처럼 음수 값을 넣으면 실제 어트리뷰트도 낮아지고, 요약에도 손해가 부호와 함께 표시된다. 최종 값은 MaxHealth 1, MaxMoveBudget 0, Damage 0, ShotPowerMultiplier 0, ProjectileCount 1 하한에서 멈춘다.
현재 누적된 전투 스탯은 캐릭터 Blueprint getter인 `GetDamageBonus()`, `GetMaxMoveBudget()`, `GetShotPowerMultiplier()`, `GetProjectileCount()`로 UI나 디버그 화면에서 확인할 수 있다. 태그 기반 UI에서는 `Attribute.Health`, `Attribute.MaxHealth`, `Attribute.MoveBudget`, `Attribute.MaxMoveBudget`, `Attribute.Damage`, `Attribute.ShotPowerMultiplier`, `Attribute.ProjectileCount`와 `TryGetCombatAttributeValueByTag()`를 사용한다. PlayerController 경유 UI와 UMG 전투 HUD에서는 `TryGetPlayerCombatAttributeValueByTag()`를 사용한다. 같은 태그로 임시 버프, 디버그 조정, 스크립트 보정을 넣을 때는 `TryApplyCombatAttributeDeltaByTag()` 또는 PlayerController의 `TryApplyPlayerCombatAttributeDeltaByTag()`를 사용한다. 한 줄 요약이 필요하면 `GetCombatStatsSummary()`를 사용한다.

`WeaponReward`로 추가된 무기는 전투 중 1-5번 슬롯으로 선택할 수 있다.
무기 목록 UI가 슬롯 번호로 선택한다면 `AFortRoguePlayerController::SelectPlayerWeaponByIndex()`를 사용한다.
무기 목록 UI가 슬롯 번호 대신 데이터 태그를 기준으로 선택해야 한다면 `AFortRogueBattleCharacter::SelectWeaponByTag()` 또는 `AFortRoguePlayerController::SelectPlayerWeaponByTag()`를 사용한다.
무기 버튼의 활성화 여부나 태그 기반 슬롯 위치를 확인할 때는 `CanSelectWeapon()`, `CanSelectWeaponByTag()`, `GetWeaponIndexByTag()`를 사용한다. PlayerController 경유 UI에서는 `CanSelectPlayerWeapon()`, `CanSelectPlayerWeaponByTag()`, `GetPlayerWeaponIndexByTag()`를 사용한다. UMG 전투 HUD에서도 같은 이름의 `CanSelectPlayerWeapon()`, `CanSelectPlayerWeaponByTag()`, `GetPlayerWeaponIndexByTag()`를 사용할 수 있다.
UMG 전투 HUD에서는 `UFortRogueBattleHUDWidget::GetPlayerWeaponLoadout()`, `GetPlayerCurrentWeaponSpec()`, `GetPlayerSelectedWeaponIndex()`로 현재 무기 목록과 선택 상태를 읽을 수 있다.
현재 무기 목록과 선택 상태를 PlayerController 경유 UI에서 읽어야 한다면 `GetPlayerWeaponLoadout()`, `GetPlayerCurrentWeaponSpec()`, `GetPlayerSelectedWeaponIndex()`를 사용한다.

런 보상 풀은 `UFortRogueStageRunDefinition`에서 설정한다.

- `RewardPool`: 등장 가능한 보상 전체 목록
- `RewardChoiceCount`: 보상 화면에 보여줄 선택지 수, 현재 키보드/Canvas HUD 기준 1-5개
- `RewardWeight`: 각 보상의 등장 가중치
- `bOfferOncePerRun`: `RewardTag`가 있는 보상을 런당 한 번만 제시
- `RequiredRewardTags`: 해당 태그 보상을 이미 선택했을 때만 등장
- `BlockedRewardTags`: 해당 태그 보상을 이미 선택했다면 등장하지 않음

보상 선택지는 같은 보상이 한 화면에 중복되지 않도록 뽑힌다.
`bOfferOncePerRun` 보상이 모두 소진되면 진행이 막히지 않도록 다시 반복 제시를 허용한다.
조건 태그는 fallback 때도 유지된다. 즉 선행 조건이 없는 보상이 다 소진되어도 `RequiredRewardTags`를 만족하지 않은 보상은 억지로 등장하지 않는다.
UI나 디버그 화면에서 현재 런의 선택 보상 태그를 보여줄 때는 `AFortRogueGameMode::GetChosenRewardTags()`를 사용한다.
개별 보상이 현재 선택 태그 조건을 만족하는지 UI에서 미리 확인하려면 `FFortRogueRewardChoice::MeetsRewardTagConditions()` 또는 `UFortRogueRewardBlueprintLibrary::DoesRewardMeetTagConditions()`를 사용한다.
잠긴 이유를 표시하려면 `GetRewardTagConditionFailureSummary()`를 사용한다. 조건을 만족하면 빈 텍스트를 반환한다.

최종 스테이지 전까지 적을 처치하면 `Reward` 상태로 들어가고, 플레이어가 보상을 선택한 뒤 다음 스테이지를 생성한다.
보상 버튼을 활성화할 때는 `AFortRogueGameMode::CanApplyRewardChoice()`로 현재 상태, 선택지 인덱스, 보상 태그 조건 유효성을 확인한다.
보상 슬롯 수와 개별 데이터는 `GetRewardChoiceCount()`와 `GetRewardChoice()`로 읽을 수 있다.
보상 조건 실패 이유를 인덱스 기준으로 표시해야 한다면 `AFortRogueGameMode::GetRewardChoiceConditionFailureSummary()`를 사용한다.
UMG 보상 화면에서는 `UFortRogueRewardScreenWidget::GetRewardChoices()`, `GetRewardChoiceCount()`, `GetRewardChoice()`, `GetRewardChoiceSummary()`, `GetRewardChoiceConditionFailureSummary()`, `GetChosenRewardTags()`, `CanChooseReward()`를 사용한다.
UMG 보상 버튼은 `UFortRogueRewardScreenWidget::ChooseReward()`에 선택 인덱스를 넘기면 GameMode의 보상 적용과 다음 스테이지 진행까지 처리한다.
PlayerController 경유 UI에서는 `GetCurrentRewardChoices()`, `GetCurrentRewardChoiceCount()`, `GetCurrentRewardChoice()`, `GetCurrentRewardChoiceSummary()`, `GetCurrentRewardChoiceConditionFailureSummary()`, `GetChosenRewardTags()`, `CanChooseReward()`, `ChooseRewardByIndex()`를 사용한다.

보상 화면은 `RewardChoice.GetEffectSummary()`를 통해 무기, 아이템, 퍽, ShotModifier, AbilitySet 효과를 자동 요약할 수 있다. UMG에서는 `UFortRogueRewardBlueprintLibrary::GetRewardEffectSummary()`를 사용한다.
보상 데이터를 검수할 때는 `RewardChoice.GetDataValidationSummary()` 또는 UMG의 `UFortRogueRewardBlueprintLibrary::GetRewardDataValidationSummary()`를 사용한다. 표시 이름 누락, 0 이하 가중치, `bOfferOncePerRun`인데 `RewardTag`가 없는 경우, 실제 gameplay effect가 없는 경우, Required/Blocked 보상 태그가 겹치는 경우를 한 줄 경고로 보여준다.
GameMode의 현재 보상 슬롯을 바로 표시해야 한다면 `AFortRogueGameMode::GetRewardChoiceSummary()`를 사용한다. 잘못된 인덱스는 빈 텍스트를 반환한다.
보상 자체의 `DisplayName`과 `Description`도 요약에 포함되므로, 직접 스탯 보상이나 커스텀 카드의 의도를 데이터에 남긴다.
무기/아이템/퍽 카탈로그나 인벤토리 UI에서는 같은 라이브러리의 `GetWeaponEffectSummary()`, `GetItemEffectSummary()`, `GetPerkEffectSummary()`로 개별 데이터 에셋의 효과 요약을 얻는다.
요약은 설정된 `WeaponTag`, `ItemTag`, `PerkTag`, `RewardTag`도 함께 표시해 데이터 검수 중 식별자를 바로 확인할 수 있게 한다.
무기 요약은 기본 피해, 폭발 반경, 2발 이상인 경우 기본 탄 수를 포함한다.
ShotModifier 배열만 따로 표시해야 하는 디버그/편집 UI에서는 `GetShotModifierEffectSummary()`를 사용한다. 요약에는 `ModifierTag`와 `EffectTags`도 포함되어 제거 가능한 효과와 실제 탄 효과 태그를 바로 확인할 수 있다.

퍽 에셋에서 런 전체의 탄 방식을 바꾸려면 `UFortRoguePerkDefinition::ShotModifiers`를 사용한다.

퍽도 `DamageBonus`, `MaxHealthBonus`, `MaxMoveBudgetBonus`, `ProjectileBonus`, `ShotPowerMultiplierBonus`로 기본 전투 어트리뷰트를 올리거나 낮출 수 있다.

둘 다 캐릭터의 `GrantedShotModifiers`에 누적되고, 이후 모든 발사에서 `BuildShotSpec()`에 반영된다.

임시 버프나 해제 가능한 효과는 `AFortRogueBattleCharacter::RemoveGrantedShotModifiersByTag()`로 제거할 수 있다.
이 함수는 먼저 `ModifierTag`를 보고, 기존 데이터 호환을 위해 `EffectTags`에 같은 태그가 있는 modifier도 제거한다.
UI나 블루프린트에서 현재 적용 여부를 확인할 때는 `GetGrantedShotModifierCountByTag()` 또는 `HasGrantedShotModifierByTag()`를 사용한다. PlayerController 경유 UI에서는 `GrantPlayerShotModifiers()`, `GetPlayerGrantedShotModifierCountByTag()`, `HasPlayerGrantedShotModifierByTag()`, `RemovePlayerGrantedShotModifiersByTag()`를 사용한다. UMG 전투 HUD에서는 `GetPlayerGrantedShotModifierCountByTag()`, `HasPlayerGrantedShotModifierByTag()`를 사용할 수 있다.
현재 누적된 전체 modifier 목록과 요약을 표시하려면 `GetGrantedShotModifiersForBlueprint()`, `GetGrantedShotModifiersSummary()`를 사용한다. PlayerController 경유 UI에서는 `GetPlayerGrantedShotModifiers()`, `GetPlayerGrantedShotModifiersSummary()`를 사용한다.
UMG 전투 HUD에서는 `UFortRogueBattleHUDWidget::GetPlayerGrantedShotModifiers()`, `GetPlayerGrantedShotModifiersSummary()`로 현재 런 modifier 목록과 요약을 읽을 수 있다.

## 6. 아이템과 AbilitySet

아이템은 기존 enum 방식도 유지한다.

- `AttackMultiplier`
- `Heal`
- `AbilitySet`

새 데이터 기반 아이템은 `ItemType = AbilitySet`으로 두고 `UseAbilitySet`을 지정한다.
아이템과 퍽 에셋에는 `DisplayName`과 `Description`을 적어둔다. 설명은 보상/카탈로그 요약에 포함되어 데이터 편집 중 의도를 바로 확인할 수 있다.
AbilitySet 에셋에는 `DisplayName`과 `Description`을 적어둔다. 보상 카드 요약은 아이템, 퍽, 직접 보상의 AbilitySet 이름을 표시하므로 데이터 편집자가 어떤 특수효과가 붙는지 바로 확인할 수 있다.
중첩/제거 가능한 AbilitySet이라면 `AbilitySetTag`에 고유 태그를 지정한다.
AbilitySet 자체를 UI에 표시할 때는 `UFortRogueAbilitySet::GetEffectSummary()` 또는 `UFortRogueRewardBlueprintLibrary::GetAbilitySetEffectSummary()`를 사용한다. 요약은 표시 이름, 설명, 유효한 Ability/Effect/AttributeSet 개수를 포함한다.
AbilitySet 데이터를 검수할 때는 `UFortRogueAbilitySet::GetDataValidationSummary()` 또는 `UFortRogueRewardBlueprintLibrary::GetAbilitySetDataValidationSummary()`를 사용한다. 표시 이름 누락, 아무 grant가 없는 에셋, 비어 있는 Ability/Effect/AttributeSet 항목, 0 이하 레벨을 한 줄 경고로 보여준다.
임시 효과나 중첩 효과를 확인할 때는 `AFortRogueBattleCharacter::GetGrantedAbilitySetCount()`로 특정 AbilitySet이 현재 몇 번 부여됐는지 읽을 수 있다. 제거는 기존 `RemoveAbilitySet()`으로 한 항목씩 처리한다.
태그 기반으로 관리하려면 `GetGrantedAbilitySetCountByTag()`, `HasGrantedAbilitySetByTag()`, `RemoveAbilitySetsByTag()`를 사용한다.
현재 부여된 AbilitySet 목록과 요약을 표시하려면 `GetGrantedAbilitySetsForBlueprint()`, `GetGrantedAbilitySetsSummary()`를 사용한다.
PlayerController 경유 UI에서는 `GrantPlayerAbilitySet()`, `RemovePlayerAbilitySet()`, `GetPlayerGrantedAbilitySetCount()`, `GetPlayerGrantedAbilitySetCountByTag()`, `HasPlayerGrantedAbilitySetByTag()`, `GetPlayerGrantedAbilitySets()`, `GetPlayerGrantedAbilitySetsSummary()`, `RemovePlayerAbilitySetsByTag()`를 사용한다.
UMG 전투 HUD에서는 `UFortRogueBattleHUDWidget::GetPlayerGrantedAbilitySets()`, `GetPlayerGrantedAbilitySetsSummary()`, `GetPlayerGrantedAbilitySetCount()`, `GetPlayerGrantedAbilitySetCountByTag()`, `HasPlayerGrantedAbilitySetByTag()`로 현재 부여된 AbilitySet 목록과 태그 상태를 읽을 수 있다.

UI나 블루프린트에서는 `AFortRoguePlayerController::UsePlayerItemByTag()`로 `ItemTag` 기반 아이템 사용을 연결할 수 있다.
현재 표시 중인 아이템 배열을 그대로 버튼에 연결하려면 `AFortRoguePlayerController::UsePlayerItemByIndex()` 또는 `AFortRogueBattleCharacter::UseItemByIndex()`를 사용한다. 인덱스는 `ItemLoadout` 기준 0부터 시작한다.
아이템 버튼을 누르기 전에 활성화 가능 여부를 확인하려면 `CanUseItemByType()`, `CanUseItemByTag()`, `CanUseItemByIndex()`를 사용한다. PlayerController 경유 UI에서는 `CanUsePlayerItemByType()`, `CanUsePlayerItemByTag()`, `CanUsePlayerItemByIndex()`를 사용한다. UMG 전투 HUD에서도 같은 이름의 `CanUsePlayerItemByType()`, `CanUsePlayerItemByTag()`, `CanUsePlayerItemByIndex()`를 사용할 수 있다.
태그 기반 아이템 버튼이 실제 어느 슬롯에 있는지 확인하려면 `GetItemIndexByTag()` 또는 `GetPlayerItemIndexByTag()`를 사용한다. UMG 전투 HUD에서도 `GetPlayerItemIndexByTag()`를 사용할 수 있다.
아이템 목록 UI는 `AFortRogueBattleCharacter::GetItemLoadoutForBlueprint()`로 현재 아이템과 수량을 읽을 수 있다. 단축키 배지처럼 타입/태그별 총 수량만 필요하면 `GetItemCharges()` 또는 `GetItemChargesByTag()`를 사용한다.
UMG 전투 HUD에서는 `UFortRogueBattleHUDWidget::GetPlayerItemLoadout()`, `GetPlayerItemCharges()`, `GetPlayerItemChargesByTag()`으로 현재 아이템 배열과 수량을 읽을 수 있다.
PlayerController 경유 UI에서는 `GetPlayerItemLoadout()`, `GetPlayerItemCharges()`, `GetPlayerItemChargesByTag()`으로 현재 아이템 배열과 수량을 읽을 수 있다.
Canvas HUD도 현재 `ItemLoadout`의 아이템 이름과 수량을 표시한다. 기존 `AttackMultiplier`와 `Heal` 아이템은 각각 `J`, `H` 입력 힌트를 붙인다.
아이템 효과 요약은 `InitialCharges`, `HealAmount`, `AttackMultiplier` 같은 기본 아이템 효과도 표시한다. 보상에서 `RepairCharges`를 override로 지정하면 override 수량을 우선 표시한다.

다음 발 강화 아이템:

- `UseShotModifiers`에 다음 발에만 적용할 modifier를 넣는다.
- 발사 전 HUD의 최종 ShotSpec 표시로 적용 결과를 확인한다.
- 실제 발사 후 `PendingShotModifiers`는 자동으로 비워진다.
- 다음 발 효과를 UI에서 직접 붙이거나 확인하거나 취소해야 한다면 `GrantPendingShotModifiers()`, `GetPendingShotModifierCountByTag()`, `HasPendingShotModifierByTag()`, `RemovePendingShotModifiersByTag()`를 사용한다. PlayerController 경유 UI에서는 `GrantPlayerPendingShotModifiers()`, `GetPlayerPendingShotModifierCountByTag()`, `HasPlayerPendingShotModifierByTag()`, `RemovePlayerPendingShotModifiersByTag()`를 사용한다. UMG 전투 HUD에서는 `GetPlayerPendingShotModifierCountByTag()`, `HasPlayerPendingShotModifierByTag()`를 사용할 수 있다.
- 다음 발에 쌓인 modifier 목록과 요약은 `GetPendingShotModifiersForBlueprint()`, `GetPendingShotModifiersSummary()`로 읽는다. PlayerController 경유 UI에서는 `GetPlayerPendingShotModifiers()`, `GetPlayerPendingShotModifiersSummary()`를 사용한다.
- UMG 전투 HUD에서는 `UFortRogueBattleHUDWidget::GetPlayerPendingShotModifiers()`, `GetPlayerPendingShotModifiersSummary()`로 다음 발 modifier 목록과 요약을 읽을 수 있다.

## 7. 주의할 점

- 단순 스탯 보상만 늘리지 말고, 가능하면 `ShotModifiers`, `ImpactSpawns`, `AbilitySet`으로 플레이 방식이 바뀌게 만든다.
- 포탄 물리, 충돌, 지형 마스크 수정은 아직 C++ 경로가 기준이다.
- `ShotEffect.*` 태그는 효과를 식별하는 공통 어휘다. 실제 동작은 `ShotSpec`, `Projectile`, `Terrain` 코드가 처리한다.
- 새 modifier를 추가하면 HUD의 최종 ShotSpec 표시로 값이 바뀌는지 먼저 확인한다.
- 최종 `ShotSpec`의 피해, 폭발/지형 반경, 발사 속도, 중력은 0 미만으로 내려가지 않게 정규화된다. 음수 보정은 저주/약화 표현에 쓸 수 있지만, 최종 피해가 음수가 되어 회복처럼 동작하지는 않는다.
