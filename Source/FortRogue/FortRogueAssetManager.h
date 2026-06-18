// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "FortRogueAssetManager.generated.h"

UCLASS()
class FORTROGUE_API UFortRogueAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	UFortRogueAssetManager();

	virtual void StartInitialLoading() override;
	virtual void FinishInitialLoading() override;

	void PreloadAsset();
	void PreloadAssetType(const FString AssetType);
	void DumpInitialScanState() const;
	void DumpAssetPathMapElements(const FPrimaryAssetId& InRequestedAssetId) const;

	static UFortRogueAssetManager& Get();
	static bool ShouldLogAssetLoads();
	static bool ShouldDumpInitialScanState();
	static UObject* SynchronousLoadAsset(const FSoftObjectPath& AssetPath);

	template <typename AssetType>
	static AssetType* GetAsset(const TSoftObjectPtr<AssetType>& AssetPointer, bool bKeepInMemory = true);

	template <typename AssetType>
	static AssetType* GetAsset(const FPrimaryAssetId& PrimaryAssetId, bool bKeepInMemory = true);

	template <typename AssetType>
	static TSubclassOf<AssetType> GetSubclass(const TSoftObjectPtr<AssetType>& AssetPointer, bool bKeepInMemory = true);

	template <typename AssetType>
	static TSubclassOf<AssetType> GetSubclass(const FPrimaryAssetId& PrimaryAssetId, bool bKeepInMemory = true);

	void AddLoadedAsset(const UObject* Asset);

	UPROPERTY()
	TSet<TObjectPtr<const UObject>> LoadedAssets;

	FCriticalSection SyncObject;
};

template <typename AssetType>
AssetType* UFortRogueAssetManager::GetAsset(const TSoftObjectPtr<AssetType>& AssetPointer, bool bKeepInMemory)
{
	AssetType* LoadedAsset = nullptr;
	const FSoftObjectPath& AssetPath = AssetPointer.ToSoftObjectPath();
	if (AssetPath.IsValid())
	{
		LoadedAsset = AssetPointer.Get();
		if (!LoadedAsset)
		{
			LoadedAsset = Cast<AssetType>(SynchronousLoadAsset(AssetPath));
			ensureAlwaysMsgf(LoadedAsset, TEXT("Failed to load asset [%s]"), *AssetPointer.ToString());
		}

		if (LoadedAsset && bKeepInMemory)
		{
			Get().AddLoadedAsset(Cast<UObject>(LoadedAsset));
		}
	}

	return LoadedAsset;
}

template <typename AssetType>
AssetType* UFortRogueAssetManager::GetAsset(const FPrimaryAssetId& PrimaryAssetId, bool bKeepInMemory)
{
	AssetType* LoadedAsset = nullptr;
	const FSoftObjectPath AssetPath = Get().GetPrimaryAssetPath(PrimaryAssetId);
	if (AssetPath.IsValid())
	{
		LoadedAsset = Cast<AssetType>(SynchronousLoadAsset(AssetPath));
		if (LoadedAsset && bKeepInMemory)
		{
			Get().AddLoadedAsset(Cast<UObject>(LoadedAsset));
		}
	}

	return LoadedAsset;
}

template <typename AssetType>
TSubclassOf<AssetType> UFortRogueAssetManager::GetSubclass(const TSoftObjectPtr<AssetType>& AssetPointer, bool bKeepInMemory)
{
	TSubclassOf<AssetType> LoadedSubclass = nullptr;
	const FSoftObjectPath& AssetPath = AssetPointer.ToSoftObjectPath();
	if (AssetPath.IsValid())
	{
		LoadedSubclass = AssetPointer.Get();
		if (!LoadedSubclass)
		{
			LoadedSubclass = Cast<UClass>(SynchronousLoadAsset(AssetPath));
			ensureAlwaysMsgf(LoadedSubclass, TEXT("Failed to load asset class [%s]"), *AssetPointer.ToString());
		}

		if (LoadedSubclass && bKeepInMemory)
		{
			Get().AddLoadedAsset(Cast<UObject>(LoadedSubclass));
		}
	}

	return LoadedSubclass;
}

template <typename AssetType>
TSubclassOf<AssetType> UFortRogueAssetManager::GetSubclass(const FPrimaryAssetId& PrimaryAssetId, bool bKeepInMemory)
{
	TSubclassOf<AssetType> LoadedSubclass = nullptr;
	const FSoftObjectPath AssetPath = Get().GetPrimaryAssetPath(PrimaryAssetId);
	if (AssetPath.IsValid())
	{
		LoadedSubclass = Cast<UClass>(SynchronousLoadAsset(AssetPath));
		if (LoadedSubclass && bKeepInMemory)
		{
			Get().AddLoadedAsset(Cast<UObject>(LoadedSubclass));
		}
	}

	return LoadedSubclass;
}
