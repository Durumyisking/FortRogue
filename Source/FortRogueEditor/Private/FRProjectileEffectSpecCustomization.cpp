// Copyright Epic Games, Inc. All Rights Reserved.

#include "FRProjectileEffectSpecCustomization.h"

#include "DetailWidgetRow.h"
#include "DetailLayoutBuilder.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"
#include "ProjectileEffects/FRProjectileEffect.h"
#include "PropertyHandle.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FRProjectileEffectSpecCustomization"

TSharedRef<IPropertyTypeCustomization> FFRProjectileEffectSpecCustomization::MakeInstance()
{
	return MakeShared<FFRProjectileEffectSpecCustomization>();
}

void FFRProjectileEffectSpecCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	StructHandle = InStructPropertyHandle;
	PropertyUtilities = StructCustomizationUtils.GetPropertyUtilities();

	HeaderRow
		.NameContent()
		[
			InStructPropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(240.0f)
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(this, &FFRProjectileEffectSpecCustomization::GetHeaderText)
		];
}

void FFRProjectileEffectSpecCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	StructHandle = InStructPropertyHandle;
	PropertyUtilities = StructCustomizationUtils.GetPropertyUtilities();
	EffectClassHandle = InStructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FFRProjectileEffectSpec, EffectClass));
	ParametersHandle = InStructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FFRProjectileEffectSpec, Parameters));

	if (EffectClassHandle.IsValid())
	{
		EffectClassHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FFRProjectileEffectSpecCustomization::SyncParametersToEffectClass));
		ChildBuilder.AddProperty(EffectClassHandle.ToSharedRef());
	}

	SyncParametersToEffectClass();

	if (ParametersHandle.IsValid())
	{
		ChildBuilder.AddProperty(ParametersHandle.ToSharedRef());
	}

	ChildBuilder.AddCustomRow(LOCTEXT("ProjectileEffectStatusSearch", "Projectile Effect Parameter Status"))
		.WholeRowContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(this, &FFRProjectileEffectSpecCustomization::GetParameterStatusText)
		];
}

void FFRProjectileEffectSpecCustomization::SyncParametersToEffectClass() const
{
	if (!StructHandle.IsValid() || !ParametersHandle.IsValid())
	{
		return;
	}

	TArray<void*> RawData;
	StructHandle->AccessRawData(RawData);
	if (RawData.Num() <= 0)
	{
		return;
	}

	bool bNeedsChange = false;
	for (void* RawEntry : RawData)
	{
		const FFRProjectileEffectSpec* EffectSpec = static_cast<const FFRProjectileEffectSpec*>(RawEntry);
		if (!EffectSpec)
		{
			continue;
		}

		const UScriptStruct* ExpectedStruct = EffectSpec->GetExpectedParameterStruct();
		if ((!ExpectedStruct && EffectSpec->Parameters.IsValid())
			|| (ExpectedStruct && (!EffectSpec->Parameters.IsValid() || EffectSpec->Parameters.GetScriptStruct() != ExpectedStruct)))
		{
			bNeedsChange = true;
			break;
		}
	}

	if (!bNeedsChange)
	{
		return;
	}

	ParametersHandle->NotifyPreChange();
	for (void* RawEntry : RawData)
	{
		FFRProjectileEffectSpec* EffectSpec = static_cast<FFRProjectileEffectSpec*>(RawEntry);
		if (!EffectSpec)
		{
			continue;
		}

		EffectSpec->EnsureParametersMatchEffectClass();
	}
	ParametersHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
	ParametersHandle->NotifyFinishedChangingProperties();
	if (PropertyUtilities.IsValid())
	{
		PropertyUtilities->ForceRefresh();
	}
}

FText FFRProjectileEffectSpecCustomization::GetHeaderText() const
{
	if (!StructHandle.IsValid())
	{
		return LOCTEXT("NoEffectSpec", "Projectile Effect");
	}

	TArray<void*> RawData;
	StructHandle->AccessRawData(RawData);
	for (void* RawEntry : RawData)
	{
		const FFRProjectileEffectSpec* EffectSpec = static_cast<const FFRProjectileEffectSpec*>(RawEntry);
		if (EffectSpec)
		{
			const FText DisplayName = EffectSpec->GetEffectDisplayName();
			if (!DisplayName.IsEmpty())
			{
				const UScriptStruct* ExpectedStruct = EffectSpec->GetExpectedParameterStruct();
				if (ExpectedStruct)
				{
					return FText::Format(
						LOCTEXT("EffectWithParams", "{0} - {1}"),
						DisplayName,
						ExpectedStruct->GetDisplayNameText());
				}

				return DisplayName;
			}
		}
	}

	return LOCTEXT("NoEffectClass", "None");
}

FText FFRProjectileEffectSpecCustomization::GetParameterStatusText() const
{
	if (!StructHandle.IsValid())
	{
		return LOCTEXT("NoEffectSpecStatus", "Projectile effect 데이터를 읽을 수 없습니다.");
	}

	TArray<void*> RawData;
	StructHandle->AccessRawData(RawData);
	for (void* RawEntry : RawData)
	{
		const FFRProjectileEffectSpec* EffectSpec = static_cast<const FFRProjectileEffectSpec*>(RawEntry);
		if (!EffectSpec)
		{
			continue;
		}

		const UScriptStruct* ExpectedStruct = EffectSpec->GetExpectedParameterStruct();
		if (!EffectSpec->EffectClass)
		{
			return LOCTEXT("NoEffectClassStatus", "EffectClass를 선택하면 Parameters가 해당 효과의 ParamStruct로 자동 설정됩니다.");
		}
		if (!ExpectedStruct)
		{
			return LOCTEXT("NoExpectedParamsStatus", "이 효과는 전용 Parameters 구조체를 요구하지 않습니다.");
		}
		if (!EffectSpec->HasValidParameters())
		{
			return FText::Format(
				LOCTEXT("InvalidParamsStatus", "Parameters가 {0}와 맞지 않습니다. EffectClass를 다시 선택하거나 Details를 새로고침하세요."),
				ExpectedStruct->GetDisplayNameText());
		}

		return FText::Format(
			LOCTEXT("ValidParamsStatus", "Parameters: {0}"),
			ExpectedStruct->GetDisplayNameText());
	}

	return LOCTEXT("MultipleEffectStatus", "여러 Projectile Effect를 동시에 편집 중입니다.");
}

#undef LOCTEXT_NAMESPACE
