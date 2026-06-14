// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "Combat/FortRogueBattleCharacter.h"
#include "Combat/FortRogueDestructibleTerrain.h"
#include "Combat/FortRogueProjectile.h"
#include "Combat/FortRogueTerrainMapDefinition.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FortRogueGameMode.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFortRogueTerrainMapDefinitionEditTest, "FortRogue.Terrain.MapDefinition.Edits", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFortRogueTerrainMapDefinitionEditTest::RunTest(const FString& Parameters)
{
	UFortRogueTerrainMapDefinition* Map = NewObject<UFortRogueTerrainMapDefinition>();
	TestNotNull(TEXT("Map asset object is created"), Map);
	TestEqual(TEXT("Default map width uses 4x Fortress screen pixels"), Map->CellsX, 1280);
	TestEqual(TEXT("Default map height uses 4x Fortress screen pixels"), Map->CellsZ, 960);
	TestEqual(TEXT("Default map uses one-unit world cells"), Map->CellSize, 1.0f);
	const float HalfMapWidth = Map->CellsX * Map->CellSize * 0.5f;
	TestTrue(TEXT("Default player spawn is inside the map width"), FMath::Abs(Map->PlayerSpawnLocal.X) < HalfMapWidth);
	TestTrue(TEXT("Default enemy spawn is inside the map width"), FMath::Abs(Map->EnemySpawnLocal.X) < HalfMapWidth);
	TestTrue(TEXT("Default spawns start above the map"), Map->PlayerSpawnLocal.Z > Map->CellsZ * Map->CellSize && Map->EnemySpawnLocal.Z > Map->CellsZ * Map->CellSize);

	Map->Resize(8, 4);
	Map->Clear(false);
	TestEqual(TEXT("Resize updates SolidMask size"), Map->SolidMask.Num(), 32);
	TestEqual(TEXT("Resize updates TextureLayerMask size"), Map->TextureLayerMask.Num(), 32);

	Map->FillRect(2, 1, 4, 2, true);
	TestEqual(TEXT("Filled rect sets minimum cell solid"), Map->SolidMask[Map->ToIndex(2, 1)], static_cast<uint8>(1));
	TestEqual(TEXT("Filled rect sets maximum cell solid"), Map->SolidMask[Map->ToIndex(4, 2)], static_cast<uint8>(1));
	TestEqual(TEXT("Outside rect stays empty"), Map->SolidMask[Map->ToIndex(1, 1)], static_cast<uint8>(0));

	Map->ApplyTextureRect(2, 1, 4, 2, 3);
	TestEqual(TEXT("Texture rect paints existing solid cell"), Map->TextureLayerMask[Map->ToIndex(3, 1)], static_cast<uint8>(3));
	TestEqual(TEXT("Texture rect does not paint empty cell"), Map->TextureLayerMask[Map->ToIndex(1, 1)], static_cast<uint8>(0));

	Map->ApplyCircle(3, 1, 1, false);
	TestEqual(TEXT("Erase circle clears center solid"), Map->SolidMask[Map->ToIndex(3, 1)], static_cast<uint8>(0));
	TestEqual(TEXT("Erase circle clears center texture layer"), Map->TextureLayerMask[Map->ToIndex(3, 1)], static_cast<uint8>(0));

	Map->ApplyCircle(6, 2, 1, true);
	Map->ApplyTextureCircle(6, 2, 1, 5);
	TestEqual(TEXT("Texture circle paints solid center"), Map->TextureLayerMask[Map->ToIndex(6, 2)], static_cast<uint8>(5));

	UFortRogueTerrainMapDefinition* ResizedMap = NewObject<UFortRogueTerrainMapDefinition>();
	TestNotNull(TEXT("Resample resize map asset object is created"), ResizedMap);
	ResizedMap->Resize(4, 2);
	ResizedMap->CellSize = 10.0f;
	ResizedMap->Clear(false);
	ResizedMap->FillRect(1, 0, 1, 1, true);
	ResizedMap->ApplyTextureRect(1, 0, 1, 1, 6);
	ResizedMap->PlayerSpawnLocal = FVector(-10.0f, 0.0f, 35.0f);
	ResizedMap->EnemySpawnLocal = FVector(10.0f, 0.0f, 35.0f);
	ResizedMap->ResizeResampled(8, 4);
	TestEqual(TEXT("Resample resize updates map width"), ResizedMap->CellsX, 8);
	TestEqual(TEXT("Resample resize updates map height"), ResizedMap->CellsZ, 4);
	TestEqual(TEXT("Resample resize preserves scaled solid terrain"), ResizedMap->SolidMask[ResizedMap->ToIndex(2, 1)], static_cast<uint8>(1));
	TestEqual(TEXT("Resample resize preserves scaled texture layer"), ResizedMap->TextureLayerMask[ResizedMap->ToIndex(2, 1)], static_cast<uint8>(6));
	TestEqual(TEXT("Resample resize keeps empty areas empty"), ResizedMap->SolidMask[ResizedMap->ToIndex(6, 1)], static_cast<uint8>(0));
	TestEqual(TEXT("Resample resize preserves player spawn horizontal ratio"), ResizedMap->PlayerSpawnLocal.X, -20.0);
	TestEqual(TEXT("Resample resize preserves enemy spawn horizontal ratio"), ResizedMap->EnemySpawnLocal.X, 20.0);
	TestEqual(TEXT("Resample resize preserves spawn clearance above the map"), ResizedMap->PlayerSpawnLocal.Z, 55.0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFortRogueTerrainMapDefinitionImportTest, "FortRogue.Terrain.MapDefinition.ImportTextureMask", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFortRogueTerrainMapDefinitionImportTest::RunTest(const FString& Parameters)
{
	UTexture2D* Texture = UTexture2D::CreateTransient(4, 3, PF_B8G8R8A8);
	TestNotNull(TEXT("Transient mask texture is created"), Texture);
	if (!Texture || !Texture->GetPlatformData() || Texture->GetPlatformData()->Mips.Num() == 0)
	{
		return false;
	}

	FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
	void* TextureData = Mip.BulkData.Lock(LOCK_READ_WRITE);
	TestNotNull(TEXT("Transient mask texture mip locks"), TextureData);
	if (!TextureData)
	{
		Mip.BulkData.Unlock();
		return false;
	}

	FColor* Pixels = static_cast<FColor*>(TextureData);
	for (int32 Index = 0; Index < 12; ++Index)
	{
		Pixels[Index] = FColor(0, 0, 0, 0);
	}

	Pixels[0] = FColor(255, 255, 255, 255);
	Pixels[5] = FColor(255, 255, 255, 255);
	Pixels[11] = FColor(255, 255, 255, 255);
	Mip.BulkData.Unlock();

	UFortRogueTerrainMapDefinition* Map = NewObject<UFortRogueTerrainMapDefinition>();
	TestNotNull(TEXT("Map asset object is created"), Map);
	const bool bImported = Map->ImportSolidMaskFromTexture(Texture, true, 0.5f, 7);
	TestTrue(TEXT("Texture alpha mask imports"), bImported);
	TestEqual(TEXT("Import updates map width"), Map->CellsX, 4);
	TestEqual(TEXT("Import updates map height"), Map->CellsZ, 3);

	TestEqual(TEXT("Top-left source pixel maps to top map cell"), Map->SolidMask[Map->ToIndex(0, 2)], static_cast<uint8>(1));
	TestEqual(TEXT("Middle source pixel maps to middle map cell"), Map->SolidMask[Map->ToIndex(1, 1)], static_cast<uint8>(1));
	TestEqual(TEXT("Bottom-right source pixel maps to bottom map cell"), Map->SolidMask[Map->ToIndex(3, 0)], static_cast<uint8>(1));
	TestEqual(TEXT("Imported solid cells receive requested texture layer"), Map->TextureLayerMask[Map->ToIndex(1, 1)], static_cast<uint8>(7));
	TestEqual(TEXT("Transparent cells remain empty"), Map->SolidMask[Map->ToIndex(2, 2)], static_cast<uint8>(0));

	UFortRogueTerrainMapDefinition* ColorMap = NewObject<UFortRogueTerrainMapDefinition>();
	TestNotNull(TEXT("Color import map asset object is created"), ColorMap);
	const bool bColorImported = ColorMap->ImportSolidMaskFromTextureByColor(Texture, FLinearColor::White, 0.01f, 2);
	TestTrue(TEXT("Texture color mask imports"), bColorImported);
	TestEqual(TEXT("Color import updates map width"), ColorMap->CellsX, 4);
	TestEqual(TEXT("Color import updates map height"), ColorMap->CellsZ, 3);
	TestEqual(TEXT("Color-matched source pixel maps to solid terrain"), ColorMap->SolidMask[ColorMap->ToIndex(0, 2)], static_cast<uint8>(1));
	TestEqual(TEXT("Color-matched solid cells receive requested texture layer"), ColorMap->TextureLayerMask[ColorMap->ToIndex(1, 1)], static_cast<uint8>(2));
	TestEqual(TEXT("Nonmatching color cells remain empty"), ColorMap->SolidMask[ColorMap->ToIndex(2, 2)], static_cast<uint8>(0));

	UFortRogueTerrainMapDefinition* ResampledMap = NewObject<UFortRogueTerrainMapDefinition>();
	TestNotNull(TEXT("Resampled import map asset object is created"), ResampledMap);
	ResampledMap->Resize(2, 2);
	const bool bResampledImported = ResampledMap->ImportSolidMaskFromTextureByColor(Texture, FLinearColor::White, 0.01f, 4, false);
	TestTrue(TEXT("Texture color mask imports while keeping current map size"), bResampledImported);
	TestEqual(TEXT("Keep-size import preserves map width"), ResampledMap->CellsX, 2);
	TestEqual(TEXT("Keep-size import preserves map height"), ResampledMap->CellsZ, 2);
	TestEqual(TEXT("Keep-size import samples matching source color"), ResampledMap->SolidMask[ResampledMap->ToIndex(1, 0)], static_cast<uint8>(1));
	TestEqual(TEXT("Keep-size import assigns requested texture layer"), ResampledMap->TextureLayerMask[ResampledMap->ToIndex(1, 0)], static_cast<uint8>(4));

	UFortRogueTerrainMapDefinition* HighResolutionMap = NewObject<UFortRogueTerrainMapDefinition>();
	TestNotNull(TEXT("High-resolution import map asset object is created"), HighResolutionMap);
	const bool bHighResolutionImported = HighResolutionMap->ImportSolidMaskFromTexture(Texture, true, 0.5f, 8, false);
	TestTrue(TEXT("Small source texture imports into the current high-resolution map size"), bHighResolutionImported);
	TestEqual(TEXT("High-resolution keep-size import preserves default map width"), HighResolutionMap->CellsX, 1280);
	TestEqual(TEXT("High-resolution keep-size import preserves default map height"), HighResolutionMap->CellsZ, 960);
	TestEqual(TEXT("High-resolution import samples top-left source pixel"), HighResolutionMap->SolidMask[HighResolutionMap->ToIndex(100, 959)], static_cast<uint8>(1));
	TestEqual(TEXT("High-resolution import samples middle source pixel"), HighResolutionMap->SolidMask[HighResolutionMap->ToIndex(400, 479)], static_cast<uint8>(1));
	TestEqual(TEXT("High-resolution import samples bottom-right source pixel"), HighResolutionMap->SolidMask[HighResolutionMap->ToIndex(1100, 159)], static_cast<uint8>(1));
	TestEqual(TEXT("High-resolution import preserves empty source areas"), HighResolutionMap->SolidMask[HighResolutionMap->ToIndex(800, 959)], static_cast<uint8>(0));
	TestEqual(TEXT("High-resolution imported solid cells receive requested texture layer"), HighResolutionMap->TextureLayerMask[HighResolutionMap->ToIndex(400, 479)], static_cast<uint8>(8));

	UTexture2D* EdgeTexture = UTexture2D::CreateTransient(2, 1, PF_B8G8R8A8);
	TestNotNull(TEXT("Transient edge texture is created"), EdgeTexture);
	if (!EdgeTexture || !EdgeTexture->GetPlatformData() || EdgeTexture->GetPlatformData()->Mips.Num() == 0)
	{
		return false;
	}

	FTexture2DMipMap& EdgeMip = EdgeTexture->GetPlatformData()->Mips[0];
	void* EdgeTextureData = EdgeMip.BulkData.Lock(LOCK_READ_WRITE);
	TestNotNull(TEXT("Transient edge texture mip locks"), EdgeTextureData);
	if (!EdgeTextureData)
	{
		EdgeMip.BulkData.Unlock();
		return false;
	}

	FColor* EdgePixels = static_cast<FColor*>(EdgeTextureData);
	EdgePixels[0] = FColor(0, 0, 0, 0);
	EdgePixels[1] = FColor(255, 255, 255, 255);
	EdgeMip.BulkData.Unlock();

	UFortRogueTerrainMapDefinition* UpscaledEdgeMap = NewObject<UFortRogueTerrainMapDefinition>();
	TestNotNull(TEXT("Upscaled edge import map asset object is created"), UpscaledEdgeMap);
	UpscaledEdgeMap->Resize(4, 1);
	const bool bUpscaledEdgeImported = UpscaledEdgeMap->ImportSolidMaskFromTexture(EdgeTexture, true, 0.2f, 3, false);
	TestTrue(TEXT("Low-resolution edge mask imports into a larger map"), bUpscaledEdgeImported);
	TestEqual(TEXT("Upscaled import keeps the first edge cell empty"), UpscaledEdgeMap->SolidMask[UpscaledEdgeMap->ToIndex(0, 0)], static_cast<uint8>(0));
	TestEqual(TEXT("Upscaled import interpolates the edge instead of nearest-only sampling"), UpscaledEdgeMap->SolidMask[UpscaledEdgeMap->ToIndex(1, 0)], static_cast<uint8>(1));
	TestEqual(TEXT("Upscaled import keeps the solid side filled"), UpscaledEdgeMap->SolidMask[UpscaledEdgeMap->ToIndex(2, 0)], static_cast<uint8>(1));
	TestEqual(TEXT("Upscaled import assigns the requested layer on interpolated cells"), UpscaledEdgeMap->TextureLayerMask[UpscaledEdgeMap->ToIndex(1, 0)], static_cast<uint8>(3));

	UFortRogueTerrainMapDefinition* RegionMap = NewObject<UFortRogueTerrainMapDefinition>();
	TestNotNull(TEXT("Region import map asset object is created"), RegionMap);
	const bool bRegionImported = RegionMap->ImportSolidMaskFromTextureRegion(Texture, 1, 1, 2, 2, true, 0.5f, 5);
	TestTrue(TEXT("Texture region alpha mask imports"), bRegionImported);
	TestEqual(TEXT("Region import updates map width to source region width"), RegionMap->CellsX, 2);
	TestEqual(TEXT("Region import updates map height to source region height"), RegionMap->CellsZ, 2);
	TestEqual(TEXT("Region top-left source pixel maps to top map cell"), RegionMap->SolidMask[RegionMap->ToIndex(0, 1)], static_cast<uint8>(1));
	TestEqual(TEXT("Region nonmatching source pixel remains empty"), RegionMap->SolidMask[RegionMap->ToIndex(1, 1)], static_cast<uint8>(0));
	TestEqual(TEXT("Region imported solid cell receives requested texture layer"), RegionMap->TextureLayerMask[RegionMap->ToIndex(0, 1)], static_cast<uint8>(5));

	UFortRogueTerrainMapDefinition* RegionColorMap = NewObject<UFortRogueTerrainMapDefinition>();
	TestNotNull(TEXT("Region color import map asset object is created"), RegionColorMap);
	RegionColorMap->Resize(2, 2);
	const bool bRegionColorImported = RegionColorMap->ImportSolidMaskFromTextureRegionByColor(Texture, 1, 1, 2, 2, FLinearColor::White, 0.01f, 6, false);
	TestTrue(TEXT("Texture region color mask imports while keeping current map size"), bRegionColorImported);
	TestEqual(TEXT("Region color import preserves map width"), RegionColorMap->CellsX, 2);
	TestEqual(TEXT("Region color import preserves map height"), RegionColorMap->CellsZ, 2);
	TestEqual(TEXT("Region color import samples matching source color"), RegionColorMap->SolidMask[RegionColorMap->ToIndex(0, 1)], static_cast<uint8>(1));
	TestEqual(TEXT("Region color import assigns requested texture layer"), RegionColorMap->TextureLayerMask[RegionColorMap->ToIndex(0, 1)], static_cast<uint8>(6));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFortRogueTerrainGameModeMapDefinitionTest, "FortRogue.Terrain.GameMode.UsesMapDefinition", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFortRogueTerrainGameModeMapDefinitionTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>(GEngine);
	TestNotNull(TEXT("Transient game instance is created"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("Transient test world is created"), World);
	if (!World)
	{
		return false;
	}

	World->SetShouldTick(false);
	World->AddToRoot();

	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.OwningGameInstance = GameInstance;
	World->SetGameInstance(GameInstance);
	WorldContext.SetCurrentWorld(World);
	GameInstance->Init();

	auto CleanupWorld = [&World, &GameInstance]()
	{
		if (World->HasBegunPlay())
		{
			World->BeginTearingDown();
			World->EndPlay(EEndPlayReason::Quit);
		}

		World->RemoveFromRoot();
		GameInstance->Shutdown();
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	};

	UFortRogueTerrainMapDefinition* Map = NewObject<UFortRogueTerrainMapDefinition>();
	Map->Resize(20, 6);
	Map->CellSize = 10.0f;
	Map->Clear(false);
	Map->FillRect(0, 0, 19, 0, true);
	Map->PlayerSpawnLocal = FVector(-40.0f, 0.0f, 140.0f);
	Map->EnemySpawnLocal = FVector(40.0f, 0.0f, 140.0f);

	FURL URL;
	World->SetGameMode(URL);
	AFortRogueGameMode* GameMode = World->GetAuthGameMode<AFortRogueGameMode>();
	TestNotNull(TEXT("FortRogue game mode is created"), GameMode);
	if (!GameMode)
	{
		CleanupWorld();
		return false;
	}

	FObjectProperty* TerrainMapProperty = FindFProperty<FObjectProperty>(GameMode->GetClass(), TEXT("TerrainMapDefinition"));
	TestNotNull(TEXT("Game mode exposes a terrain map definition property"), TerrainMapProperty);
	if (!TerrainMapProperty)
	{
		CleanupWorld();
		return false;
	}
	TerrainMapProperty->SetObjectPropertyValue_InContainer(GameMode, Map);

	World->InitializeActorsForPlay(URL);
	World->BeginPlay();

	AFortRogueDestructibleTerrain* SpawnedTerrain = nullptr;
	for (TActorIterator<AFortRogueDestructibleTerrain> It(World); It; ++It)
	{
		if (It->MapDefinition == Map)
		{
			SpawnedTerrain = *It;
			break;
		}
	}

	TestNotNull(TEXT("Game mode spawns destructible terrain using the configured map definition"), SpawnedTerrain);
	if (SpawnedTerrain)
	{
		TestEqual(TEXT("Spawned terrain width follows the configured map definition"), SpawnedTerrain->Width, 200.0f);
		TestEqual(TEXT("Spawned terrain height follows the configured map definition"), SpawnedTerrain->Height, 60.0f);
		TestNotNull(TEXT("Spawned terrain creates its runtime texture"), SpawnedTerrain->GetRuntimeTerrainTexture());
	}
	TestNotNull(TEXT("Game mode spawns the player character"), GameMode->GetPlayerCharacter());
	TestNotNull(TEXT("Game mode spawns the enemy character"), GameMode->GetEnemyCharacter());

	ACameraActor* BattleCamera = nullptr;
	for (TActorIterator<ACameraActor> It(World); It; ++It)
	{
		BattleCamera = *It;
		break;
	}

	TestNotNull(TEXT("Game mode spawns a battle camera"), BattleCamera);
	if (BattleCamera)
	{
		TestEqual(TEXT("Battle camera faces the X/Z gameplay plane without requiring terrain rotation"), BattleCamera->GetActorRotation(), FRotator(0.0f, 90.0f, 0.0f));
		TestNotNull(TEXT("Battle camera has a camera component"), BattleCamera->GetCameraComponent());
		if (BattleCamera->GetCameraComponent())
		{
			TestEqual(TEXT("Battle camera uses orthographic projection"), BattleCamera->GetCameraComponent()->ProjectionMode, ECameraProjectionMode::Orthographic);
		}
	}

	CleanupWorld();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFortRogueDestructibleTerrainRuntimeTest, "FortRogue.Terrain.DestructibleTerrain.RuntimeQueries", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFortRogueDestructibleTerrainRuntimeTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>(GEngine);
	TestNotNull(TEXT("Transient game instance is created"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("Transient test world is created"), World);
	if (!World)
	{
		return false;
	}

	World->SetShouldTick(false);
	World->AddToRoot();

	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.OwningGameInstance = GameInstance;
	World->SetGameInstance(GameInstance);
	WorldContext.SetCurrentWorld(World);
	GameInstance->Init();

	auto CleanupWorld = [&World, &GameInstance]()
	{
		if (World->HasBegunPlay())
		{
			World->BeginTearingDown();
			World->EndPlay(EEndPlayReason::Quit);
		}

		World->RemoveFromRoot();
		GameInstance->Shutdown();
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	};

	UFortRogueTerrainMapDefinition* Map = NewObject<UFortRogueTerrainMapDefinition>();
	Map->Resize(20, 6);
	Map->CellSize = 10.0f;
	Map->Clear(false);
	Map->FillRect(0, 0, 19, 0, true);
	Map->FillRect(3, 1, 3, 1, true);
	Map->FillRect(4, 1, 4, 2, true);
	Map->FillRect(7, 1, 7, 3, true);
	Map->FillRect(10, 0, 10, 5, true);
	Map->PlayerSpawnLocal = FVector(-999.0f, 0.0f, 5.0f);
	Map->EnemySpawnLocal = FVector(999.0f, 0.0f, 5.0f);

	UTexture2D* LayerTexture = nullptr;
#if WITH_EDITOR
	LayerTexture = NewObject<UTexture2D>();
#else
	LayerTexture = UTexture2D::CreateTransient(2, 2, PF_B8G8R8A8);
#endif
	TestNotNull(TEXT("Layer source texture is created"), LayerTexture);
	if (LayerTexture)
	{
#if WITH_EDITOR
		const uint8 SourcePixels[] = {
			0, 0, 255, 255,
			0, 255, 0, 255,
			255, 0, 0, 255,
			0, 255, 255, 255
		};
		LayerTexture->Source.Init(2, 2, 1, 1, TSF_BGRA8, SourcePixels);
#else
		FTexture2DMipMap& LayerMip = LayerTexture->GetPlatformData()->Mips[0];
		void* LayerData = LayerMip.BulkData.Lock(LOCK_READ_WRITE);
		TestNotNull(TEXT("Layer source texture mip locks"), LayerData);
		if (LayerData)
		{
			FColor* LayerPixels = static_cast<FColor*>(LayerData);
			LayerPixels[0] = FColor::Red;
			LayerPixels[1] = FColor::Green;
			LayerPixels[2] = FColor::Blue;
			LayerPixels[3] = FColor::Yellow;
		}
		LayerMip.BulkData.Unlock();
#endif
		Map->SetTextureLayer(0, LayerTexture);
	}

	UFortRogueTerrainMapDefinition* OverlappingMap = NewObject<UFortRogueTerrainMapDefinition>();
	OverlappingMap->Resize(20, 6);
	OverlappingMap->CellSize = 10.0f;
	OverlappingMap->Clear(false);
	OverlappingMap->FillRect(0, 0, 19, 0, true);

	FActorSpawnParameters SpawnParams;
	AFortRogueDestructibleTerrain* Terrain = World->SpawnActor<AFortRogueDestructibleTerrain>(AFortRogueDestructibleTerrain::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Terrain actor is spawned"), Terrain);
	if (!Terrain)
	{
		CleanupWorld();
		return false;
	}

	TestEqual(TEXT("Fallback terrain uses high-resolution cells"), Terrain->CellSize, 1.0f);
	UStaticMeshComponent* TexturePlane = Cast<UStaticMeshComponent>(Terrain->GetDefaultSubobjectByName(TEXT("TerrainTexturePlane")));
	TestNotNull(TEXT("Terrain texture plane exists"), TexturePlane);
	if (TexturePlane)
	{
		TestEqual(TEXT("Terrain texture plane is aligned to the X/Z gameplay plane"), TexturePlane->GetRelativeRotation(), FRotator(0.0f, 0.0f, 90.0f));
		TestEqual(TEXT("Terrain texture plane does not use Unreal collision"), TexturePlane->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
	}
	UInstancedStaticMeshComponent* TextureTerrainInstances = Cast<UInstancedStaticMeshComponent>(Terrain->GetDefaultSubobjectByName(TEXT("TerrainInstances")));
	if (TextureTerrainInstances)
	{
		TestEqual(TEXT("Terrain instances do not use Unreal collision"), TextureTerrainInstances->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
	}

	Terrain->MapDefinition = Map;

	AFortRogueDestructibleTerrain* OverlappingTerrain = World->SpawnActor<AFortRogueDestructibleTerrain>(AFortRogueDestructibleTerrain::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Overlapping terrain actor is spawned"), OverlappingTerrain);
	if (OverlappingTerrain)
	{
		OverlappingTerrain->MapDefinition = OverlappingMap;
	}

	FURL URL;
	World->SetGameMode(URL);
	World->InitializeActorsForPlay(URL);
	World->BeginPlay();

	UTexture2D* RuntimeTexture = Terrain->GetRuntimeTerrainTexture();
	TestNotNull(TEXT("Runtime terrain texture is created"), RuntimeTexture);
	if (RuntimeTexture)
	{
		TestTrue(TEXT("Runtime terrain texture uses bilinear filtering to avoid chunky upscale"), RuntimeTexture->Filter == TF_Bilinear);
		TestTrue(TEXT("Runtime terrain texture does not stream"), RuntimeTexture->NeverStream);
		FTexture2DMipMap& RuntimeMip = RuntimeTexture->GetPlatformData()->Mips[0];
		const FColor* RuntimePixels = static_cast<const FColor*>(RuntimeMip.BulkData.LockReadOnly());
		TestNotNull(TEXT("Runtime terrain texture mip locks"), RuntimePixels);
		if (RuntimePixels)
		{
			TestEqual(TEXT("Layer texture maps across bottom-left terrain cells"), RuntimePixels[5 * 20 + 2], FColor::Blue);
			TestEqual(TEXT("Layer texture maps across bottom-right terrain cells"), RuntimePixels[5 * 20 + 17], FColor::Yellow);
			TestEqual(TEXT("Layer texture maps across top-right terrain cells"), RuntimePixels[0 * 20 + 10], FColor::Green);
		}
		RuntimeMip.BulkData.Unlock();
	}
	if (TexturePlane)
	{
		TestEqual(TEXT("Terrain texture plane width follows the map definition"), static_cast<float>(TexturePlane->GetRelativeScale3D().X), 2.0f);
		TestEqual(TEXT("Terrain texture plane height follows the map definition"), static_cast<float>(TexturePlane->GetRelativeScale3D().Y), 0.6f);
		if (TexturePlane->IsVisible() && TextureTerrainInstances)
		{
			TestEqual(TEXT("Texture-rendered terrain does not build fallback instances"), TextureTerrainInstances->GetInstanceCount(), 0);
		}
	}

	const FVector PlayerSpawn = Terrain->GetPlayerSpawnWorldLocation();
	const FVector EnemySpawn = Terrain->GetEnemySpawnWorldLocation();
	TestTrue(TEXT("Player spawn is clamped inside terrain width"), PlayerSpawn.X > -100.0f && PlayerSpawn.X < 100.0f);
	TestTrue(TEXT("Enemy spawn is clamped inside terrain width"), EnemySpawn.X > -100.0f && EnemySpawn.X < 100.0f);
	TestTrue(TEXT("Spawns below map top are lifted above terrain height"), PlayerSpawn.Z > 60.0f && EnemySpawn.Z > 60.0f);
	TestEqual(TEXT("Player spawn height resolves from the terrain surface at its clamped X"), PlayerSpawn.Z, 90.0);
	TestEqual(TEXT("Enemy spawn height resolves from the terrain surface at its clamped X"), EnemySpawn.Z, 90.0);

	TestTrue(TEXT("Bottom cell is solid in world space"), Terrain->IsSolidAtWorldLocation(FVector(-15.0f, 0.0f, 5.0f)));
	TestFalse(TEXT("Empty air cell is not solid in world space"), Terrain->IsSolidAtWorldLocation(FVector(-15.0f, 0.0f, 15.0f)));
	TestFalse(TEXT("Carve circle outside the terrain reports no change"), Terrain->CarveCircle(FVector(500.0f, 0.0f, 5.0f), 20.0f));
	TestTrue(TEXT("Out-of-bounds carve leaves terrain unchanged"), Terrain->IsSolidAtWorldLocation(FVector(-15.0f, 0.0f, 5.0f)));

	float SurfaceZ = 0.0f;
	TestTrue(TEXT("Surface can be found above a solid column"), Terrain->FindSurfaceZAtWorldX(-15.0f, 30.0f, 40.0f, SurfaceZ));
	TestEqual(TEXT("Surface Z matches the top of the solid cell"), SurfaceZ, 10.0f);
	TestFalse(TEXT("Buried wall cells are not treated as walkable surfaces"), Terrain->FindSurfaceZAtWorldX(5.0f, 44.0f, 90.0f, SurfaceZ));
	TestTrue(TEXT("Exposed wall top is still found from above"), Terrain->FindSurfaceZAtWorldX(5.0f, 65.0f, 90.0f, SurfaceZ));
	TestEqual(TEXT("Exposed wall top surface matches the column height"), SurfaceZ, 60.0f);

	AFortRogueBattleCharacter* Character = World->SpawnActor<AFortRogueBattleCharacter>(AFortRogueBattleCharacter::StaticClass(), FVector(-5.0f, 0.0f, 55.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Battle character is spawned"), Character);
	if (Character)
	{
		if (UPrimitiveComponent* Body = Cast<UPrimitiveComponent>(Character->GetDefaultSubobjectByName(TEXT("Body"))))
		{
			TestEqual(TEXT("Battle character body does not use Unreal collision"), Body->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
		}
		Character->SetTerrain(Terrain);
		Character->BeginTurn();
		Character->MoveHorizontal(1.0f, 0.05f);
		TestTrue(TEXT("Battle character advances until a steep terrain wall blocks it"), Character->GetActorLocation().X > -5.0f);
		TestTrue(TEXT("Battle character does not tunnel into the steep terrain wall"), Character->GetActorLocation().X < 0.0f);
	}

	auto SetFloatProperty = [](UObject* Object, const FName PropertyName, float Value)
	{
		if (FFloatProperty* Property = FindFProperty<FFloatProperty>(Object->GetClass(), PropertyName))
		{
			Property->SetPropertyValue_InContainer(Object, Value);
		}
	};

	auto GetFloatProperty = [](UObject* Object, const FName PropertyName, float FallbackValue)
	{
		if (FFloatProperty* Property = FindFProperty<FFloatProperty>(Object->GetClass(), PropertyName))
		{
			return Property->GetPropertyValue_InContainer(Object);
		}

		return FallbackValue;
	};

	AFortRogueBattleCharacter* LateBoundCharacter = World->SpawnActor<AFortRogueBattleCharacter>(AFortRogueBattleCharacter::StaticClass(), FVector(-15.0f, 0.0f, 200.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Late-bound terrain battle character is spawned"), LateBoundCharacter);
	if (LateBoundCharacter)
	{
		LateBoundCharacter->SetActorLocation(FVector(-15.0f, 0.0f, 200.0f));
		LateBoundCharacter->SetTerrain(Terrain);
		const float FootOffsetZ = GetFloatProperty(LateBoundCharacter, TEXT("FootOffsetZ"), 45.0f);
		TestEqual(TEXT("Assigning terrain after BeginPlay snaps character feet onto the highest footprint surface"), static_cast<float>(LateBoundCharacter->GetActorLocation().Z), 60.0f + FootOffsetZ);
	}

	AFortRogueBattleCharacter* RampCharacter = World->SpawnActor<AFortRogueBattleCharacter>(AFortRogueBattleCharacter::StaticClass(), FVector(-95.0f, 0.0f, 55.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Ramp battle character is spawned"), RampCharacter);
	if (RampCharacter)
	{
		RampCharacter->SetTerrain(Terrain);
		SetFloatProperty(RampCharacter, TEXT("FootProbeHalfWidth"), 0.0f);
		SetFloatProperty(RampCharacter, TEXT("BodySlopeProbeHalfWidth"), 5.0f);
		RampCharacter->SetActorLocation(FVector(-95.0f, 0.0f, 55.0f));
		RampCharacter->BeginTurn();
		RampCharacter->MoveHorizontal(1.0f, 0.12f);
		TestTrue(TEXT("Battle character climbs a traversable gentle slope"), RampCharacter->GetActorLocation().X > -70.0f && RampCharacter->GetActorLocation().Z >= 65.0f);
		UStaticMeshComponent* RampBody = Cast<UStaticMeshComponent>(RampCharacter->GetDefaultSubobjectByName(TEXT("Body")));
		TestNotNull(TEXT("Ramp battle character body exists"), RampBody);
		if (RampBody)
		{
			TestTrue(TEXT("Battle character body visually aligns to terrain slope"), RampBody->GetRelativeRotation().Pitch > 15.0f);
		}
	}

	AFortRogueBattleCharacter* SteepSlopeCharacter = World->SpawnActor<AFortRogueBattleCharacter>(AFortRogueBattleCharacter::StaticClass(), FVector(-45.0f, 0.0f, 55.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Steep slope battle character is spawned"), SteepSlopeCharacter);
	if (SteepSlopeCharacter)
	{
		SteepSlopeCharacter->SetTerrain(Terrain);
		SetFloatProperty(SteepSlopeCharacter, TEXT("FootProbeHalfWidth"), 0.0f);
		SteepSlopeCharacter->SetActorLocation(FVector(-45.0f, 0.0f, 55.0f));
		SteepSlopeCharacter->BeginTurn();
		SteepSlopeCharacter->MoveHorizontal(1.0f, 0.12f);
		TestTrue(TEXT("Battle character stops at terrain steeper than the slope limit"), SteepSlopeCharacter->GetActorLocation().X < -30.0f);
		TestEqual(TEXT("Battle character does not climb the rejected steep slope"), static_cast<float>(SteepSlopeCharacter->GetActorLocation().Z), 55.0f);
	}

	AFortRogueBattleCharacter* SettlingCharacter = World->SpawnActor<AFortRogueBattleCharacter>(AFortRogueBattleCharacter::StaticClass(), FVector(5.0f, 0.0f, 105.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Settling battle character is spawned"), SettlingCharacter);
	if (SettlingCharacter)
	{
		SettlingCharacter->SetTerrain(Terrain);
		SetFloatProperty(SettlingCharacter, TEXT("FootProbeHalfWidth"), 0.0f);
		TestTrue(TEXT("Carve circle lowers nearby support under the character"), Terrain->CarveCircle(FVector(5.0f, 0.0f, 55.0f), 11.0f));
		SettlingCharacter->ReevaluateTerrainSupport();
		TestEqual(TEXT("Battle character settles onto a lower surface inside the step-down range"), static_cast<float>(SettlingCharacter->GetActorLocation().Z), 85.0f);
	}

	FVector ImpactLocation = FVector::ZeroVector;
	TestTrue(TEXT("Segment query hits solid terrain"), Terrain->FindFirstSolidAlongWorldSegment(FVector(-15.0f, 0.0f, 25.0f), FVector(-15.0f, 0.0f, 5.0f), ImpactLocation));
	TestTrue(TEXT("Segment impact lies inside the solid band"), ImpactLocation.Z >= 0.0f && ImpactLocation.Z < 10.0f);

	AFortRogueProjectile* AssignedTerrainProjectile = World->SpawnActor<AFortRogueProjectile>(AFortRogueProjectile::StaticClass(), FVector(-75.0f, 0.0f, 25.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Assigned-terrain projectile is spawned"), AssignedTerrainProjectile);
	if (AssignedTerrainProjectile && OverlappingTerrain)
	{
		if (UPrimitiveComponent* ProjectileCollision = Cast<UPrimitiveComponent>(AssignedTerrainProjectile->GetDefaultSubobjectByName(TEXT("Collision"))))
		{
			TestEqual(TEXT("Projectile root collision is disabled for custom terrain checks"), ProjectileCollision->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
		}
		if (UPrimitiveComponent* ProjectileVisual = Cast<UPrimitiveComponent>(AssignedTerrainProjectile->GetDefaultSubobjectByName(TEXT("Visual"))))
		{
			TestEqual(TEXT("Projectile visual collision is disabled"), ProjectileVisual->GetCollisionEnabled(), ECollisionEnabled::NoCollision);
		}
		AssignedTerrainProjectile->InitializeProjectile(Character, Terrain, FVector(0.0f, 0.0f, -100.0f), 0.0f, 24.0f, 0.0f);
		AssignedTerrainProjectile->Tick(0.25f);
		TestFalse(TEXT("Projectile carves its assigned terrain"), Terrain->IsSolidAtWorldLocation(FVector(-75.0f, 0.0f, 5.0f)));
		TestTrue(TEXT("Projectile does not carve overlapping unassigned terrain"), OverlappingTerrain->IsSolidAtWorldLocation(FVector(-75.0f, 0.0f, 5.0f)));
	}

	AFortRogueBattleCharacter* FallingCharacter = World->SpawnActor<AFortRogueBattleCharacter>(AFortRogueBattleCharacter::StaticClass(), FVector(45.0f, 0.0f, 55.0f), FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Falling battle character is spawned"), FallingCharacter);
	if (FallingCharacter)
	{
		FallingCharacter->SetTerrain(Terrain);
		const float BeforeReevaluateZ = FallingCharacter->GetActorLocation().Z;
		TestTrue(TEXT("Carve circle removes support under the falling character"), Terrain->CarveCircle(FVector(45.0f, 0.0f, 5.0f), 25.0f));
		FallingCharacter->ReevaluateTerrainSupport();
		TestTrue(TEXT("Battle character starts falling immediately after support terrain is destroyed"), FallingCharacter->GetActorLocation().Z < BeforeReevaluateZ);
	}

	TestTrue(TEXT("Carve circle reports a terrain change"), Terrain->CarveCircle(FVector(-15.0f, 0.0f, 5.0f), 6.0f));
	TestFalse(TEXT("Carved bottom cell is no longer solid"), Terrain->IsSolidAtWorldLocation(FVector(-15.0f, 0.0f, 5.0f)));
	TestFalse(TEXT("No surface remains after carving that column"), Terrain->FindSurfaceZAtWorldX(-15.0f, 30.0f, 40.0f, SurfaceZ));
	if (TexturePlane && TexturePlane->IsVisible() && TextureTerrainInstances)
	{
		TestEqual(TEXT("Texture-rendered terrain does not rebuild fallback instances after carving"), TextureTerrainInstances->GetInstanceCount(), 0);
	}

	AFortRogueDestructibleTerrain* RotatedTerrain = World->SpawnActor<AFortRogueDestructibleTerrain>(AFortRogueDestructibleTerrain::StaticClass(), FVector(200.0f, 0.0f, 0.0f), FRotator(0.0f, 90.0f, 90.0f), SpawnParams);
	TestNotNull(TEXT("Rotated terrain actor is spawned"), RotatedTerrain);
	if (RotatedTerrain)
	{
		TestTrue(TEXT("Terrain actor rotation is normalized for the gameplay X/Z plane"), RotatedTerrain->GetActorRotation().IsNearlyZero());
	}

	CleanupWorld();
	return true;
}

#endif
