# FortRogue Perk Concepts

작성일: 2026-06-26 KST

## 목적

이 문서는 FortRogue에서 사용할 Perk 콘셉트 풀을 관리한다. 실제 `UFRPerkDefinition` 데이터 에셋으로 옮기기 전, 이름, 희귀도, 플레이 의도, 구현 후보를 한곳에서 검토하기 위한 초안이다.

FortRogue의 Perk은 단순한 스탯 보상이 아니라 캐릭터 고유 규칙, Shell, 지형, 각도, 바람, 보상 선택을 연결하는 빌드 엔진이어야 한다.

## 작성 기준

- 같은 Shell을 먹어도 캐릭터마다 다른 판단이 생기게 한다.
- 보상 선택 후 다음 샷, 이동, 무기 선택이 달라져야 한다.
- Common은 약한 보상이 아니라 바로 이해되는 기본형이다.
- Rare는 조건과 시너지를 열지만 설명이 짧아야 한다.
- Epic은 새 기믹, 트레이드오프, 턴 운영 변화를 만든다.
- Legendary는 수치가 큰 보상이 아니라 룰 자체를 바꾼다.
- 실제 데이터화 시 `PerkTag`는 현재 코드 기준 `Trait.*` 카테고리에서 시작한다.
- Shell 조건은 `Weapon.*` 또는 `ShotEffect.*` 태그 조건으로 관리한다.

## 구현 준비도

| 준비도 | 의미 |
| --- | --- |
| P0 | 현재 스탯 보너스, `ShotModifier`, 기존 `ProjectileEffects`로 데이터화 가능 |
| P1 | AbilitySet, 전투 이벤트 후킹, 지형 파괴량/직격 판정 등 추가 로직 필요 |
| P2 | 신규 ProjectileEffect 또는 새 전투 규칙 구현 필요 |

## ShellEffect 조합 기준

현재 코드 이름은 `UFRProjectileEffectBase`지만, 설계상 목표는 `ProjectileEffect`보다 넓은 `ShellEffect` 조합 방식이다. Shell은 여러 effect 조각을 순서대로 조립해서 만들고, `FFRShotSpec`은 그 조립 결과로 나온 최종 런타임 값이다.

결정:

- 새 효과를 만들 때 `FFRShotModifierSpec`에 단순 수치 필드를 계속 추가하지 않는다.
- `ShotModifier`는 장기적으로 "조건 + ShellEffect 묶음" 역할로 축소한다.
- `HitDamage`, `Damage`, `BlastRadius`, `TerrainDamage`, `LaunchSpeed`, `Gravity`, `ProjectileCount` 같은 값은 최종 `FFRShotSpec`에는 남긴다.
- 위 값을 바꾸는 데이터는 `BonusHitDamage`, `ExplosionDamageBonus`, `TerrainDamageMultiplier` 같은 ShellEffect 파라미터로 둔다.
- 기존 `ShotModifier` 수치 필드는 당장 삭제하지 않고, 새 데이터 설계에서는 ShellEffect 방식으로 우선 작성한다.

### ShellEffect 단계

ShellEffect는 어느 단계에 개입하는지에 따라 나눈다.

| 단계 | 역할 | 예시 |
| --- | --- | --- |
| Spec Effect | 발사 전 `FFRShotSpec` 값을 만든다 | DirectHitDamage, ExplosionPayload, TerrainCarvePayload, TerrainFillPayload, FlightProfile |
| Fire Pattern Effect | 발사 입력 하나가 투사체를 어떻게 스폰하는지 정한다 | SingleShot, MultiProjectile, Salvo, Burst, Spread |
| Runtime Projectile Effect | 비행 중 투사체 상태를 매 Tick 바꾼다 | Homing, Roller, Bounce, WindBend, ForcePull |
| Impact Effect | 충돌 시 추가 결과를 만든다 | Split, DrillImpact, TerrainCreateImpact, TerrainColumn, ExtraExplosion, HazardZone |
| Run Event Effect | 샷 결과나 전투/보상 이벤트에 반응한다 | DirectHitRefund, LuckyChamber, BlackMarketPatron, RewardBias |

### ShotModifier의 역할

