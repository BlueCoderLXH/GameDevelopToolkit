//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Snap/Lib/Streaming/SnapStreaming.h"

#include "Frameworks/Snap/Lib/Connection/SnapConnectionActor.h"
#include "Frameworks/Snap/Lib/Serialization/ConnectionSerialization.h"
#include "Frameworks/Snap/Lib/Serialization/SnapActorSerialization.h"
#include "Frameworks/Snap/Lib/SnapLibrary.h"

#include "Core/DungeonArchitectConstants.h"

#include "Components/ModelComponent.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"

///////////////////////////////////// USnapStreamingChunk /////////////////////////////////////

USnapStreamingChunk::USnapStreamingChunk(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SerializedData = ObjectInitializer.CreateDefaultSubobject<USnapStreamingChunkActorData>(this, "SerializedData");
}

void USnapStreamingChunk::HandleChunkVisible()
{
	Super::HandleChunkVisible();

	if (OnChunkVisible.IsBound())
	{
		OnChunkVisible.Execute(this);
	}

	ULevel* Level = GetLoadedLevel();

	// Serialize the level actor data
	if (Level)
	{
		SerializedData->LoadLevel(Level);
	}
}

void USnapStreamingChunk::HandleChunkHidden()
{
	Super::HandleChunkHidden();

	if (OnChunkHidden.IsBound())
	{
		OnChunkHidden.Execute(this);
	}

	ULevel* Level = GetLoadedLevel();
	if (Level)
	{
		SerializedData->SaveLevel(Level);
	}
}


void USnapStreamingChunk::HandleChunkLoaded()
{
	Super::HandleChunkLoaded();

	if (OnChunkLoaded.IsBound())
	{
		OnChunkLoaded.Execute(this);
	}
}

void USnapStreamingChunk::HandleChunkUnloaded()
{
	Super::HandleChunkUnloaded();

	if (OnChunkUnloaded.IsBound())
	{
		OnChunkUnloaded.Execute(this);
	}
}

void USnapStreamingChunk::DestroyChunk(UWorld* InWorld)
{
	Super::DestroyChunk(InWorld);
}

///////////////////////////////////// FSnapStreamingChunkHandlerBase /////////////////////////////////////

namespace
{
	template <typename T>
	TArray<T*> GetActorsOfType(ULevel* Level)
	{
		TArray<T*> Result;
		for (int i = 0; i < Level->Actors.Num(); i++)
		{
			if (T* TargetActor = Cast<T>(Level->Actors[i]))
			{
				Result.Add(TargetActor);
			}
		}
		return Result;
	}

	const FSnapConnectionInstance* FindConnectedDoorInstance(const FSnapConnectionInstance* InConnection,
	                                                         const TArray<FSnapConnectionInstance>&
	                                                         InConnectionList)
	{
		if (InConnection)
		{
			for (const FSnapConnectionInstance& Other : InConnectionList)
			{
				if (Other.ModuleA == InConnection->ModuleB && Other.DoorA == InConnection->DoorB
					&& Other.ModuleB == InConnection->ModuleA && Other.DoorB == InConnection->DoorA)
				{
					return &Other;
				}
			}
		}
		return nullptr;
	}
}

void FSnapStreamingChunkHandlerBase::RegisterEvents(USnapStreamingChunk* InChunk)
{
	InChunk->OnChunkVisible.BindSP(SharedThis(this), &FSnapStreamingChunkHandlerBase::OnChunkVisible);
	InChunk->OnChunkHidden.BindSP(SharedThis(this), &FSnapStreamingChunkHandlerBase::OnChunkHidden);
	InChunk->OnChunkLoaded.BindSP(SharedThis(this), &FSnapStreamingChunkHandlerBase::OnChunkLoaded);
	InChunk->OnChunkUnloaded.BindSP(SharedThis(this), &FSnapStreamingChunkHandlerBase::OnChunkUnloaded);
}

void FSnapStreamingChunkHandlerBase::OnChunkVisible(USnapStreamingChunk* Chunk)
{
	UWorld* World = GetWorld();
	if (!World) return;

	UpdateChunkDoorStates(Chunk, World->PersistentLevel);

	VisibleModules.Add(Chunk->ID);
}

void FSnapStreamingChunkHandlerBase::OnChunkHidden(USnapStreamingChunk* Chunk)
{
	HideChunkDoorActors(Chunk);
}

