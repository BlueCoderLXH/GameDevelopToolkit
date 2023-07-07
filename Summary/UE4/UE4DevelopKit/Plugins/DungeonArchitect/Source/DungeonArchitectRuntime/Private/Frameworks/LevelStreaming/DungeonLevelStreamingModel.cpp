//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/LevelStreaming/DungeonLevelStreamingModel.h"

#include "Core/Dungeon.h"
#include "Core/Utils/EditorService/IDungeonEditorService.h"
#include "Frameworks/LevelStreaming/DungeonLevelStreamingNavigation.h"

#include "Engine/Engine.h"
#include "Engine/LevelStreaming.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogLevelStreamingModel);

///////////////////////////////// UDungeonStreamingLevel ///////////////////////////////// 
void UDungeonStreamingLevel::CreateLevelStreaming(UObject* WorldContextObject, const FString& LongPackageName, uint32 InstanceId, const FVector& Location, const FRotator& Rotation, bool& bOutSuccess)
{
	LevelStreaming = FLevelStreamLoader::LoadLevelInstanceBySoftObjectPtr(
		WorldContextObject, LongPackageName, InstanceId, Location, Rotation, bOutSuccess, LevelPackage);

	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	if (World && !World->IsNetMode(NM_Client))
	{
		RegisterStreamingCallbacks();
	}	
}

void UDungeonStreamingLevel::RegisterStreamingCallbacks()
{
	if (LevelStreaming)
	{
		LevelStreaming->OnLevelLoaded.AddDynamic(this, &UDungeonStreamingLevel::HandleChunkLoaded);
		LevelStreaming->OnLevelUnloaded.AddDynamic(this, &UDungeonStreamingLevel::HandleChunkUnloaded);
		LevelStreaming->OnLevelShown.AddDynamic(this, &UDungeonStreamingLevel::HandleChunkVisible);
		LevelStreaming->OnLevelHidden.AddDynamic(this, &UDungeonStreamingLevel::HandleChunkHidden);
	}	
}

void UDungeonStreamingLevel::SetStreamingLevelState(bool bInVisible, bool bInLoaded)
{
	if (!LevelStreaming)
	{
		bShouldBeLoaded = false;
		bShouldBeVisible = false;
		return;
	}

	const bool bRequiresUpdate = RequiresStateUpdate(bInVisible, bInLoaded);

	if (bRequiresUpdate)
	{
		bShouldBeLoaded = bInLoaded;
		bShouldBeVisible = bInVisible;

		LevelStreaming->bShouldBlockOnUnload = false;
		LevelStreaming->bShouldBlockOnLoad = false;
		LevelStreaming->SetShouldBeLoaded(bInLoaded);
		LevelStreaming->SetShouldBeVisible(bInVisible);
	}
}

UDungeonStreamingChunk* UDungeonStreamingLevel::GetOwner() const
{
	return Cast<UDungeonStreamingChunk>(GetOuter());
}

void UDungeonStreamingLevel::HandleChunkLoaded()
{
	bIsLoaded = true;

	UDungeonStreamingChunk* OwnerChunk = GetOwner();
	if (OwnerChunk)
	{
		OwnerChunk->HandleChunkLoaded();
	}
}

void UDungeonStreamingLevel::HandleChunkUnloaded()
{
	bIsLoaded = false;

	UDungeonStreamingChunk* OwnerChunk = GetOwner();
	if (OwnerChunk)
	{
		OwnerChunk->HandleChunkUnloaded();
	}
}

void UDungeonStreamingLevel::HandleChunkVisible()
{
	bIsVisible = true;

	UDungeonStreamingChunk* OwnerChunk = GetOwner();
	if (OwnerChunk)
	{
		OwnerChunk->HandleChunkVisible();
	}
}

void UDungeonStreamingLevel::HandleChunkHidden()
{
	bIsVisible = false;

	UDungeonStreamingChunk* OwnerChunk = GetOwner();
	if (OwnerChunk)
	{
		OwnerChunk->HandleChunkHidden();
	}
}

void UDungeonStreamingLevel::OnDestroyChunk()
{
	if (LevelStreaming)
	{
		LevelStreaming->SetIsRequestingUnloadAndRemoval(true);
		LevelStreaming->SetShouldBeVisible(false);
		LevelStreaming->SetShouldBeLoaded(false);
	}

	LevelStreaming = nullptr;
	LevelPackage = nullptr;
}