`ShotModifier`는 effect 자체가 아니라 effect를 적용할 조건과 묶음을 들고 있는 래퍼로 본다.

ShotModifier가 담당하는 것:

- 낮은 각도일 때만 적용
- 바람 방향과 같을 때만 적용
- `Weapon.*` 또는 `ShotEffect.*` 태그가 있을 때만 적용
- 특정 태그가 있으면 적용하지 않음
- 위 조건을 만족하면 ShellEffect 배열을 최종 ShotSpec 조립에 추가

ShotModifier가 새로 가져가면 안 되는 것:

- `BonusHitDamage` 같은 새 전투 수치 필드
- `SalvoCount` 같은 발사 패턴 필드
- `SplitChildCount` 같은 특정 effect 전용 파라미터
- 보상 선택지, 상점 가격, 캐릭터 보상 가중치 같은 런 시스템 값

### WeaponTag의 역할

`WeaponTag`는 무기의 효과가 아니라 무기의 정체성을 나타내는 식별자다.

사용 목적:

- UI나 입력에서 특정 무기를 태그로 선택한다.
- `ShotModifier.RequiredShotTags` / `BlockedShotTags`에서 "이 무기일 때만 적용" 같은 조건으로 쓴다.
- 보상 요약과 데이터 검수에서 어떤 무기인지 추적한다.
- 런 보상 태그와 연결해 "이 무기를 이미 얻었는가"를 추적할 수 있다.

구분:

| 태그 | 의미 | 예시 |
| --- | --- | --- |
| `Weapon.*` | 어떤 무기/무기 계열인가 | `Weapon.Shell`, `Weapon.Miner.Special` |
| `ShotEffect.*` | 이번 샷이 어떤 효과를 갖고 있는가 | `ShotEffect.Drill`, `ShotEffect.SplitOnImpact` |
| `Trait.*` | 런에서 얻은 퍽/패시브 축 | `Trait.Damage`, `Trait.ShotModifier` |

중요한 점:

- `WeaponTag` 자체는 피해, 분열, 굴착 같은 동작을 만들지 않는다.
- 동작은 ShellEffect가 만든다.
- 같은 `WeaponTag`라도 effect 구성이 바뀌면 전혀 다른 샷이 될 수 있다.
- 같은 ShellEffect라도 `WeaponTag`가 다르면 보상 조건, 캐릭터 Bias, UI 분류가 달라질 수 있다.

### 애매한 경우의 결정 규칙

- `HitDamage +20`은 `ShotModifier` 필드가 아니라 DirectHitDamage ShellEffect 파라미터다.
- `폭발 반경 +20%`는 ExplosionPayload ShellEffect 파라미터다.
- `Drill 반경 +20%`는 TerrainCarvePayload 또는 DrillImpact ShellEffect 파라미터다.
- `모든 탄이 Drill처럼 지형을 판다`는 해당 Shell에 DrillImpact ShellEffect를 추가한다.
- `Split child 피해 +20%`는 Split effect 내부 child ShellEffect 또는 child ShotModifier로 처리한다.
- `Split child 수 +1`은 Split effect 전용 파라미터다.
- `SalvoCount +1`은 Fire Pattern Effect다. 투사체 충돌 효과가 아니므로 현재 이름 그대로라면 `ProjectileEffect`에 넣지 않는다.
- `직격하면 다음 턴 강화`처럼 결과 이벤트가 필요하면 Run Event Effect 또는 AbilitySet으로 처리한다.
- `보상 선택지 +1`, `상점 가격 감소`, `캐릭터 보상 가중치`는 Run Event Effect 또는 보상 시스템 modifier로 처리한다.

## 축 분류

| 축 | 역할 |
| --- | --- |
| Armor | 생존, 회복, 실수 복구 |
| Mobility | 이동, 위치 선정, 각도 확보 |
| Cannon | 피해, 직격, 탄속, 폭발 |
| Terrain | 지형 파괴, 생성, 낙사, 엄폐 |
| Projectile | 다중탄, 분열탄, 자식 탄, 탄 형태 |
| Wind | 풍향, 풍속, 궤적 읽기 |
| Economy | 보상 선택, 상자, 상점, 아이템 |
| Risk | 손해를 받고 강한 효과 획득 |
| Character | 캐릭터 Core, 전용 규칙, Reward Bias |
| Rule | 전투 룰 변경 |

