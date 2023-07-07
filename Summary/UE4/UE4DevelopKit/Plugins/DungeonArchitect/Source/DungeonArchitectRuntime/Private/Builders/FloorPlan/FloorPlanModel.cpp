//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/FloorPlan/FloorPlanModel.h"

#include "Builders/FloorPlan/Volumes/FloorPlanDoorVolume.h"
#include "Core/Dungeon.h"

void UFloorPlanModel::Reset() {
}


///////////////////////////////////////////// FloorChunkDB //////////////////////////////////////////////

FloorChunkPtr FloorChunkDB::Create() {
    FloorChunkPtr Chunk = MakeShareable(new FloorChunk());
    Chunk->Id = ++IdCounter;
    return Chunk;
}

void FloorChunkDB::Register(FloorChunkPtr Chunk) {
    if (!Chunks.Contains(Chunk->Id)) {
        Chunks.Add(Chunk->Id, Chunk);
    }
}

void FloorChunkDB::GetChunks(TArray<FloorChunkPtr>& OutChunks) const {
    OutChunks.Reset();
    Chunks.GenerateValueArray(OutChunks);
}

void FloorChunkDB::GetChunks(TArray<FloorChunkPtr>& OutChunks, FloorChunkType ChunkType) const {
    OutChunks.Reset();
    TArray<FloorChunkPtr> AllChunks;
    Chunks.GenerateValueArray(AllChunks);
    for (FloorChunkPtr Chunk : AllChunks) {
        if (Chunk->ChunkType == ChunkType) {
            OutChunks.Add(Chunk);
        }
    }
}

FloorChunkPtr FloorChunkDB::GetChunk(FloorChunkId Id) {
    if (!Chunks.Contains(Id)) {
        return nullptr;
    }
    return Chunks[Id];
}

FloorChunkPtr FloorChunkDB::GetChunkAt(const FIntVector& Location) {
    return GetChunkAt(Location.X, Location.Y, Location.Z);
}

FloorChunkPtr FloorChunkDB::GetChunkAt(int32 x, int32 y, int32 z) {
    int32 Hash = HASH(x, y, z);
    if (CachePositionToChunk.Contains(Hash)) {
        FloorChunkId ChunkId = CachePositionToChunk[Hash].ChunkId;
        return GetChunk(ChunkId);
    }
    return nullptr;
}

void FloorChunkDB::CacheChunkPositions() {
    TArray<FloorChunkPtr> ChunkList;
    GetChunks(ChunkList);

    for (FloorChunkPtr Chunk : ChunkList) {
        const int32 X0 = Chunk->Bounds.Location.X;
        const int32 Y0 = Chunk->Bounds.Location.Y;
        const int32 X1 = X0 + Chunk->Bounds.Size.X;
        const int32 Y1 = Y0 + Chunk->Bounds.Size.Y;
        const int32 z = Chunk->Bounds.Location.Z;
        TArray<FIntVector> BoundCells = Chunk->BoundCells;
        if (BoundCells.Num() == 0) {
            for (int x = X0; x < X1; x++) {
                for (int y = Y0; y < Y1; y++) {
                    BoundCells.Add(FIntVector(x, y, z));
                }
            }
        }

        for (const FIntVector& Cell : BoundCells) {
            uint32 Hash = HASH(Cell.X, Cell.Y, Cell.Z);
            if (!CachePositionToChunk.Contains(Hash)) {
                CachePositionToChunk.Add(Hash, FChunkCacheNode(Chunk->Id, Chunk->Priority));
            }
            else {
                // Entry already exists.  Override if the priority is higher
                FChunkCacheNode& Node = CachePositionToChunk[Hash];
                if (Node.Priority < Chunk->Priority) {
                    Node.ChunkId = Chunk->Id;
                    Node.Priority = Chunk->Priority;
                }
            }
        }
    }
}


void FloorDoorManager::Initialize(ADungeon* InDungeon) {
    Clear();
    this->DungeonTransform = InDungeon ? InDungeon->GetActorTransform() : FTransform::Identity;
}

void FloorDoorManager::RegisterDoor(const FIntVector& A, const FIntVector& B) {
    if (!DoorMap.Contains(A)) {
        DoorMap.Add(A, TSet<FIntVector>());
    }
    DoorMap[A].Add(B);

    if (!DoorMap.Contains(B)) {
        DoorMap.Add(B, TSet<FIntVector>());
    }
    DoorMap[B].Add(A);
}

bool FloorDoorManager::ContainsDoorVolume(const FVector& WorldLocation,
                                          const TArray<AFloorPlanDoorVolume*>& DoorVolumes) {
    FVector FixedLocation = DungeonTransform.TransformPosition(WorldLocation);
    for (AFloorPlanDoorVolume* DoorVolume : DoorVolumes) {
        if (DoorVolume->EncompassesPoint(FixedLocation)) {
            return true;
        }
    }
    return false;
}