///////////////////////////////// UDungeonLogicStreamingLevel ///////////////////////////////// 
void UDungeonLogicStreamingLevel::HandleChunkLoaded()
{
	bIsLoaded = true;

	const UDungeonStreamingChunk* OwnerChunk = GetOwner();
	const int32 OrderNum = OwnerChunk ? OwnerChunk->OrderNum : 0; 
	if (OrderNum < 0)
	{
		return;
	}

	if (!GetLevelStream() || !GetLevelStream()->GetLoadedLevel())
	{
		UE_LOG(LogLevelStreamingModel, Warning, TEXT("UDungeonStreamingChunk::HandleLogicChunkLoaded 'LogicLevelStreaming' is invalid!"));
		return;
	}

	FString PlayerStartTag;
	PlayerStartTag.Append(TEXT("PlayerStart_Level_Area_"));
	PlayerStartTag.Append(FString::FormatAsNumber(OrderNum));

	const ULevel* ActorLevel = GetLevelStream()->GetLoadedLevel();
	for (AActor* Actor : ActorLevel->Actors)
	{
		APlayerStart* PlayerStartActor = Cast<APlayerStart>(Actor);
		if (PlayerStartActor)
		{
			PlayerStartActor->PlayerStartTag = FName(PlayerStartTag);
		}
	}	
}

void UDungeonLogicStreamingLevel::HandleChunkUnloaded()
{
	bIsLoaded = false;
}

void UDungeonLogicStreamingLevel::HandleChunkVisible()
{
	bIsVisible = true;
}

void UDungeonLogicStreamingLevel::HandleChunkHidden()
{
	bIsVisible = false;
}


///////////////////////////////// UDungeonStreamingChunk ///////////////////////////////// 
UDungeonStreamingChunk::UDungeonStreamingChunk(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ArtLevel = nullptr;
}

void UDungeonStreamingChunk::CreateLevelStreaming(UObject* WorldContextObject, const FString& LongPackageName,
                                                  uint32 InstanceId, const FVector& Location, const FRotator& Rotation,
                                                  bool& bOutSuccess)
{
	ArtLevel = NewObject<UDungeonStreamingLevel>(this, UDungeonStreamingLevel::StaticClass());
	if (ArtLevel)
	{
		ArtLevel->CreateLevelStreaming(WorldContextObject, LongPackageName, InstanceId, Location, Rotation, bOutSuccess);
	}
}

void UDungeonStreamingChunk::CreateLogicLevelStreaming(UObject* WorldContextObject, const FString& LogicLongPackageName, uint32 InstanceId,
                                                       const FVector& Location, const FRotator& Rotation, bool& bOutSuccess)
{
	LogicLevel = NewObject<UDungeonLogicStreamingLevel>(this, UDungeonLogicStreamingLevel::StaticClass());
	if (LogicLevel)
	{
		LogicLevel->CreateLevelStreaming(WorldContextObject, LogicLongPackageName, InstanceId, Location, Rotation, bOutSuccess);
	}
}

bool UDungeonStreamingChunk::ShouldMakeOrder(const FString& OrderCategory) const
{
	// Check if need to make order or not
	return OrderCategory == TEXT("") ||
		OrderCategory == DAMarkerConstants::CC_All ||
		OrderCategory == DAMarkerConstants::CC_None ||
		OrderCategory == DAMarkerConstants::CC_StartRoom ||
		Category == DAMarkerConstants::CC_StartRoom ||
		Category.Contains(OrderCategory);
}

void UDungeonStreamingChunk::MakeOrder(const FString& OrderCategory, int32& CurrentOrder)
{
	if (ShouldMakeOrder(OrderCategory))
	{
		// 非初始关卡排序才自增序号
		if (Category != DAMarkerConstants::CC_StartRoom)
		{
			++CurrentOrder;
		}
		OrderNum = CurrentOrder;
	}
}

void UDungeonStreamingChunk::SetStreamingLevelState(bool bInVisible, bool bInLoaded) const
{
	if (!ArtLevel || !LogicLevel)
	{
		return;
	}

	ArtLevel->SetStreamingLevelState(bInVisible, bInLoaded);
	LogicLevel->SetStreamingLevelState(bInVisible, bInLoaded);
}

void UDungeonStreamingChunk::RegisterManagedActor(TWeakObjectPtr<AActor> InActor)
{
	ManagedActors.Add(InActor);
}

