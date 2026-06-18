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

HUD는 `GetCurrentShotSpec()`을 통해 modifier가 적용된 최종 값을 표시한다.

modifier 적용 순서:

1. 무기 `ShotModifiers`
2. 보상/퍽으로 누적된 `GrantedShotModifiers`
3. 아이템으로 받은 다음 발 전용 `PendingShotModifiers`

`PendingShotModifiers`는 실제 발사 후 초기화된다.

## 2. 무기 데이터

`UFortRogueWeaponDefinition`의 `Weapon` 안에서 다음을 편집한다.

- `WeaponTag`: 기본 무기 식별 태그
- `ShotEffectTags`: 탄에 붙는 효과 태그
- `ShotModifiers`: 피해, 폭발 반경, 탄 수, 지형 반경 등을 바꾸는 보정 목록
- `ImpactSpawns`: 충돌 시 자식 탄을 생성하는 데이터

기존 무기는 `ShotModifiers`와 `ImpactSpawns`를 비워두면 이전처럼 동작한다.

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
- 두 조건을 같이 켜면 둘 다 만족해야 적용된다.

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

자식 탄은 기본적으로 다시 분열하지 않는다. 무한 분열을 막기 위해 `ImpactSpawns`를 자식에게 전달하지 않는다.

## 5. 보상과 퍽

보상 카드에서 바로 탄 방식을 바꾸려면 `FFortRogueRewardChoice::ShotModifiers`를 사용한다.

퍽 에셋에서 런 전체의 탄 방식을 바꾸려면 `UFortRoguePerkDefinition::ShotModifiers`를 사용한다.

둘 다 캐릭터의 `GrantedShotModifiers`에 누적되고, 이후 모든 발사에서 `BuildShotSpec()`에 반영된다.

## 6. 아이템과 AbilitySet

아이템은 기존 enum 방식도 유지한다.

- `AttackMultiplier`
- `Heal`
- `AbilitySet`

새 데이터 기반 아이템은 `ItemType = AbilitySet`으로 두고 `UseAbilitySet`을 지정한다.

UI나 블루프린트에서는 `AFortRoguePlayerController::UsePlayerItemByTag()`로 `ItemTag` 기반 아이템 사용을 연결할 수 있다.

다음 발 강화 아이템:

- `UseShotModifiers`에 다음 발에만 적용할 modifier를 넣는다.
- 발사 전 HUD의 최종 ShotSpec 표시로 적용 결과를 확인한다.
- 실제 발사 후 `PendingShotModifiers`는 자동으로 비워진다.

## 7. 주의할 점

- 단순 스탯 보상만 늘리지 말고, 가능하면 `ShotModifiers`, `ImpactSpawns`, `AbilitySet`으로 플레이 방식이 바뀌게 만든다.
- 포탄 물리, 충돌, 지형 마스크 수정은 아직 C++ 경로가 기준이다.
- `ShotEffect.*` 태그는 효과를 식별하는 공통 어휘다. 실제 동작은 `ShotSpec`, `Projectile`, `Terrain` 코드가 처리한다.
- 새 modifier를 추가하면 HUD의 최종 ShotSpec 표시로 값이 바뀌는지 먼저 확인한다.
