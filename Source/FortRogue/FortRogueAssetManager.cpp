// Copyright Epic Games, Inc. All Rights Reserved.

#include "FortRogueAssetManager.h"

#include "FortRogue.h"
#include "Engine/AssetManagerSettings.h"

UFortRogueAssetManager::UFortRogueAssetManager()
{
}

void UFortRogueAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	OnCompletedInitialScanDelegate.AddUObject(this, &ThisClass::PreloadAsset);
}

void UFortRogueAssetManager::FinishInitialLoading()
{
	Super::FinishInitialLoading();
}

void UFortRogueAssetManager::PreloadAsset()
{
	if (ShouldDumpInitialScanState())
	{
		DumpInitialScanState();
	}

	OnCompletedInitialScanDelegate.RemoveAll(this);
}

void UFortRogueAssetManager::PreloadAssetType(const FString AssetType)
{
	TArray<FPrimaryAssetId> PrimaryAssetIds;
	Get().GetPrimaryAssetIdList(*AssetType, PrimaryAssetIds);
	for (const FPrimaryAssetId& PrimaryAssetId : PrimaryAssetIds)
	{
		GetAsset<UObject>(PrimaryAssetId, true);
	}
}

UFortRogueAssetManager& UFortRogueAssetManager::Get()
{
	UFortRogueAssetManager* Singleton = Cast<UFortRogueAssetManager>(GEngine->AssetManager);
	if (Singleton)
	{
		return *Singleton;
	}

	UE_LOG(LogFortRogue, Fatal, TEXT("Cannot use FortRogueAssetManager if AssetManagerClassName is not configured."));
	return *NewObject<UFortRogueAssetManager>();
}

bool UFortRogueAssetManager::ShouldLogAssetLoads()
{
	const TCHAR* CommandLineContent = FCommandLine::Get();
	static bool bLogAssetLoads = FParse::Param(CommandLineContent, TEXT("LogAssetLoads"));
	return bLogAssetLoads;
}

bool UFortRogueAssetManager::ShouldDumpInitialScanState()
{
	const TCHAR* CommandLineContent = FCommandLine::Get();
	static bool bDumpInitialScanState = FParse::Param(CommandLineContent, TEXT("LogAssetManagerScan"));
	return bDumpInitialScanState;
}

UObject* UFortRogueAssetManager::SynchronousLoadAsset(const FSoftObjectPath& AssetPath)
{
	if (!AssetPath.IsValid())
	{
		return nullptr;
	}

	TUniquePtr<FScopeLogTime> LogTimePtr;
	if (ShouldLogAssetLoads())
	{
		LogTimePtr = MakeUnique<FScopeLogTime>(*FString::Printf(TEXT("Synchronous loaded asset [%s]"), *AssetPath.ToString()), nullptr, FScopeLogTime::ScopeLog_Seconds);
	}

	if (IsInitialized())
	{
		return GetStreamableManager().LoadSynchronous(AssetPath);
	}

	return AssetPath.TryLoad();
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

void UFortRogueAssetManager::DumpAssetPathMapElements(const FPrimaryAssetId& InRequestedAssetId) const
{
	TArray<FString> AssetPathMapEntries;
	AssetPathMapEntries.Reserve(AssetPathMap.Num());

	for (const TPair<FSoftObjectPath, FPrimaryAssetId>& AssetPathPair : AssetPathMap)
	{
		AssetPathMapEntries.Add(FString::Printf(TEXT("Path=%s -> AssetId=%s"), *AssetPathPair.Key.ToString(), *AssetPathPair.Value.ToString()));
	}

	AssetPathMapEntries.Sort();

	UE_LOG(LogFortRogue, Warning, TEXT("[AssetPathMapDump] ===== Begin Request=%s Count=%d ====="), *InRequestedAssetId.ToString(), AssetPathMapEntries.Num());
	for (const FString& AssetPathMapEntry : AssetPathMapEntries)
	{
		UE_LOG(LogFortRogue, Warning, TEXT("[AssetPathMapDump] %s"), *AssetPathMapEntry);
	}
	UE_LOG(LogFortRogue, Warning, TEXT("[AssetPathMapDump] RequestedPath=%s"), *GetPrimaryAssetPath(InRequestedAssetId).ToString());
	UE_LOG(LogFortRogue, Warning, TEXT("[AssetPathMapDump] RequestedClass=%s"), *GetNameSafe(GetPrimaryAssetObjectClass<UObject>(InRequestedAssetId)));
	UE_LOG(LogFortRogue, Warning, TEXT("[AssetPathMapDump] ===== End Request=%s ====="), *InRequestedAssetId.ToString());
}

void UFortRogueAssetManager::AddLoadedAsset(const UObject* Asset)
{
	if (ensureAlways(Asset))
	{
		FScopeLock Lock(&SyncObject);
		LoadedAssets.Add(Asset);
	}
}
