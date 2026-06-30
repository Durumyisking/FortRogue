# External Project Architecture Learnings

분석 대상:
- `D:\Project\ClevvonChronicles`
- `D:\Project\LyraStarterGame`

목적은 특정 전투 기능을 복사하는 것이 아니라, 두 프로젝트가 전체 코드베이스에서 책임을 어떻게 나누고 객체를 어떻게 배치하는지 학습해서 FortRogue 작업 기준으로 삼는 것이다.

## 1. 전체 구조에서 보이는 차이

### LyraStarterGame

Lyra는 기능 축이 매우 강하게 분리되어 있다.

대표 폴더:
- `AbilitySystem`
- `Camera`
- `Character`
- `Equipment`
- `GameFeatures`
- `GameModes`
- `Input`
- `Inventory`
- `Messages`
- `Player`
- `System`
- `Teams`
- `UI`
- `Weapons`

특징:
- 액터 하나가 모든 기능을 직접 들고 있지 않는다.
- `Character`는 Pawn 생명주기와 핵심 연결만 맡고, 실제 기능은 `PawnExtensionComponent`, `HealthComponent`, `CameraComponent`, Equipment, Ability, Weapon 쪽으로 분리한다.
- 게임 모드 구성은 `ExperienceDefinition`, `ExperienceManagerComponent`, `GameFeatureAction`이 맡는다.
- 전역 접근/공통 조회는 `AssetManager`, `WorldSubsystem`, `GameInstanceSubsystem` 계층으로 뺀다.
- 입력도 `InputAction`을 직접 하드코딩하지 않고 `LyraInputConfig` 데이터 에셋에서 GameplayTag와 매핑한다.

Lyra에서 배울 점:
- Actor는 "기능 구현자"보다 "소유자/조립자"에 가깝다.
- 런타임 상태와 기능 생명주기가 있으면 Component 또는 Subsystem으로 격리한다.
- 설정값은 DataAsset/PrimaryDataAsset로 편집 가능하게 둔다.
- UI는 게임 로직의 원본이 아니라 표시 계층이다.

### ClevvonChronicles

Clevvon은 Lyra보다 프로젝트 고유 시스템이 더 직접적이고, DataTable/DataAsset/Subsystem/Widget 비중이 크다.

대표 폴더:
- `Actor`
- `Animation`
- `Combat`
- `Game`
- `GameData`
- `MVVM`
- `Object`
- `Player`
- `Quest`
- `Widget`

특징:
- `GameData`에 캐릭터, 장비, 액션, 퀘스트, 사운드, 모드 설정 같은 편집 데이터를 넓게 둔다.
- `CCAssetManager`, `CCDataManageSubsystem`이 데이터 테이블과 에셋 접근을 중앙화한다.
- `CCInventorySubsystem`, `CCGameFlowManager`, `CCObjectPoolSubsystem`, `CCViewModelSubsystem`처럼 장기 상태나 전역 흐름은 Subsystem이 담당한다.
- `Widget` 계층이 매우 크지만, `CCUserWidget`, `CCHUDWidget`, `CCTableWidgetBase`, `CCTooltipBase`, `CCWidgetComponent` 같은 공통 베이스를 두어 반복 UI 동작을 재사용한다.
- 일부 Actor는 크고 많은 책임을 갖지만, 시각 표현, 이동 안전성, 효과 라우팅 같은 독립 기능은 Component로 분리하려는 방향이 보인다.

Clevvon에서 배울 점:
- 프로젝트가 커질수록 데이터 접근, 저장, 인벤토리, UI 루트, 오브젝트 풀 같은 것은 Actor에 넣으면 안 된다.
- 반복되는 UMG 패턴은 공통 위젯 베이스나 전용 위젯 컴포넌트로 묶는 것이 낫다.
- 다만 큰 Actor에 책임이 다시 쌓일 수 있으므로, FortRogue에서는 이 부분을 그대로 따라 하면 안 된다.

