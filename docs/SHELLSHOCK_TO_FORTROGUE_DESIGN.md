# ShellShock Elements Into FortRogue

작성일: 2026-06-20 KST

참고 문서:
- `docs/SHELLSHOCK_LIVE_REFERENCE.md`
- `docs/PRODUCT_VISION.md`
- `docs/GAS_ABILITY_DATA.md`

## 1. 해석

사용자 요청의 의미는 ShellShock Live의 무기 수, 업그레이드, 퍽 감각을 FortRogue에 그대로 복제하는 것이 아니라, FortRogue의 싱글플레이 턴제 지형파괴 로그라이크 구조로 번역하는 것이다.

핵심 원칙:

- ShellShock의 개별 무기명은 참고 자료로만 사용한다.
- FortRogue에는 "무기 계열", "탄 효과", "런 보상", "퍽/패시브", "아이템"으로 재구성한다.
- 같은 효과를 이름만 바꿔 100개 만드는 것보다, 조합 가능한 탄 원형과 런 보상으로 다양성을 만든다.
- 데모 단계에서는 많은 무기보다 서로 다른 판단을 요구하는 무기를 우선한다.

## 2. ShellShock 요소를 FortRogue 시스템으로 번역

| ShellShock 요소 | FortRogue에서의 위치 | 설명 |
| --- | --- | --- |
| 무기 계열 Tier 1-4 | `UFortRogueWeaponDefinition` + 해금/보상 등급 | 같은 탄 원형이 런 중 강화되거나 상위 변형으로 등장한다. |
| 레벨 해금 무기 | 런 보상 풀, 장기 해금 풀 | 플레이어 레벨 대신 런 진행, Act, 난이도, 업적 해금으로 배치한다. |
| Mission/Challenge 무기 | 특수 보상, 조건부 해금, 보스 보상 | 특정 빌드나 플레이 조건을 달성했을 때 풀에 들어오는 보상으로 쓴다. |
| Gear Shop 무기 | 상점/휴식 지점/희귀 보상 | 런 중 제한 자원으로 구매하는 선택지로 둔다. |
| Chassis/Track/Gun/Luck 업그레이드 | 캐릭터 퍽 축 + 런 패시브 | FortRogue에서는 영구 스탯 트리보다 런 안에서 선택하는 빌드 축으로 우선 적용한다. |
| Potion/Health Aura류 | 아이템 또는 회복형 보상 | 다음 턴 생존, 방어막, 회복 오라 등 제한 자원으로 변환한다. |

## 3. FortRogue 무기 계열

아래 계열은 `WeaponDefinition`의 탄 원형이다. 피해량 배율, 조건부 추가 효과, 전역 강화는 무기 자체가 아니라 `ShotModifier`, `PerkDefinition`, `RewardChoice`로 붙인다.

