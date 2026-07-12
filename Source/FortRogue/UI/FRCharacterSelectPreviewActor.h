// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FRCharacterSelectPreviewActor.generated.h"

class UCameraComponent;
class UFRCharacterDefinition;
class UPaperFlipbookComponent;

/**
 * 캐릭터 선택 화면에서 월드에 캐릭터 스프라이트를 크게 띄워 보여주는 프리뷰 액터입니다.
 * 자체 직교 카메라를 갖고 있어 위젯이 이 액터를 뷰 타겟으로 지정하면
 * 캐릭터가 화면 왼쪽에 보이고 오른쪽은 정보 패널 공간으로 남습니다.
 */
UCLASS()
class FORTROGUE_API AFRCharacterSelectPreviewActor : public AActor
{
	GENERATED_BODY()

public:
	AFRCharacterSelectPreviewActor();

	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintCallable, Category = "FortRogue|Character Select", meta = (ToolTip = "지정한 캐릭터의 대기 애니메이션을 프리뷰 스프라이트에 표시합니다. nullptr이면 스프라이트를 숨깁니다."))
	void ShowCharacter(const UFRCharacterDefinition* CharacterDefinition);

protected:
	UPROPERTY(VisibleAnywhere, Category = "FortRogue|Character Select")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "FortRogue|Character Select", meta = (ToolTip = "선택한 캐릭터의 대기 플립북을 반복 재생하는 스프라이트입니다."))
	TObjectPtr<UPaperFlipbookComponent> PreviewSprite;

	UPROPERTY(VisibleAnywhere, Category = "FortRogue|Character Select", meta = (ToolTip = "프리뷰 전용 직교 카메라입니다. 캐릭터가 화면 왼쪽에 오도록 오른쪽으로 치우쳐 있습니다."))
	TObjectPtr<UCameraComponent> PreviewCamera;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Character Select", meta = (ClampMin = "1.0", ToolTip = "프리뷰 카메라의 직교 폭입니다. 값이 작을수록 캐릭터가 크게 보입니다."))
	float PreviewOrthoWidth = 1500.0f;

	UPROPERTY(EditDefaultsOnly, Category = "FortRogue|Character Select", meta = (ToolTip = "카메라를 스프라이트 기준으로 얼마나 오른쪽(+X)으로 치울지입니다. 값이 클수록 캐릭터가 화면 왼쪽으로 이동합니다."))
	float CameraSideOffset = 330.0f;
};