void UDungeonStreamingChunk::DestroyChunk(UWorld* InWorld)
{
	// Destroy the managed actors
	for (TWeakObjectPtr<AActor> ManagedActor : ManagedActors)
	{
		if (ManagedActor.IsValid())
		{
			ManagedActor->Destroy();
		}
	}
	ManagedActors.Reset();

	if (ArtLevel)
	{
		ArtLevel->OnDestroyChunk();
	}

	if (LogicLevel)
	{
		LogicLevel->OnDestroyChunk();
	}
}

UPackage* UDungeonStreamingChunk::GetLevelPackage() const
{
	return ArtLevel ? ArtLevel->GetLevelPackage() : nullptr;
}

UPackage* UDungeonStreamingChunk::GetLogicLevelPackage() const
{
	return LogicLevel ? LogicLevel->GetLevelPackage() : nullptr;
}

ULevel* UDungeonStreamingChunk::GetLoadedLevel() const
{
	return ArtLevel ? ArtLevel->GetLevel() : nullptr;
}

ULevel* UDungeonStreamingChunk::GetLoadedLogicLevel() const
{
	return LogicLevel ? LogicLevel->GetLevel() : nullptr;
}

ULevelStreaming* UDungeonStreamingChunk::GetStreamingLevel() const
{
	return ArtLevel ? ArtLevel->GetLevelStream() : nullptr;
}

ULevelStreaming* UDungeonStreamingChunk::GetLogicStreamingLevel() const
{
	return LogicLevel ? LogicLevel->GetLevelStream() : nullptr;
}

FTransform UDungeonStreamingChunk::GetLevelTransform() const
{
	return GetStreamingLevel() ? GetStreamingLevel()->LevelTransform : FTransform::Identity;
}

void UDungeonStreamingChunk::HandleChunkVisible()
{
	UDungeonLevelStreamingModel* StreamingModel = Cast<UDungeonLevelStreamingModel>(GetOuter());
	if (StreamingModel && StreamingModel->StreamingNavigation)
	{
		StreamingModel->StreamingNavigation->AddLevelNavigation(ArtLevel->GetLevel(), Bounds);
	}
}

void UDungeonStreamingChunk::HandleChunkHidden()
{
	UDungeonLevelStreamingModel* StreamingModel = Cast<UDungeonLevelStreamingModel>(GetOuter());
	if (ArtLevel && StreamingModel && StreamingModel->StreamingNavigation)
	{
		StreamingModel->StreamingNavigation->RemoveLevelNavigation(ArtLevel->GetLevel());
	}
}

void UDungeonStreamingChunk::HandleChunkLoaded()
{
}

void UDungeonStreamingChunk::HandleChunkUnloaded()
{
}


///////////////////////////////// UDungeonLevelStreamingModel /////////////////////////////////

UDungeonLevelStreamingModel::UDungeonLevelStreamingModel(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	StreamingNavigation = ObjectInitializer.CreateDefaultSubobject<UDungeonLevelStreamingNavigation>(
		this, "StreamingNavigation");
	StreamingNavigation->bAutoResizeNavVolume = true;
}

void UDungeonLevelStreamingModel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UDungeonLevelStreamingModel, ChunkParamsToClient);
}

void UDungeonLevelStreamingModel::Initialize(UWorld* InWorld)
{
	bNotifiedInitialChunkLoadEvent = false;

	if (StreamingNavigation)
	{
		StreamingNavigation->Initialize(InWorld);
	}
}

void UDungeonLevelStreamingModel::Release(UWorld* InWorld)
{
	if (StreamingNavigation)
	{
		StreamingNavigation->Release();
	}

	TArray<ULevelStreaming*> LevelsToRemove;
	TArray<UPackage*> LevelPackages;

	for (UDungeonStreamingChunk* Chunk : Chunks)
	{
		if (Chunk)
		{
			// Save the level package so we can unload it later on
			UPackage* LevelPackage = Chunk->GetLevelPackage();
			if (LevelPackage)
			{
				LevelPackage->SetDirtyFlag(false);
				LevelPackages.Add(LevelPackage);
			}

			UPackage* LogicLevelPackage = Chunk->GetLogicLevelPackage();
			if (LogicLevelPackage)
			{
				LogicLevelPackage->SetDirtyFlag(false);
				LevelPackages.Add(LogicLevelPackage);
			}

			ULevelStreaming* LevelToRemove = Chunk->GetStreamingLevel();
			if (LevelToRemove)
			{
				LevelsToRemove.Add(LevelToRemove);
			}

			ULevelStreaming* LogicLevelToRemove = Chunk->GetLogicStreamingLevel();
			if (LogicLevelToRemove)
			{
				LevelsToRemove.Add(LogicLevelToRemove);
			}

			// Destroy the chunk
			Chunk->DestroyChunk(InWorld);
		}
	}
	Chunks.Empty();

	if (!InWorld->IsGameWorld())
	{
		// Unload the transient levels from memory
		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
	}

	if (LevelsToRemove.Num() > 0)
	{
		// Make sure the streaming levels in the world are flushed before we unload the levels
		InWorld->FlushLevelStreaming(EFlushLevelStreamingType::Full);
		InWorld->RemoveStreamingLevels(LevelsToRemove);
	}

	if (LevelPackages.Num() > 0)
	{
		TSharedPtr<IDungeonEditorService> EditorService = IDungeonEditorService::Get();
		if (EditorService.IsValid())
		{
			if (!EditorService->UnloadPackages(LevelPackages))
			{
				UE_LOG(LogLevelStreamingModel, Error, TEXT("Failed to unload streaming chunk level packages"));
			}
		}
	}
}