## 2. 공통으로 확인된 책임 분배 원칙

### Actor는 소유와 조립을 맡는다

두 프로젝트 모두 잘 정리된 부분에서는 Actor가 모든 계산을 직접 수행하지 않는다. Actor는 컴포넌트 생성, 이벤트 연결, 현재 상태 조회의 진입점 역할을 한다.

FortRogue 기준:
- `AFRBattleCharacter`가 발사 위치, 체력 UI, 각도 표시 UI, 지형 회전 보정, 투사체 방향 계산을 모두 직접 들고 있으면 책임이 퍼진다.
- Actor는 `BodyFrame`, `Sprite`, `Muzzle`, `AngleIndicatorWidget`, `HpWidget` 같은 하위 컴포넌트를 소유하고, 실제 기준값은 해당 컴포넌트에서 가져오게 해야 한다.

### Component는 독립 상태와 생명주기를 가진 기능에 붙인다

Lyra의 `HealthComponent`, `PawnExtensionComponent`, Equipment/Weapon 구조와 Clevvon의 이동 안전성/비주얼/효과 라우팅 컴포넌트는 같은 방향이다.

FortRogue 기준:
- 단순히 함수 하나 줄이려고 Component를 만들 필요는 없다.
- 하지만 자체 상태, Tick, 델리게이트, 에디터 설정, 다른 시스템과의 연결이 생기면 Component로 빼야 한다.
- 예: HP 표시와 각도 인디케이터는 서로 회전 기준도 다르고 부착 위치도 다르므로 같은 WidgetComponent 안에 같이 넣는 것은 좋지 않다.

### Transform의 원본은 SceneComponent/Socket이어야 한다

Lyra는 무기/장비를 Mesh Socket이나 Pawn Root에 붙이고, Clevvon도 비주얼 컴포넌트가 장비 메시를 소켓에 붙인다. 위치를 코드에서 숫자로 계속 재계산하지 않는다.

FortRogue 기준:
- 총알 생성 위치는 `GetActorLocation() + Direction * 70 + Z` 같은 하드코딩 오프셋이 아니라 `Muzzle` 컴포넌트 위치여야 한다.
- 캐릭터 몸체 회전은 `BodyFrame` 하나가 담당하고, `Sprite`, `Muzzle`, `AngleIndicatorWidget`은 그 하위에 둔다.
- HP는 몸체 회전 영향을 받으면 안 되므로 `BodyFrame` 밖에 둔다.

권장 계층:

```text
BattleCharacter
  BodyFrame
    AngleIndicatorWidget
    Sprite
    Muzzle
  HpWidget
```

이 구조에서는:
- 몸체가 지형에 맞춰 회전하면 `BodyFrame`만 회전한다.
- 스프라이트, 총구, 각도 인디케이터는 같은 기준 회전을 공유한다.
- 체력바는 별도 컴포넌트라 회전 영향을 받지 않는다.
- 발사 방향/위치는 `Muzzle` transform에서 얻는다.

### UI는 계산의 주인이 아니다

Lyra의 Indicator 시스템은 `IndicatorDescriptor`가 Actor/Component/Socket을 가리키고, UI는 그 위치를 표시한다. Clevvon의 위젯들도 공통 베이스와 Subsystem이 상태를 공급하는 식으로 구성되어 있다.

FortRogue 기준:
- 각도 인디케이터 위젯은 캐릭터 회전이나 발사 위치를 계산하면 안 된다.
- 위젯은 `MinAngle`, `MaxAngle`, `CurrentAngle` 같은 표시용 파라미터만 받는다.
- 표시 각도 변경은 Material Instance Dynamic의 Scalar Parameter로 처리한다.
- 숫자 텍스트를 보여줄 필요가 없다면 UI 요소를 추가하지 않는다.

### 데이터는 편집 가능한 자산으로 둔다