void FSnapStreamingChunkHandlerBase::UpdateChunkDoorStates(USnapStreamingChunk* Chunk, ULevel* PersistentLevel) const
{
	if (!Chunk)
	{
		return;
	}

	ULevel* ChunkLevel = Chunk->GetLoadedLevel();
	if (!ChunkLevel)
	{
		return;
	}

	TArray<FSnapConnectionInstance>* ConnectionsPtr = GetConnections();
	if (!ConnectionsPtr)
	{
		return;
	}
	TArray<FSnapConnectionInstance>& Connections = *ConnectionsPtr;

	// The connection info for this chunk, by their door ids
	TMap<FGuid, FSnapConnectionInstance*> ModuleConnections;
	for (FSnapConnectionInstance& ConnectionData : Connections)
	{
		if (ConnectionData.ModuleA == Chunk->ID)
		{
			if (!ModuleConnections.Contains(ConnectionData.DoorA))
			{
				ModuleConnections.Add(ConnectionData.DoorA, &ConnectionData);
			}
		}
	}

	TArray<ASnapConnectionActor*> ConnectionActors = GetActorsOfType<ASnapConnectionActor>(ChunkLevel);
	for (ASnapConnectionActor* ConnectionActor : ConnectionActors)
	{
		FSnapConnectionInstance** ConnectionDataSearchResult = ModuleConnections.Find(
			ConnectionActor->GetConnectionId());
		FSnapConnectionInstance* ConnectionData = ConnectionDataSearchResult ? *ConnectionDataSearchResult : nullptr;

		const FSnapConnectionInstance* OtherConnectionData = FindConnectedDoorInstance(ConnectionData, Connections);
		if (ConnectionData && OtherConnectionData)
		{
			// We make door only in 'Corridor' Chunk
			if (Chunk->Category != DAMarkerConstants::CC_Corridor)
			{
				continue;
			}

			// 前门序号(bReverse:false) = Chunk序号 * 2, 后门序号(bReverse:true) = Chunk序号 * 2 - 1
			const int32 DoorOrderNum = Chunk->OrderNum * 2 - (ConnectionData->bReverse ? 1 : 0);

			// We have a connection to another chunk node.  This is a door

			if (!ConnectionData->bHasSpawnedDoorActor)
			{
				ConnectionActor->DestroyConnectionInstance();
				UpdateConnectionDoorType(ConnectionData, ConnectionActor->ConnectionComponent);

				// Check if the other door has spawned the shared door actor
				if (OtherConnectionData->bHasSpawnedDoorActor)
				{
					ConnectionData->SpawnedDoorActors = OtherConnectionData->SpawnedDoorActors;
					ConnectionData->bHasSpawnedDoorActor = true;
				}
				else
				{
					ConnectionActor->BuildConnectionInstance(PersistentLevel, DoorOrderNum);
					ConnectionData->SpawnedDoorActors = ConnectionActor->GetSpawnedInstancesPtr();
					ConnectionData->bHasSpawnedDoorActor = true;
					OnConnectionDoorCreated(ConnectionData);
				}
			}

			// Show the actor in the persistent level (it might have been previously hidden due to level streaming)
			if (ConnectionData->bHasSpawnedDoorActor)
			{
				for (TWeakObjectPtr<AActor> SpawnedDoorActor : ConnectionData->SpawnedDoorActors)
				{
					if (SpawnedDoorActor.IsValid())
					{
						SpawnedDoorActor->SetActorHiddenInGame(false);
					}
				}
			}
		}
		else
		{
			// No connection exists. This is a wall
			ConnectionActor->ConnectionComponent->ConnectionState = ESnapConnectionState::Wall;
			ConnectionActor->BuildConnectionInstance(ChunkLevel, -1);
		}
	}
}