void UDungeonLevelStreamingModel::NotifyInitialChunksLoaded()
{
	ADungeon* Dungeon = Cast<ADungeon>(GetOuter());
	OnInitialChunksLoaded.Broadcast(Dungeon);
	bNotifiedInitialChunkLoadEvent = true;
}

namespace
{
	void GetPlayerSourceLocations(UWorld* InWorld, TArray<FVector>& OutSourceLocations)
	{
		OutSourceLocations.Reset();
		for (TActorIterator<APlayerController> It(InWorld); It; ++It)
		{
			APlayerController* PlayerController = *It;
			if (PlayerController)
			{
				APawn* Pawn = PlayerController->GetPawnOrSpectator();
				if (Pawn)
				{
					FVector PlayerLocation = Pawn->GetActorLocation();
					OutSourceLocations.Add(PlayerLocation);
				}
			}
		}
	}

	void GetSpawnRoomSourceLocations(TArray<UDungeonStreamingChunk*> InChunks, TArray<FVector>& OutSourceLocations)
	{
		OutSourceLocations.Reset();
		for (UDungeonStreamingChunk* Chunk : InChunks)
		{
			if (Chunk && Chunk->bSpawnRoomChunk)
			{
				OutSourceLocations.Add(Chunk->Bounds.GetCenter());
			}
		}
	}
}

void UDungeonLevelStreamingModel::GetStreamingSourceLocations(UWorld* InWorld,
                                                              EDungeonLevelStreamChunkSelection InChunkSelection,
                                                              TArray<FVector>& OutSourceLocations) const
{
	switch (InChunkSelection)
	{
	case EDungeonLevelStreamChunkSelection::SpawnRoomLocations:
		GetSpawnRoomSourceLocations(Chunks, OutSourceLocations);
		break;

	case EDungeonLevelStreamChunkSelection::PlayerLocations:
	default:
		GetPlayerSourceLocations(InWorld, OutSourceLocations);
		break;
	}
}

int32 UDungeonLevelStreamingModel::GetChunkNumberByObject(const UObject* SceneObject) const
{
	if (!IsValid(SceneObject))
	{
		return -1;
	}

	const UObject* OuterLevel = SceneObject->GetOuter();
	if (!IsValid(OuterLevel))
	{
		return -1;
	}
	
	int32 OutChunkNumber = -1;

	for (const UDungeonStreamingChunk* Chunk : Chunks)
	{
		if (!Chunk)
		{
			continue;
		}

		if (OuterLevel == Chunk->GetLoadedLevel() ||
			OuterLevel == Chunk->GetLoadedLogicLevel())
		{
			OutChunkNumber = Chunk->OrderNum;
			break;
		}
	}
	
	return OutChunkNumber;
}

