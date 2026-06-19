// Copyright Epic Games, Inc. All Rights Reserved.

#include "FortRogueAssetManager.h"

#include "FortRogue.h"
#include "Engine/AssetManagerSettings.h"

void UFortRogueAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	OnCompletedInitialScanDelegate.AddUObject(this, &ThisClass::HandleCompletedInitialScan);
}

void UFortRogueAssetManager::HandleCompletedInitialScan()
{
	if (ShouldDumpInitialScanState())
	{
		DumpInitialScanState();
	}

	OnCompletedInitialScanDelegate.RemoveAll(this);
}

bool UFortRogueAssetManager::ShouldDumpInitialScanState()
{
	const TCHAR* CommandLineContent = FCommandLine::Get();
	static bool bDumpInitialScanState = FParse::Param(CommandLineContent, TEXT("LogAssetManagerScan"));
	return bDumpInitialScanState;
}

void UFortRogueAssetManager::DumpInitialScanState() const
{
	const UAssetManagerSettings* AssetManagerSettings = GetDefault<UAssetManagerSettings>();
	if (!AssetManagerSettings)
	{
		UE_LOG(LogFortRogue, Error, TEXT("[AssetManagerScanDump] AssetManagerSettings not found."));
		return;
	}

	UE_LOG(LogFortRogue, Warning, TEXT("[AssetManagerScanDump] ===== Initial Scan Dump Begin ====="));
	UE_LOG(LogFortRogue, Warning, TEXT("[AssetManagerScanDump] ConfiguredTypes=%d ExcludedDirectories=%d"), AssetManagerSettings->PrimaryAssetTypesToScan.Num(), AssetManagerSettings->DirectoriesToExclude.Num());

	for (const FDirectoryPath& DirectoryToExclude : AssetManagerSettings->DirectoriesToExclude)
	{
		UE_LOG(LogFortRogue, Warning, TEXT("[AssetManagerScanDump] ExcludeDirectory=%s"), *DirectoryToExclude.Path);
	}

	for (const FPrimaryAssetTypeInfo& TypeInfo : AssetManagerSettings->PrimaryAssetTypesToScan)
	{
		const FString AssetBaseClassPath = TypeInfo.GetAssetBaseClass().ToString();
		UE_LOG(
			LogFortRogue,
			Warning,
			TEXT("[AssetManagerScanDump] Type=%s BaseClass=%s HasBlueprintClasses=%d IsEditorOnly=%d AssetCount=%d ScanPaths=%d SpecificAssets=%d"),
			*TypeInfo.PrimaryAssetType.ToString(),
			*AssetBaseClassPath,
			TypeInfo.bHasBlueprintClasses ? 1 : 0,
			TypeInfo.bIsEditorOnly ? 1 : 0,
			TypeInfo.NumberOfAssets,
			TypeInfo.GetDirectories().Num(),
			TypeInfo.GetSpecificAssets().Num());

		for (const FDirectoryPath& Directory : TypeInfo.GetDirectories())
		{
			UE_LOG(LogFortRogue, Warning, TEXT("[AssetManagerScanDump] Type=%s Directory=%s"), *TypeInfo.PrimaryAssetType.ToString(), *Directory.Path);
		}

		for (const FSoftObjectPath& SpecificAsset : TypeInfo.GetSpecificAssets())
		{
			UE_LOG(LogFortRogue, Warning, TEXT("[AssetManagerScanDump] Type=%s SpecificAsset=%s"), *TypeInfo.PrimaryAssetType.ToString(), *SpecificAsset.ToString());
		}

		TArray<FPrimaryAssetId> AssetIds;
		GetPrimaryAssetIdList(TypeInfo.PrimaryAssetType, AssetIds);
		UE_LOG(LogFortRogue, Warning, TEXT("[AssetManagerScanDump] Type=%s ResolvedAssetIds=%d"), *TypeInfo.PrimaryAssetType.ToString(), AssetIds.Num());

		for (const FPrimaryAssetId& AssetId : AssetIds)
		{
			const FSoftObjectPath AssetPath = GetPrimaryAssetPath(AssetId);
			UE_LOG(LogFortRogue, Warning, TEXT("[AssetManagerScanDump] Type=%s AssetId=%s Path=%s"), *TypeInfo.PrimaryAssetType.ToString(), *AssetId.ToString(), *AssetPath.ToString());
		}
	}

	UE_LOG(LogFortRogue, Warning, TEXT("[AssetManagerScanDump] ===== Initial Scan Dump End ====="));
}