## Common (30)

| ID | 이름 | 축 | 효과 콘셉트 | 구현 후보 |
| --- | --- | --- | --- | --- |
| C01 | Reinforced Barrel | Cannon | 모든 Shell 피해가 소폭 증가한다. | P0: DamageBonus |
| C02 | Wider Fuse | Cannon | 모든 폭발 반경이 소폭 증가한다. | P0: ShotModifier BlastRadius |
| C03 | Stable Powder | Cannon | 샷 파워 배율이 소폭 증가해 같은 차지로 더 멀리 쏜다. | P0: ShotPowerMultiplierBonus |
| C04 | Extra Tread | Mobility | 턴당 이동 예산이 증가한다. | P0: MaxMoveBudgetBonus |
| C05 | Plated Hull | Armor | 최대 체력이 증가한다. | P0: MaxHealthBonus |
| C06 | Light Frame | Mobility | 이동 예산이 증가하지만 최대 체력이 조금 감소한다. | P0: Move/Health tradeoff |
| C07 | Heavy Frame | Armor | 최대 체력이 증가하지만 이동 예산이 조금 감소한다. | P0: Health/Move tradeoff |
| C08 | Primer Charge | Cannon | 첫 번째 선택 Shell의 피해가 증가한다. | P0: RequiredShotTags 또는 무기 슬롯 조건 후보 |
| C09 | Packed Payload | Cannon | 폭발 피해가 증가하지만 탄속이 조금 감소한다. | P0: ShotModifier Damage/LaunchSpeed |
| C10 | Clean Bore | Cannon | 탄속이 증가하고 중력 영향을 조금 덜 받는다. | P0: LaunchSpeed/Gravity modifier |
| C11 | Low Angle Practice | Cannon | 낮은 각도에서 명중하면 피해가 증가한다. | P0: AimAngle condition |
| C12 | High Arc Practice | Cannon | 높은 각도에서 명중하면 폭발 반경이 증가한다. | P0: AimAngle condition |
| C13 | Drill Bit | Terrain | Drill 효과의 지형 파괴 반경이 증가한다. | P0: Required ShotEffect.Drill |
| C14 | Scaffold Kit | Terrain | TerrainCreate 효과의 생성 반경이 증가한다. | P0: Required ShotEffect.TerrainCreate |
| C15 | Split Primer | Projectile | Split child 피해가 소폭 증가한다. | P0: Split ChildShotModifiers |
| C16 | Cluster Packing | Projectile | 다중 투사체 Shell의 피해 감소폭을 줄인다. | P0: Required ShotEffect.Projectiles 후보 |
| C17 | Blast Padding | Armor | 자신의 폭발에 가까운 지형 피해 위험을 줄이는 방향의 방어형 퍽. | P1: 자기 피해/지형 안정 로직 |
| C18 | Crate Sense | Economy | 가까운 상자 위치를 더 명확히 보여주거나 보상 획득 반경을 늘린다. | P1: 상자 감지/획득 로직 |
| C19 | Spare Parts | Economy | 아이템 보상 수량이 소폭 증가한다. | P1: Reward/Item amount hook |
| C20 | Emergency Patch | Armor | 스테이지마다 첫 피격 후 소량 회복한다. | P1: AbilitySet |
| C21 | Ridge Footing | Mobility | 높은 지형에 있을 때 이동 예산이 조금 증가한다. | P1: 위치 높이 조건 |
| C22 | Ground Reader | Terrain | 약한 지형 위의 적에게 주는 피해가 증가한다. | P1: 지형 내구도 조건 |
| C23 | Wind Notes | Wind | 바람 방향으로 쏜 탄의 탄속이 소폭 증가한다. | P0: bRequireWindAligned |
| C24 | Counterweight | Wind | 바람 반대 방향으로 쏠 때 중력 영향을 조금 덜 받는다. | P0: Wind condition 후보 |
| C25 | Direct Hit Drill | Cannon | 직격 피해가 증가한다. | P1: hit damage/직격 판정 보정 |
| C26 | Short Fuse | Cannon | 가까운 거리 명중 시 폭발 반경이 증가한다. | P1: 거리 조건 |
| C27 | Long Fuse | Cannon | 먼 거리 명중 시 피해가 증가한다. | P1: 거리 조건 |
| C28 | Salvage Habit | Economy | 전투 보상 선택지 중 기본형 퍽 등장 가중치가 증가한다. | P1: RewardWeight bias |
| C29 | Safe Charge | Armor | 차지 중 피격되거나 턴 손실 시 소량 방어막을 얻는다. | P1: AbilitySet |
| C30 | Familiar Shell | Character | Signature Core 계열 Shell의 기본 수치가 소폭 증가한다. | P1: 캐릭터 Core 조건 |