void FSnapStreamingChunkHandlerBase::HideChunkDoorActors(USnapStreamingChunk* Chunk)
{
	if (!Chunk)
	{
		return;
	}
	VisibleModules.Remove(Chunk->ID);

	ULevel* ChunkLevel = Chunk->GetLoadedLevel();
	if (!ChunkLevel)
	{
		return;
	}
	TArray<FSnapConnectionInstance>* ConnectionsPtr = GetConnections();
	if (!ConnectionsPtr)
	{
		return;
	}
	TArray<FSnapConnectionInstance>& Connections = *ConnectionsPtr;

	// The connection info for this chunk, by their door ids
	TMap<FGuid, FSnapConnectionInstance*> ModuleConnections;
	for (FSnapConnectionInstance& ConnectionData : Connections)
	{
		if (ConnectionData.ModuleA == Chunk->ID)
		{
			if (!ModuleConnections.Contains(ConnectionData.DoorA))
			{
				ModuleConnections.Add(ConnectionData.DoorA, &ConnectionData);
			}
		}
	}

	TArray<ASnapConnectionActor*> ConnectionActors = GetActorsOfType<ASnapConnectionActor>(ChunkLevel);
	for (ASnapConnectionActor* ConnectionActor : ConnectionActors)
	{
		if (ConnectionActor->ConnectionComponent->ConnectionState == ESnapConnectionState::Wall)
		{
			// Destroy the wall actors
			ConnectionActor->DestroyConnectionInstance();
		}
		else
		{
			FSnapConnectionInstance** ConnectionDataSearchResult = ModuleConnections.Find(
				ConnectionActor->GetConnectionId());
			FSnapConnectionInstance* ConnectionData =
				ConnectionDataSearchResult ? *ConnectionDataSearchResult : nullptr;

			if (ConnectionData)
			{
				// We have a door here in the persistent level
				FGuid ConnectedChunkID = (ConnectionData->ModuleA == Chunk->ID)
					                         ? ConnectionData->ModuleB
					                         : ConnectionData->ModuleA;
				if (!VisibleModules.Contains(ConnectedChunkID))
				{
					// This chunk and the connected chunks are both hidden.   Hide the persistent door actor
					for (TWeakObjectPtr<AActor> PersistentDoorActor : ConnectionData->SpawnedDoorActors)
					{
						PersistentDoorActor->SetActorHiddenInGame(true);
					}
				}
			}
		}
	}
}

void FSnapStreamingChunkHandlerBase::OnChunkLoaded(USnapStreamingChunk* Chunk)
{
}

void FSnapStreamingChunkHandlerBase::OnChunkUnloaded(USnapStreamingChunk* Chunk)
{
}

void FSnapStreamingChunkHandlerBase::ClearStreamingLevels()
{
	VisibleModules.Reset();

	UWorld* World = GetWorld();
	if (!World) return;

	// Destroy the spawned door actors
	{
		TArray<FSnapConnectionInstance>* ConnectionInstances = GetConnections();
		if (ConnectionInstances)
		{
			for (FSnapConnectionInstance& ConnectionInstance : *ConnectionInstances)
			{
				for (TWeakObjectPtr<AActor> PersistentDoorActor : ConnectionInstance.SpawnedDoorActors)
				{
					if (PersistentDoorActor.IsValid())
					{
						PersistentDoorActor->Destroy();
						PersistentDoorActor.Reset();
					}
				}
				ConnectionInstance.SpawnedDoorActors.Reset();
			}
		}
	}

	UDungeonLevelStreamingModel* LevelStreamingModel = GetLevelStreamingModel();
	if (LevelStreamingModel)
	{
		// Save a reference to this chunk so they can be removed when destroyed
		LevelStreamingModel->Release(World);
	}

	World->UpdateLevelStreaming();
	World->FlushLevelStreaming(EFlushLevelStreamingType::Full);

	if (GEngine)
	{
		GEngine->ForceGarbageCollection(true);
	}
}


///////////////////////////////////// FSnapStreaming /////////////////////////////////////