| FortRogue 계열 | 흡수한 ShellShock 패턴 | 기본 역할 | 데이터 구현 |
| --- | --- | --- | --- |
| Standard Shell | Shot, Big Shot, Heavy Shot | 기준 피해와 기준 폭발 반경 | ProjectileEffects 없음 |
| Cluster Shell | Three-Ball, Five-Ball, Guppies, Fleet, Bees 계열 | 넓은 지역 압박, 명중 안정성 | `ProjectileCount` 또는 Split effect |
| Split Shell | Splitter, Breaker, Bounsplode, Bumper 계열 | 첫 충돌 후 자식 탄 생성 | `UFRProjectileEffectSplit` |
| Drill Shell | Digger, Tunneler, Driller, Penetrator 계열 | 지형을 파서 각도/낙사 문제 해결 | `UFRProjectileEffectDrill` |
| Builder Shell | Builder, Wall, Fortress, Health Aura 일부 | 지형 생성, 방어, 발판 만들기 | `UFRProjectileEffectTerrainCreate` |
| Roller Shell | Roller, Back-Roller, Dual-Roller, Permaroller | 지면을 타고 흐르는 압박 | 신규 GroundRoll effect 후보 |
| Bounce Shell | One-Bounce, Three-Bounce, Bounder, Bouncy Ball | 반사 각도 계산을 보상 | 신규 Bounce effect 후보 |
| Precision Shell | Sniper, Pinpoint, Laser Beam, Taser | 작지만 강한 단일 명중 | 작은 반경, 높은 피해, 조건부 치명타 |
| Arc Bomb | Grenade, Banana, Flower, Water Balloon | 고각 포물선과 착탄 예측 | 높은 중력, 중간 반경 |
| Air Support | Air Strike, Helicopter Strike, AC-130, Artillery | 지형 뒤 적을 위에서 압박 | ImpactSpawns 또는 AbilitySet |
| Beam/Line Shot | Laser Beam, Radar/Sonar/Lidar 감각 | 직선 관통 또는 스캔형 공격 | 신규 Ray/Line effect 후보 |
| Gravity/Control Shell | Gravies, Black Hole, Magnets, Imploder | 적/탄/지형을 끌거나 밀기 | 신규 ForceField effect 후보 |
| Homing Shell | Homing Missile, Drone, Recruiter 일부 | 조준 부담을 줄이는 희귀 공격 | 신규 Homing effect 후보 |
| Fire/Area Hazard | Flame, Napalm, Carpet Fire, Acid Rain | 지형 위 위험 지대 생성 | 신규 HazardZone effect 후보 |
| Risk Shell | Kamikaze, Suicide Bomber, Fury/Rage | 큰 보상과 명확한 손해 | ShotModifier + 자기 피해/불리한 조건 |
| Trick Shell | Portal Gun, Relocator, Confuser, Puzzler | 전투 룰을 잠깐 비트는 희귀 효과 | AbilitySet 또는 전용 ProjectileEffect |

## 4. 데모 우선 무기 12개

데모에는 442개 무기 감각을 모두 넣지 않는다. 대신 위 계열을 대표하는 12개 무기로 "다른 문제 해결 방식"을 보여준다.

| 우선순위 | 무기 | 역할 | 구현 상태 기준 |
| --- | --- | --- | --- |
| P0 | Standard Shell | 기준탄 | 기존 무기 데이터로 가능 |
| P0 | Heavy Shell | 느리고 강한 기준탄 | 기존 스탯만으로 가능 |
| P0 | Drill Shell | 지형 파괴/낙사 유도 | Drill effect 사용 |
| P0 | Builder Shell | 임시 발판/방벽 생성 | TerrainCreate effect 사용 |
| P0 | Split Shell | 착탄 후 3분열 | Split effect 사용 |
| P0 | Cluster Shell | 여러 발 산개 | ProjectileCount 또는 Split effect |
| P1 | Roller Shell | 지면을 타는 탄 | GroundRoll effect 필요 |
| P1 | Bounce Shell | 반사 활용 | Bounce effect 필요 |
| P1 | Air Support | 상단 낙하 공격 | AbilitySet 또는 Spawn effect 필요 |
| P1 | Precision Shell | 작은 반경, 높은 피해 | 기존 스탯 + 조건부 보상으로 가능 |
| P2 | Gravity Shell | 끌어당김/밀어냄 | ForceField effect 필요 |
| P2 | Hazard Shell | 장판/잔류 피해 | HazardZone effect 필요 |

데모 최소 목표는 P0 6개다. P1은 전투 재미를 크게 늘리지만, 새 ProjectileEffect가 필요하므로 구현 비용이 있다. P2는 정식 콘텐츠 쪽으로 미뤄도 된다.

## 5. 무기 Tier 구조

ShellShock의 Tier 1-4 구조는 FortRogue에서 다음처럼 바꾼다.

| Tier | FortRogue 의미 | 예시 |
| --- | --- | --- |
| Tier 1 | 기본 무기 원형 | Drill Shell |
| Tier 2 | 원형 강화 | Wider Drill: 파괴 반경 증가 |
| Tier 3 | 역할 확장 | Split Drill: 충돌 후 작은 굴착탄 생성 |
| Tier 4 | 빌드 축 완성 | Excavator: 지형 파괴 시 추가 피해 또는 낙사 보너스 |

주의:

- Tier가 단순히 피해 +20%, +40%, +60%가 되면 안 된다.
- 상위 Tier는 "이번 런에서 이 무기를 중심으로 플레이할 이유"를 줘야 한다.
- Tier 4는 항상 강할 필요는 없지만, 빌드 방향을 선명하게 바꿔야 한다.