Lyra는 `PawnData`, `ExperienceDefinition`, `InputConfig`, `AbilitySet`, `EquipmentDefinition` 같은 데이터 에셋을 많이 쓴다. Clevvon은 DataTable/DataAsset 중심이다.

FortRogue 기준:
- 무기별 최소/최대 각도, 탄 속도, UI 머티리얼, 위젯 클래스 같은 값은 가능한 `UPROPERTY(EditDefaultsOnly/EditAnywhere)` 또는 DataAsset 쪽으로 열어둔다.
- 단, 아직 한 번만 쓰는 설정을 위해 새 DataAsset 체계를 만드는 것은 과하다. 기존 FortRogue 패턴 안에서 노출 가능한 프로퍼티부터 쓰는 것이 낫다.

### 전역 상태는 Subsystem으로 둔다

Lyra의 Team/UI/Message 계층과 Clevvon의 Inventory/GameFlow/ObjectPool/ViewModel 계층은 공통적으로 전역 상태를 Actor에서 빼고 있다.

FortRogue 기준:
- 캐릭터 한 개에만 필요한 상태는 Actor/Component에 둔다.
- 여러 캐릭터, 전투 전체, 저장/로드, 풀링, UI 루트처럼 수명이 넓은 것은 Subsystem 후보로 본다.
- 전역 매니저를 먼저 만들지 말고, 두 곳 이상에서 같은 책임이 반복될 때 승격한다.

## 3. 코드 스타일에서 배울 점

### Unreal 기능을 먼저 쓴다

두 프로젝트 모두 C++ 자체 유틸보다 Unreal의 구조를 적극적으로 쓴다.

사용 패턴:
- `UActorComponent`
- `USceneComponent`
- `UWidgetComponent`
- `UDataAsset`
- `UPrimaryDataAsset`
- `UGameInstanceSubsystem`
- `UWorldSubsystem`
- `GameplayTag`
- `Delegate`
- `SoftObjectPtr` / `SoftClassPtr`
- `BindWidget`

FortRogue 기준:
- 임의의 싱글톤, 전역 static 상태, 수동 좌표 보정 함수를 먼저 만들지 않는다.
- 컴포넌트 부착, 소켓, Material Parameter, UPROPERTY 노출 같은 Unreal 기본 수단을 먼저 쓴다.

### 함수 이름은 책임을 드러내야 한다

Lyra는 `InitializeAbilitySystem`, `UninitializeAbilitySystem`, `SetCurrentExperience`, `FindNativeInputActionForTag`처럼 함수가 맡는 일을 드러낸다.

Clevvon도 `ResolveIntendedDestination`, `BuildBakedEnemyParty`, `ShowCanvasSingleton`, `PushFocusWidget`, `AcquireActorByKey`처럼 기능 단위가 이름에 나타난다.

FortRogue 기준:
- `UpdateCharacterRotation()` 같은 함수 안에서 지형 샘플, Sprite 회전, Indicator 회전, 발사 기준 계산까지 섞이면 이름과 책임이 맞지 않는다.
- `UpdateBodyFrameFromTerrain()`, `GetMuzzleTransform()`, `RefreshAimIndicatorMaterial()`처럼 기준을 나눠야 한다.

### 이벤트/델리게이트로 변경을 전파한다

Lyra의 Health/Message 계층, Clevvon의 Inventory/ViewModel/Widget 계층은 상태 변경을 델리게이트로 흘려보내는 패턴을 많이 쓴다.

FortRogue 기준:
- HP 변경, 조준 각 변경, 장비 변경처럼 값이 바뀌는 시점이 명확하면 위젯이 매 프레임 추측하지 않게 한다.
- 지금 당장 큰 이벤트 시스템을 만들 필요는 없지만, 새 UI가 계속 늘어나면 델리게이트 기반 갱신으로 바꾸는 편이 좋다.

## 4. FortRogue에 적용할 구체 원칙

