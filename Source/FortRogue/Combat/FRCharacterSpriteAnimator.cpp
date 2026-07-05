// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/FRCharacterSpriteAnimator.h"

#include "PaperFlipbook.h"
#include "PaperFlipbookComponent.h"

UFRCharacterSpriteAnimator::UFRCharacterSpriteAnimator()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UFRCharacterSpriteAnimator::Initialize(UPaperFlipbookComponent* InSprite, const FFRCharacterAnimationSet& InAnimationSet, UPaperFlipbook* FallbackIdle)
{
	if (Sprite)
	{
		Sprite->OnFinishedPlaying.RemoveDynamic(this, &UFRCharacterSpriteAnimator::HandleOneShotFinished);
	}

	Sprite = InSprite;
	AnimationSet = InAnimationSet;
	if (!AnimationSet.Idle)
	{
		AnimationSet.Idle = FallbackIdle;
	}

	if (!Sprite)
	{
		SetComponentTickEnabled(false);
		return;
	}

	Sprite->OnFinishedPlaying.AddDynamic(this, &UFRCharacterSpriteAnimator::HandleOneShotFinished);
	AnimState = EFRCharacterAnimState::Hurt; // SetAnimState가 상태 변화를 인식하도록 Idle이 아닌 값에서 시작합니다.
	SetAnimState(EFRCharacterAnimState::Idle);
	SetComponentTickEnabled(true);
}

void UFRCharacterSpriteAnimator::NotifyMoving()
{
	if (!Sprite)
	{
		return;
	}

	LastMovingTime = GetWorld() ? GetWorld()->GetTimeSeconds() : LastMovingTime;
	if (AnimState == EFRCharacterAnimState::Idle)
	{
		SetAnimState(EFRCharacterAnimState::Move);
	}
}

void UFRCharacterSpriteAnimator::NotifyShoot(bool bSpecial)
{
	if (!Sprite)
	{
		return;
	}

	const EFRCharacterAnimState DesiredState = bSpecial && AnimationSet.Special ? EFRCharacterAnimState::Special : EFRCharacterAnimState::Shoot;
	SetAnimState(DesiredState);
}

void UFRCharacterSpriteAnimator::NotifyHurt()
{
	if (!Sprite)
	{
		return;
	}

	// 같은 상태 재진입도 처음부터 다시 재생합니다 (연속 피격).
	if (AnimState == EFRCharacterAnimState::Hurt)
	{
		Sprite->PlayFromStart();
		return;
	}

	SetAnimState(EFRCharacterAnimState::Hurt);
}

void UFRCharacterSpriteAnimator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (AnimState == EFRCharacterAnimState::Move && GetWorld()
		&& GetWorld()->GetTimeSeconds() - LastMovingTime > MoveIdleRevertSeconds)
	{
		SetAnimState(EFRCharacterAnimState::Idle);
	}
}

void UFRCharacterSpriteAnimator::SetAnimState(EFRCharacterAnimState NewState)
{
	if (!Sprite || AnimState == NewState)
	{
		return;
	}

	UPaperFlipbook* Flipbook = GetFlipbookForState(NewState);
	if (!Flipbook)
	{
		// 해당 상태의 플립북이 없으면 Idle 유지 (한 장짜리 캐릭터 등).
		if (NewState == EFRCharacterAnimState::Idle)
		{
			AnimState = NewState;
		}
		return;
	}

	AnimState = NewState;
	Sprite->SetLooping(!IsOneShotState(NewState));
	Sprite->SetFlipbook(Flipbook);
	Sprite->PlayFromStart();
}

UPaperFlipbook* UFRCharacterSpriteAnimator::GetFlipbookForState(EFRCharacterAnimState State) const
{
	switch (State)
	{
	case EFRCharacterAnimState::Idle:
		return AnimationSet.Idle;
	case EFRCharacterAnimState::Move:
		return AnimationSet.Move ? AnimationSet.Move.Get() : AnimationSet.Idle.Get();
	case EFRCharacterAnimState::Shoot:
		return AnimationSet.Shoot;
	case EFRCharacterAnimState::Special:
		return AnimationSet.Special ? AnimationSet.Special.Get() : AnimationSet.Shoot.Get();
	case EFRCharacterAnimState::Hurt:
		return AnimationSet.Hurt;
	default:
		return nullptr;
	}
}

bool UFRCharacterSpriteAnimator::IsOneShotState(EFRCharacterAnimState State) const
{
	return State == EFRCharacterAnimState::Shoot
		|| State == EFRCharacterAnimState::Special
		|| State == EFRCharacterAnimState::Hurt;
}

void UFRCharacterSpriteAnimator::HandleOneShotFinished()
{
	if (IsOneShotState(AnimState))
	{
		SetAnimState(EFRCharacterAnimState::Idle);
	}
}
