// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/FRCharacterSelectPreviewActor.h"

#include "Camera/CameraComponent.h"
#include "Characters/FRCharacterDefinition.h"
#include "PaperFlipbook.h"
#include "PaperFlipbookComponent.h"

AFRCharacterSelectPreviewActor::AFRCharacterSelectPreviewActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	PreviewSprite = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("PreviewSprite"));
	PreviewSprite->SetupAttachment(Root);
	PreviewSprite->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewSprite->SetLooping(true);
	PreviewSprite->SetVisibility(false);

	PreviewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PreviewCamera"));
	PreviewCamera->SetupAttachment(Root);
	// 전투 카메라와 같은 축(+Y에서 -Y를 바라봄)을 사용하고, 카메라를 오른쪽으로 치워 캐릭터를 화면 왼쪽에 배치합니다.
	PreviewCamera->SetRelativeLocation(FVector(CameraSideOffset, 900.0f, 40.0f));
	PreviewCamera->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	PreviewCamera->SetProjectionMode(ECameraProjectionMode::Orthographic);
	PreviewCamera->SetOrthoWidth(PreviewOrthoWidth);
}

void AFRCharacterSelectPreviewActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// 에디터/파생 블루프린트에서 조정한 프로퍼티 값을 카메라에 반영합니다.
	if (PreviewCamera)
	{
		PreviewCamera->SetRelativeLocation(FVector(CameraSideOffset, 900.0f, 40.0f));
		PreviewCamera->SetOrthoWidth(PreviewOrthoWidth);
	}
}

void AFRCharacterSelectPreviewActor::ShowCharacter(const UFRCharacterDefinition* CharacterDefinition)
{
	if (!PreviewSprite)
	{
		return;
	}

	UPaperFlipbook* PreviewFlipbook = nullptr;
	if (CharacterDefinition)
	{
		PreviewFlipbook = CharacterDefinition->AnimationSet.Idle ? CharacterDefinition->AnimationSet.Idle.Get() : CharacterDefinition->BodyFlipbook.Get();
	}

	PreviewSprite->SetFlipbook(PreviewFlipbook);
	PreviewSprite->SetVisibility(PreviewFlipbook != nullptr);
	if (PreviewFlipbook)
	{
		PreviewSprite->PlayFromStart();
	}
}