## Rare (30)

| ID | 이름 | 축 | 효과 콘셉트 | 구현 후보 |
| --- | --- | --- | --- | --- |
| R01 | Low Angle Doctrine | Cannon | 25도 이하로 명중하면 피해와 넉백이 증가한다. | P0/P1: angle + knockback |
| R02 | High Ground Routine | Mobility | 적보다 높은 지형에서 쏘면 폭발 반경이 증가한다. | P1: 높이 비교 조건 |
| R03 | Buried Prize | Terrain | 지형을 많이 파괴한 샷 이후 상자/돈 보상 확률이 증가한다. | P1: 파괴량 기록 |
| R04 | Emergency Scaffold | Terrain | 체력이 낮을 때 피격되면 발밑에 작은 발판을 만든다. | P1: AbilitySet + TerrainCreate |
| R05 | Split Shrapnel | Projectile | Split child가 작은 추가 폭발을 가진다. | P0: Split child modifier |
| R06 | Child Labor | Projectile | Split child가 지형 파괴량을 조금 더 가진다. | P0: child Drill/TerrainDamage |
| R07 | Drill Followthrough | Terrain | Drill Shell이 적 아래 지형을 파면 다음 샷 피해가 증가한다. | P1: 파괴 위치 조건 |
| R08 | Builder's Cover | Terrain | 생성한 지형 근처에 있으면 받는 피해가 감소한다. | P1: 생성 지형 근접 판정 |
| R09 | Wind Rider | Wind | 바람 방향으로 쏜 탄은 피해가 증가하고 반대 방향은 감소한다. | P0: wind aligned + tradeoff |
| R10 | Weather Vane | Wind | 첫 발을 쏘기 전 현재 바람 정보를 더 정확하게 읽는다. | P1: UI/예측 보조 |
| R11 | Salvage Contract | Economy | 상자 보상이 좋아지지만 전투 시작 체력이 감소한다. | P1: reward quality + health penalty |
| R12 | Black Market Coupon | Economy | 상점 가격이 낮아지지만 회복 보상 등장률이 감소한다. | P1: shop/reward bias |
| R13 | Hollow Charge | Cannon | 폭발 반경은 줄지만 직격 피해가 크게 증가한다. | P0/P1: blast down + hit damage |
| R14 | Oversized Warhead | Cannon | 폭발 반경이 크게 증가하지만 탄속이 감소한다. | P0 |
| R15 | Creeping Shot | Projectile | 낮은 탄속 Shell의 피해와 지형 피해가 증가한다. | P1: launch speed condition |
| R16 | Snap Aim | Mobility | 이동하지 않은 턴의 직격 피해가 증가한다. | P1: movement spent condition |
| R17 | Reposition Drill | Mobility | 발사하지 않고 이동만 한 턴 이후 다음 샷 탄속이 증가한다. | P1: turn action history |
| R18 | Cracked Floor Tactics | Terrain | 약한 지형 위의 적에게 폭발 반경이 증가한다. | P1: terrain health condition |
| R19 | Shielded Fuse | Armor | 큰 폭발 반경 Shell을 쏠 때 자신에게 임시 방어막을 준다. | P1: AbilitySet + blast condition |
| R20 | Second Fuse | Cannon | 한 스테이지에서 첫 빗나간 샷 이후 다음 샷 피해가 증가한다. | P1: miss tracking |
| R21 | Lucky Chamber | Economy | 보상 화면에 낮은 확률로 추가 선택지가 하나 열린다. | P1: RewardChoiceCount hook |
| R22 | Crate Hook | Economy | 가까운 상자를 자동 획득할 수 있다. | P1: crate pickup |
| R23 | Miner Bias | Character | Drill, Quake, Terrain 계열 보상 가중치가 증가한다. | P1: character reward bias |
| R24 | Engineer Bias | Character | Builder, Shield, Repair 계열 보상 가중치가 증가한다. | P1: character reward bias |
| R25 | Gunslinger Bias | Character | Precision, Crit, Low Angle 계열 보상 가중치가 증가한다. | P1: character reward bias |
| R26 | Scavenger Bias | Character | Luck, Shop, Item 계열 보상 가중치가 증가한다. | P1: character reward bias |
| R27 | Soft Landing | Armor | 낙사 직전 한 번만 작은 발판을 생성하거나 낙사 피해를 줄인다. | P1: fall prevention |
| R28 | Follow-up Powder | Cannon | 직전 턴과 같은 Shell을 쓰면 피해가 증가한다. | P1: previous shell tracking |
| R29 | Alternating Loadout | Projectile | 직전 턴과 다른 Shell을 쓰면 투사체 수가 증가한다. | P1: previous shell tracking |
| R30 | Target Marker | Cannon | 같은 적을 연속으로 맞히면 두 번째 명중 피해가 증가한다. | P1: target history |

