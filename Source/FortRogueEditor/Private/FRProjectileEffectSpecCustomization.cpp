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

	bool bChanged = false;
	ParametersHandle->NotifyPreChange();
	for (void* RawEntry : RawData)
	{
		FFRProjectileEffectSpec* EffectSpec = static_cast<FFRProjectileEffectSpec*>(RawEntry);
		if (!EffectSpec)
		{
			continue;
		}

		const UScriptStruct* ExpectedStruct = EffectSpec->GetExpectedParameterStruct();
		if (!ExpectedStruct)
		{
			if (EffectSpec->Parameters.IsValid())
			{
				EffectSpec->Parameters.Reset();
				bChanged = true;
			}
			continue;
		}

		if (!EffectSpec->Parameters.IsValid() || EffectSpec->Parameters.GetScriptStruct() != ExpectedStruct)
		{
			EffectSpec->Parameters.InitializeAs(ExpectedStruct);
			bChanged = true;
		}
	}

	if (bChanged)
	{
		ParametersHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
		ParametersHandle->NotifyFinishedChangingProperties();
		if (PropertyUtilities.IsValid())
		{
			PropertyUtilities->ForceRefresh();
		}
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
		if (EffectSpec && EffectSpec->EffectClass)
		{
			return FText::FromString(EffectSpec->EffectClass->GetName());
		}
	}

	return LOCTEXT("NoEffectClass", "None");
}

#undef LOCTEXT_NAMESPACE