void UDungeonLevelStreamingModel::OnRep_ChunkParamsToClient()
{
	if (!IsNetMode(NM_Client))
	{
		return;
	}

	// Create Chunks
	for (const FDungeonStreamingChunkParamRep& ChunkParam : ChunkParamsToClient)
	{
		UDungeonStreamingChunk* Chunk = NewObject<UDungeonStreamingChunk>(this, UDungeonStreamingChunk::StaticClass());
		if (Chunk)
		{
			bool bOutSuccess;
			Chunk->CreateLevelStreaming(GetWorld(), ChunkParam.LevelLongPackageName, ChunkParam.InstanceId, ChunkParam.Location, ChunkParam.Rotation,
			                            bOutSuccess);
			Chunk->CreateLogicLevelStreaming(GetWorld(), ChunkParam.LogicLevelLongPackageName, ChunkParam.InstanceId, ChunkParam.Location, ChunkParam.Rotation,
			                                 bOutSuccess);

			Chunk->OrderNum = ChunkParam.OrderNum;
			Chunk->Category = ChunkParam.Category;
			Chunk->ID = ChunkParam.ID;
			Chunk->Bounds = ChunkParam.Bounds;
			Chunk->bSpawnRoomChunk = ChunkParam.bSpawnRoomChunk;

			Chunks.Add(Chunk);
		}
	}

	// Handle neighbor chunks
	for (const FDungeonStreamingChunkParamRep& ChunkParam : ChunkParamsToClient)
	{
		UDungeonStreamingChunk** Chunk = Chunks.FindByKey(ChunkParam.ID);
		if (Chunk)
		{
			for (const FGuid& NeighborID : ChunkParam.Neighbors)
			{
				UDungeonStreamingChunk** NeighborChunk = Chunks.FindByKey(NeighborID);
				if (NeighborChunk)
				{
					(*Chunk)->Neighbors.Add(*NeighborChunk);
				}
			}
		}
	}
}


///////////////////////////////// UDungeonStreamingLevel ///////////////////////////////// 
ULevelStreamingDynamic* FLevelStreamLoader::LoadLevelInstanceBySoftObjectPtr(
	UObject* WorldContextObject, const FString& LongPackageName, uint32 InstanceId, FVector Location, FRotator Rotation,
	bool& bOutSuccess, UPackage*& OutLevelPackage)
{
	bOutSuccess = false;
	OutLevelPackage = nullptr;

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return nullptr;
	}

	// Check whether requested map exists, this could be very slow if LevelName is a short package name
	if (LongPackageName.IsEmpty())
	{
		return nullptr;
	}

	bOutSuccess = true;
	return LoadLevelInstance_Internal(World, LongPackageName, InstanceId, Location, Rotation, OutLevelPackage);
}

ULevelStreamingDynamic* FLevelStreamLoader::LoadLevelInstance_Internal(UWorld* World, const FString& LongPackageName,
                                                                       uint32 InstanceId, FVector Location,
                                                                       FRotator Rotation, UPackage*& OutLevelPackage)
{
	if (!World) return nullptr;

	// Create Unique Name for sub-level package
	const FString PackagePath = FPackageName::GetLongPackagePath(LongPackageName);
	FString ShortPackageName = FPackageName::GetShortName(LongPackageName);

	if (ShortPackageName.StartsWith(World->StreamingLevelsPrefix))
	{
		ShortPackageName.RightChopInline(World->StreamingLevelsPrefix.Len(), false);
	}

	// Remove PIE prefix if it's there before we actually load the level
	const FString OnDiskPackageName = PackagePath + TEXT("/") + ShortPackageName;

	// Create Unique Name for sub-level package
	const uint32 Hash = HashCombine(GetTypeHash(Location), GetTypeHash(Rotation.Euler()));
	FString UniqueLevelPackageName = FString::Printf(
		TEXT("%s/%s%s_LI_%s_%d_%u"), *PackagePath, *World->StreamingLevelsPrefix,
		*ShortPackageName, *World->GetName(), InstanceId, Hash);

	if (!World->IsGameWorld())
	{
		UPackage* LevelPackage = CreatePackage(*UniqueLevelPackageName);
		LevelPackage->SetFlags(RF_Transient | RF_DuplicateTransient);
		LevelPackage->SetDirtyFlag(false);
		OutLevelPackage = LevelPackage;
	}
	else
	{
		UniqueLevelPackageName += TEXT("G");
	}

	ULevelStreamingDynamic* StreamingLevel = NewObject<ULevelStreamingDynamic>(
		World, ULevelStreamingDynamic::StaticClass(), NAME_None, RF_Transient | RF_DuplicateTransient, nullptr);
	StreamingLevel->SetWorldAssetByPackageName(FName(*UniqueLevelPackageName));

	StreamingLevel->LevelColor = FColor::MakeRandomColor();
	StreamingLevel->SetShouldBeLoaded(false);
	StreamingLevel->SetShouldBeVisible(false);
	StreamingLevel->bShouldBlockOnLoad = false;
	StreamingLevel->bInitiallyLoaded = false;
	StreamingLevel->bInitiallyVisible = false;
	// Transform
	StreamingLevel->LevelTransform = FTransform(Rotation, Location);
	// Map to Load
	StreamingLevel->PackageNameToLoad = FName(*OnDiskPackageName);

	// Add the new level to world.
	World->AddStreamingLevel(StreamingLevel);

	return StreamingLevel;
}