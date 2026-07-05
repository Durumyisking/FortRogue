// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Characters/FRCharacterDefinition.h"
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "FRCharacterSpriteAnimator.generated.h"

class UPaperFlipbook;
class UPaperFlipbookComponent;

UENUM(BlueprintType)
enum class EFRCharacterAnimState : uint8
{
	Idle,
	Move,
	Shoot,
	Special,
	Hurt
};

/**
 * 전투 캐릭터의 상태별 플립북 전환을 담당하는 연출 컴포넌트입니다.
 * Idle/Move는 반복 재생하고 Shoot/Special/Hurt는 한 번 재생한 뒤 Idle로 복귀합니다.
 * 캐릭터 파사드는 NotifyMoving/NotifyShoot/NotifyHurt만 호출하면 됩니다.
 */
UCLASS(ClassGroup = (FortRogue), meta = (BlueprintSpawnableComponent))
class FORTROGUE_API UFRCharacterSpriteAnimator : public UActorComponent
{
	GENERATED_BODY()

public:
	UFRCharacterSpriteAnimator();

	/** 구동할 플립북 컴포넌트와 애니메이션 세트를 연결하고 Idle 재생을 시작합니다. */
	void Initialize(UPaperFlipbookComponent* InSprite, const FFRCharacterAnimationSet& InAnimationSet, UPaperFlipbook* FallbackIdle);

	/** 이동 입력이 실제 이동으로 이어진 프레임마다 호출하세요. 잠시 호출이 끊기면 Idle로 복귀합니다. */
	void NotifyMoving();

	/** 발사 직후 호출하세요. 특수 공격이면 bSpecial을 켭니다. */
	void NotifyShoot(bool bSpecial);

	/** 피해가 실제로 적용됐을 때 호출하세요. 진행 중인 다른 연출을 중단하고 피격을 재생합니다. */
	void NotifyHurt();

	UFUNCTION(BlueprintPure, Category = "FortRogue|Animation")
	EFRCharacterAnimState GetAnimState() const { return AnimState; }

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void SetAnimState(EFRCharacterAnimState NewState);
	UPaperFlipbook* GetFlipbookForState(EFRCharacterAnimState State) const;
	bool IsOneShotState(EFRCharacterAnimState State) const;

	UFUNCTION()
	void HandleOneShotFinished();

	UPROPERTY()
	TObjectPtr<UPaperFlipbookComponent> Sprite;

	UPROPERTY()
	FFRCharacterAnimationSet AnimationSet;

	UPROPERTY(EditAnywhere, Category = "FortRogue|Animation", meta = (ClampMin = "0.01", ToolTip = "이동 알림이 이 시간 동안 끊기면 Move에서 Idle로 복귀합니다. 초 단위입니다."))
	float MoveIdleRevertSeconds = 0.15f;

	EFRCharacterAnimState AnimState = EFRCharacterAnimState::Idle;
	float LastMovingTime = -1.0f;
};