namespace
{
	UDungeonStreamingChunk* GenerateLevelStreamingChunkRecursive(
		UWorld* World, const FString& OrderCategory, int32& CurrentOrder,
		UObject* ChunkOwner, TSubclassOf<UDungeonStreamingChunk> ChunkClass,
		SnapLib::FModuleNodePtr Node,
		UDungeonStreamingChunk* IncomingChunk, TArray<UDungeonStreamingChunk*>& OutChunks,
		TMap<SnapLib::FModuleNodePtr, UDungeonStreamingChunk*>& VisitedChunkMap,
		TFunction<void(UDungeonStreamingChunk*, SnapLib::FModuleNodePtr)> FnInitChunk)
	{
		if (!Node.IsValid()) return nullptr;

		check(!VisitedChunkMap.Contains(Node));

		UDungeonStreamingChunk* Chunk = NewObject<UDungeonStreamingChunk>(ChunkOwner, ChunkClass);
		FnInitChunk(Chunk, Node);

		Chunk->Category = Node->Category.ToString();
		Chunk->ID = Node->ModuleInstanceId;
		Chunk->Bounds = Node->GetModuleBounds();
		VisitedChunkMap.Add(Node, Chunk);

		Chunk->MakeOrder(OrderCategory, CurrentOrder);

		for (SnapLib::FModuleDoorPtr OutgoingDoor : Node->Outgoing)
		{
			if (!OutgoingDoor.IsValid() || !OutgoingDoor->ConnectedDoor.IsValid())
			{
				continue;
			}
			SnapLib::FModuleNodePtr ChildNode = OutgoingDoor->ConnectedDoor->Owner;
			if (!ChildNode.IsValid())
			{
				continue;
			}

			UDungeonStreamingChunk* ChildChunk;
			if (!VisitedChunkMap.Contains(ChildNode))
			{
				ChildChunk = GenerateLevelStreamingChunkRecursive(
					World, OrderCategory, CurrentOrder, ChunkOwner, ChunkClass, ChildNode, Chunk, OutChunks, VisitedChunkMap, FnInitChunk);
				if (ChildChunk)
				{
					Chunk->Neighbors.Add(ChildChunk);
				}
			}
			else
			{
				// This child has already been processed. Grab the child chunk node and add a reference
				ChildChunk = VisitedChunkMap[ChildNode];
				if (ChildChunk)
				{
					Chunk->Neighbors.Add(ChildChunk);
					ChildChunk->Neighbors.Add(Chunk);
				}
			}

			// Room order number equal to that of Corridor
			if (Chunk && Chunk->IsCorridor() &&
				ChildChunk && ChildChunk->IsRoom())
			{
				ChildChunk->OrderNum = Chunk->OrderNum;
			}
		}
		
		if (IncomingChunk)
		{
			Chunk->Neighbors.Add(IncomingChunk);
		}

		// Generate the level streaming object
		{
			const FTransform NodeTransform = Node->WorldTransform;
			const FVector Location = NodeTransform.GetLocation();
			const FRotator Rotation = NodeTransform.GetRotation().Rotator();
			const FString& ArtLevelPackageName = Node->ModuleDBItem->GetLevel().GetLongPackageName();
			const FString& LogicLevelPackageName = Node->ModuleDBItem->GetLogicLevel().GetLongPackageName();
			bool bSuccess = false;
			Chunk->CreateLevelStreaming(World, ArtLevelPackageName, Node->NetworkId, Location, Rotation, bSuccess);
			Chunk->CreateLogicLevelStreaming(World, LogicLevelPackageName, Node->NetworkId, Location, Rotation,bSuccess);

			{
				UDungeonLevelStreamingModel* StreamingModel = Cast<UDungeonLevelStreamingModel>(ChunkOwner);
				if (StreamingModel)
				{
					FDungeonStreamingChunkParamRep ChunkParam;
					ChunkParam.LevelLongPackageName = ArtLevelPackageName;
					ChunkParam.LogicLevelLongPackageName = LogicLevelPackageName;
					ChunkParam.InstanceId = Node->NetworkId;
					ChunkParam.Location = Location;
					ChunkParam.Rotation = Rotation;
					ChunkParam.OrderNum = Chunk->OrderNum;
					ChunkParam.Category = Chunk->Category;
					ChunkParam.ID = Chunk->ID;
					ChunkParam.Bounds = Chunk->Bounds;
					ChunkParam.bSpawnRoomChunk = Chunk->bSpawnRoomChunk;
					for (auto NeighborChunk : Chunk->Neighbors)
					{
						ChunkParam.Neighbors.Add(NeighborChunk->ID);
					}
					StreamingModel->PushChunkParam(ChunkParam);
				}
			}
		}

		OutChunks.Add(Chunk);

		return Chunk;
	}
}


void FSnapStreamingChunkHandlerBase::UpdateConnectionDoorType(const FSnapConnectionInstance* ConnectionData,
                                                              USnapConnectionComponent* ConnectionComponent) const
{
	ConnectionComponent->ConnectionState = ESnapConnectionState::Door;
}

void FSnapStreaming::GenerateLevelStreamingModel(
	UWorld* World,
	const TArray<SnapLib::FModuleNodePtr>& InModuleNodes,
	UDungeonLevelStreamingModel* LevelStreamingModel,
	TSubclassOf<UDungeonStreamingChunk> ChunkClass,
	TFunction<void(UDungeonStreamingChunk*, SnapLib::FModuleNodePtr)> FnInitChunk,
	const FString& OrderCategory)
{
	if (!World || !LevelStreamingModel) return;

	LevelStreamingModel->Initialize(World);

	TMap<SnapLib::FModuleNodePtr, UDungeonStreamingChunk*> VisitedChunkMap;
	UObject* ChunkOwner = LevelStreamingModel;
	int32 ChunkOrder = 0;
	for (SnapLib::FModuleNodePtr Node : InModuleNodes)
	{
		if (VisitedChunkMap.Contains(Node)) continue;
		GenerateLevelStreamingChunkRecursive(World, OrderCategory, ChunkOrder, ChunkOwner, ChunkClass, Node, nullptr,
		                                     LevelStreamingModel->Chunks, VisitedChunkMap, FnInitChunk);
	}

	World->UpdateLevelStreaming();
	World->FlushLevelStreaming(EFlushLevelStreamingType::Full);
}