## 6. FortRogue 퍽 축

ShellShock의 Chassis, Track, Gun, Luck을 FortRogue의 패시브 축으로 번역한다.

| FortRogue 축 | 원본 감각 | FortRogue 역할 | 예시 퍽 |
| --- | --- | --- | --- |
| Armor Line | Chassis | 오래 버티며 실수 복구 | Plated Hull: 최대 체력 증가. Emergency Patch: 스테이지마다 첫 피격 후 소량 회복. Last Plate: 체력이 낮을 때 방어막. |
| Mobility Line | Track | 각도와 위치를 만드는 힘 | Extra Tread: 이동력 증가. Climber: 경사 이동 보정. Reposition: 발사하지 않은 턴에 추가 이동. |
| Cannon Line | Gun | 명중 보상과 치명타 | Reinforced Barrel: 직격 피해 증가. Weakpoint Hit: 작은 폭발 반경 무기 치명타 확률 증가. Overbore: 피해 증가 대신 폭발 반경 감소. |
| Salvage Line | Luck | 보상 질, 상자, 아이템 | Scavenger: 보상 선택지 하나 추가 확률. Crate Hook: 가까운 상자 자동 획득. Loaded Crates: 아이템 보상 강화. |

FortRogue에서는 이 축을 영구 성장보다 런 보상/캐릭터 시작 특성으로 먼저 쓴다. 영구 메타 성장으로 확장할 수는 있지만, 데모에서는 런 안 선택이 더 중요하다.

## 7. 보상 설계로 녹이기

ShellShock식 "많은 무기"는 FortRogue에서 "무기 + 보상 조합"으로 확장한다.

| 보상 타입 | 의도 | 예시 |
| --- | --- | --- |
| Weapon Reward | 새 문제 해결 방식 추가 | Drill Shell 획득, Air Support 획득 |
| Perk Reward | 전체 런 방향 강화 | 모든 Split child 피해 증가 |
| Shot Modifier Reward | 특정 조건의 다음 샷 판단 변화 | 낮은 각도 명중 시 폭발 반경 증가 |
| Item Reward | 위기 대응과 한 턴 역전 | 다음 발을 Builder Shell처럼 변환 |
| Upgrade Reward | 기존 무기 Tier 상승 | Standard Shell -> Heavy Shell 선택 |

보상 카드 작성 규칙:

- 카드 설명은 "피해 +10"보다 "어떤 샷을 하고 싶어지는지"를 먼저 말한다.
- 같은 계열 보상은 `RequiredRewardTags`로 체인화한다.
- 강한 특수 보상은 `bOfferOncePerRun`을 켠다.
- 특정 계열을 버리는 보상은 `BlockedRewardTags`로 충돌을 막는다.

## 8. ShellShock 무기 목록을 FortRogue 콘텐츠 풀로 나누기

전체 무기 목록은 `docs/SHELLSHOCK_LIVE_REFERENCE.md`에 보관한다. FortRogue 데이터 작업에서는 개별 이름보다 아래 풀을 기준으로 만든다.

| 콘텐츠 풀 | 포함할 원본 패턴 | FortRogue 사용처 |
| --- | --- | --- |
| Starter Pool | Shot, Grenade, Digger, Splitter, Builder류 | 기본 로드아웃과 초반 보상 |
| Terrain Pool | Digger, Tunneler, Builder, Wall, Quicksand, Earthquake류 | 지형 파괴/생성 빌드 |
| Projectile Pool | Three-Ball, Fleet, Bees, Asteroids, Cluster류 | 다중 투사체 빌드 |
| Angle Pool | Sniper, Shank, Arrow, Laser류 | 정확도/각도 조건 빌드 |
| Movement Pool | Roller, Bounce, Boomerang, Wanderer류 | 지형 이동/반사 빌드 |
| Control Pool | Magnets, Black Hole, Imploder, Relocator류 | 위치 조작/강제 이동 빌드 |
| Support Pool | Health Aura, Potion, Shield/repair 감각 | 생존/회복/방어 빌드 |
| Chaos Pool | Puzzler, Confuser, Portal, Jackpot류 | 고희귀도 룰 변경 빌드 |