## Epic (20)

| ID | 이름 | 축 | 효과 콘셉트 | 구현 후보 |
| --- | --- | --- | --- | --- |
| E01 | Excavation Chain | Terrain | Drill로 적 아래 지형을 파면 다음 샷이 피해와 폭발 반경을 얻는다. | P1: terrain destruction event |
| E02 | Split Payload | Projectile | Split child가 현재 Shell의 일부 ProjectileEffect를 상속한다. | P0/P1: Split child modifier setup |
| E03 | Payload Echo | Rule | 이번 턴 선택한 Shell이 명중하면 다음 턴 같은 Shell의 투사체 수가 증가하고 피해가 감소한다. | P1: turn history + modifier |
| E04 | Wind Banking | Wind | 바람 방향으로 명중하면 다음 턴 바람 반대 방향 페널티를 무시한다. | P1: wind state buff |
| E05 | Siege Rhythm | Cannon | 2턴 연속 같은 각도 범위로 명중하면 세 번째 샷이 강화된다. | P1: aim history |
| E06 | Moving Battery | Mobility | 이동 후 발사하면 피해는 감소하지만 폭발 반경과 넉백이 증가한다. | P1: movement condition |
| E07 | No-Move Emplacement | Cannon | 이동하지 않은 턴에는 피해와 직격 보너스가 증가하지만 피격 피해도 증가한다. | P1: risk condition |
| E08 | Fortress Routine | Terrain | Builder로 만든 지형 위나 근처에서 쏘면 방어막과 탄속 보너스를 얻는다. | P1: created terrain ownership |
| E09 | Deep Vein | Economy | 지형 안의 광맥/상자를 파괴하면 보상이 강화되고 다음 보상에서 Terrain 계열이 늘어난다. | P1: map object reward hook |
| E10 | Volatile Mining | Risk | 지형 파괴량이 클수록 피해가 오르지만 자기 주변 지형도 약해진다. | P1/P2: self terrain weakening |
| E11 | Precision Refund | Cannon | 작은 폭발 반경 Shell로 직격하면 다음 턴 아이템 충전 또는 보상 조각을 얻는다. | P1: hit radius condition |
| E12 | Scatter Discipline | Projectile | 투사체 수가 많을수록 산개가 줄지만 각 투사체 피해가 감소한다. | P1: spread control |
| E13 | Ricochet Lesson | Projectile | 반사 또는 굴절 후 명중한 탄이 추가 피해를 준다. | P2: Bounce effect |
| E14 | Roller Momentum | Projectile | 지면을 탄 Shell이 이동한 거리에 따라 피해가 증가한다. | P2: GroundRoll effect |
| E15 | Hazard Claim | Terrain | 남긴 장판이나 위험 지대 위의 적에게 추가 피해를 준다. | P2: HazardZone effect |
| E16 | Salvage Engine | Economy | 보상 선택지 하나를 포기하면 남은 선택지의 희귀도 가중치가 증가한다. | P1: reward reroll/skip UI |
| E17 | Rival Study | Character | 라이벌/엘리트 처치 후 캐릭터 전용 Perk 등장률이 증가한다. | P1: encounter reward bias |
| E18 | Core Infusion | Character | 일반 Shell 하나가 Signature Core 태그 조건을 일부 받는다. | P1: character core bridge |
| E19 | Controlled Collapse | Terrain | 낙사나 지형 붕괴로 적을 처치하면 다음 샷이 강화된다. | P1: kill cause tracking |
| E20 | Emergency Overload | Risk | 체력이 낮을 때 모든 Shell이 강화되지만 회복 효율이 감소한다. | P1: health threshold + healing modifier |