1. `AFRBattleCharacter`는 얇게 유지한다.
   - 컴포넌트를 소유하고 외부 API를 제공하되, 화면 표시와 발사 위치 계산을 직접 다 떠안지 않는다.

2. 캐릭터 회전 기준은 `BodyFrame` 하나로 통일한다.
   - Sprite, Muzzle, AngleIndicator는 BodyFrame 하위에 둔다.
   - 캐릭터 지형 회전도 BodyFrame에만 적용한다.

3. 총알 생성은 `Muzzle`에서 한다.
   - 생성 위치는 `Muzzle->GetComponentLocation()`.
   - 발사 방향은 `Muzzle->GetForwardVector()` 또는 FortRogue 2D 축에 맞춘 명시 함수에서 가져온다.

4. HP 위젯과 각도 인디케이터 위젯은 분리한다.
   - HP는 BodyFrame 밖에 둬서 회전 영향을 받지 않게 한다.
   - 각도 인디케이터는 BodyFrame 안에 둬서 캐릭터 회전 영향을 받게 한다.

5. 각도 인디케이터는 표시만 한다.
   - 머티리얼 파라미터는 `MinAngle`, `MaxAngle`, `CurrentAngle` 세 개면 충분하다.
   - 위젯은 1:1 비율을 기준으로 만들고, 머티리얼 내부의 원점/기준축이 위젯 중앙과 맞아야 한다.
   - 숫자 표시는 만들지 않는다.

6. 하드코딩 오프셋은 컴포넌트 위치로 대체한다.
   - 위치가 의미를 가진다면 `UPROPERTY`로 노출된 SceneComponent를 둔다.
   - 특히 총구, UI 기준점, 스프라이트 중심은 코드 숫자가 아니라 컴포넌트/피벗이 원본이어야 한다.

7. 데이터는 에디터에서 조정 가능하게 둔다.
   - 각도 범위, 위젯 클래스, 머티리얼, DrawSize, 총구 위치는 C++ 상수보다 프로퍼티/블루프린트 조정값이 낫다.
   - 다만 단일 기능 때문에 새 매니저나 새 DataAsset 체계를 만들지는 않는다.

8. 새 추상화는 반복 책임이 보일 때만 만든다.
   - Lyra의 구조를 그대로 가져오면 FortRogue 규모에는 과하다.
   - Clevvon처럼 큰 액터에 계속 붙이는 것도 피해야 한다.
   - 현재 문제는 먼저 컴포넌트 계층과 책임 기준을 바로잡는 것이 우선이다.

## 5. 현재 각도/체력바 문제에 대한 판단

현재 FortRogue 문제는 단순히 머티리얼 파라미터나 위젯 위치 하나의 문제가 아니다.

근본 원인:
- 캐릭터 몸체 회전 기준, 스프라이트 회전 기준, 각도 인디케이터 회전 기준, 총알 생성 기준이 한 계층에서 나오지 않는다.
- HP와 각도 인디케이터가 같은 위젯 컴포넌트 안에 섞여 있다.
- 총알 생성 위치가 총구 컴포넌트가 아니라 코드 오프셋으로 계산된다.
- 각도 인디케이터는 캐릭터 회전 영향을 받아야 하는데, 별도 UMG RenderTransform으로 맞추려 하고 있다.

해결 방향:
- `BodyFrame` 중심 계층으로 회전 책임을 모은다.
- `Muzzle`을 발사 위치/방향의 원본으로 만든다.
- `AngleIndicatorWidget`은 `BodyFrame` 하위에 둔다.
- `HpWidget`은 `BodyFrame` 밖에 둔다.
- 머티리얼은 표시 범위와 현재 각도만 받아 그린다.

이 방향이 Lyra의 "Actor는 조립, 기능은 컴포넌트/데이터", Clevvon의 "반복 UI/데이터/전역 상태는 별도 계층"과 둘 다 맞는다.