## 9. 구현 우선순위

### P0: 현재 구조로 바로 만들 수 있는 것

- Standard Shell, Heavy Shell, Precision Shell
- Drill Shell
- Builder Shell
- Split Shell
- Cluster Shell
- Chassis/Track/Gun/Luck에서 파생한 기본 퍽 12개
- 보상 태그 체인으로 Tier 1-3 무기 업그레이드

### P1: 새 ProjectileEffect 하나씩 추가하면 좋은 것

- GroundRoll effect: Roller 계열
- Bounce effect: Bounce 계열
- AirDrop/Support effect: Air Support 계열
- Knockback/Force effect: Gravity/Control 계열의 첫 단계

### P2: 정식 콘텐츠 후보

- Homing effect
- HazardZone effect
- Portal/Relocate effect
- Confuse/Puzzle 계열 상태 이상
- 상점/휴식 지점 기반 Gear Shop 변환

## 10. 데이터 에셋 작성 예시

### Weapon: Drill Shell

- DisplayName: Drill Shell
- Description: 지형을 깊게 파서 엄폐를 무너뜨리고 낙사를 노린다.
- WeaponTag: `Weapon.Drill`
- ProjectileEffects: `UFRProjectileEffectDrill`
- RewardTag: `Weapon.Drill`

### Perk: Excavation Prize

- Rarity: Epic
- Description: Drill 계열 탄이 지형을 많이 파괴할수록 피해가 증가한다.
- RequiredShotTags: `ShotEffect.Drill`
- EffectTags: `Trait.Terrain`
- 구현 후보: 지형 파괴량 기반 피해 보정이 아직 없으면 P1로 둔다.

### Reward Chain: Split Line

- Tier 1 RewardTag: `Weapon.Split`
- Tier 2 RequiredRewardTags: `Weapon.Split`, RewardTag: `Trait.Split.ChildCount`
- Tier 3 RequiredRewardTags: `Trait.Split.ChildCount`, RewardTag: `Trait.Split.ChildDrill`
- Tier 3 효과: Split child에 Drill effect 추가

## 11. 디자인 리스크

| 리스크 | 왜 위험한가 | 대응 |
| --- | --- | --- |
| 무기 수만 늘어남 | 선택은 많지만 플레이가 비슷해진다. | 계열별 역할을 먼저 고정하고, 같은 역할의 중복 무기는 Tier/스킨/상위 변형으로 묶는다. |
| ShellShock 복제품처럼 보임 | FortRogue의 싱글 로그라이크 정체성이 흐려진다. | 이름, 보상 구조, 런 진행, 적/맵 문제를 FortRogue식으로 설계한다. |
| 보상이 단순 수치화됨 | 다음 샷 판단이 바뀌지 않는다. | ShotModifier와 ProjectileEffect 중심으로 보상 작성. |
| 새 Effect가 과도하게 늘어남 | 구현과 검수가 무거워진다. | P0는 기존 효과로 끝내고, P1부터 effect를 하나씩 추가한다. |
| Luck 축이 랜덤 보상으로만 보임 | 실력보다 운이라는 인상이 커진다. | 선택지 수, 상자 접근, 아이템 변환처럼 플레이어 선택을 늘리는 방향으로 설계한다. |

## 12. 다음 작업

1. P0 무기 6개를 실제 `UFortRogueWeaponDefinition` 데이터로 만든다.
2. Armor/Mobility/Cannon/Salvage 각 3개씩, 총 12개 P0 퍽을 만든다.
3. P0 무기와 퍽을 `RewardPool`에 넣고 7스테이지 런에서 등장하도록 검수한다.
4. 한 런에서 최소 3개의 다른 빌드가 실제로 생기는지 플레이 테스트한다.

완료 기준:

- 데모 런에서 플레이어가 최소 5개 이상의 구분되는 무기 선택을 본다.
- 보상 선택 후 다음 스테이지의 조준/이동/무기 선택이 바뀐다.
- P0 범위는 새 C++ 기능 없이 데이터 중심으로 만들 수 있다.