## Legendary (10)

| ID | 이름 | 축 | 효과 콘셉트 | 구현 후보 |
| --- | --- | --- | --- | --- |
| L01 | Unstable Barrel | Rule | 모든 Shell이 한 번 더 폭발하지만 플레이어 주변 지형도 매 턴 약해진다. | P1/P2: extra impact + self terrain risk |
| L02 | Borrowed Core | Character | 각 스테이지 첫 발에서 장착한 일반 Shell이 Signature Core 효과 일부를 복사한다. | P1: core infusion rule |
| L03 | Drill World | Terrain | 모든 투사체가 약한 Drill 효과를 얻고 Drill Shell은 추가 지형 파괴 보너스를 받는다. | P0/P1: global Drill modifier |
| L04 | Architect's Law | Terrain | 모든 폭발이 작은 발판을 남기지만 적도 그 발판을 이용할 수 있다. | P1/P2: global TerrainCreate + enemy path |
| L05 | Wind Sovereign | Wind | 각 스테이지 첫 발은 바람을 무시하고, 이후 바람 방향 명중이 영구 보너스를 쌓는다. | P1: wind ignore + stacking buff |
| L06 | Mirror Arsenal | Rule | 직전 턴 사용한 Shell 효과 일부가 다음 턴 다른 Shell에 붙는다. | P1: previous shell effect copy |
| L07 | Catastrophe Chain | Projectile | 한 발로 2명 이상 맞히면 다음 샷이 분열하고, 이미 분열탄이면 child 수가 증가한다. | P1: multi-hit tracking |
| L08 | Last Stand Cannon | Risk | 체력이 30% 이하일 때 피해, 폭발 반경, 투사체 수가 증가하지만 회복 보상이 사라진다. | P1: health threshold + reward block |
| L09 | Black Market Patron | Economy | 매 보상 화면에서 체력을 지불해 추가 Rare 이상 Perk을 볼 수 있다. | P1: reward UI + health cost |
| L10 | Perfect Trajectory | Rule | 스테이지마다 한 번, 직격 명중한 탄이 같은 궤적으로 약한 복제탄을 다시 발사한다. | P1/P2: trajectory replay |

## 초기 데이터화 후보

데모에 바로 넣기 좋은 후보는 구현 준비도 P0 중심으로 고른다.

| 우선순위 | 후보 |
| --- | --- |
| P0 Common | Reinforced Barrel, Wider Fuse, Stable Powder, Extra Tread, Plated Hull, Low Angle Practice, High Arc Practice, Drill Bit, Scaffold Kit, Split Primer |
| P0 Rare | Low Angle Doctrine, Split Shrapnel, Child Labor, Wind Rider, Hollow Charge, Oversized Warhead |
| P0 Epic | Split Payload |
| P0 Legendary | Drill World |

## 보류 기준

- 새로운 ProjectileEffect가 필요한 Perk은 해당 Effect 설계가 끝나기 전까지 데이터 에셋으로 만들지 않는다.
- 지형 파괴량, 직격, 낙사, 이전 턴 행동처럼 이벤트 기록이 필요한 Perk은 먼저 전투 로그/조건 API를 정리한다.
- Reward Bias Perk은 캐릭터별 보상 풀 설계가 확정된 뒤 실제 가중치로 옮긴다.
- UI에서 조건 실패 이유를 설명할 수 없는 Perk은 보상 풀에 넣지 않는다.
