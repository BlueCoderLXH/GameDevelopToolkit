//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/DungeonModel.h"
#include "Core/Utils/Rectangle.h"
#include "FloorPlanModel.generated.h"

enum class FloorChunkType {
    Room,
    Hall,
    Outside // Outside the floorplan
};

typedef int32 FloorChunkId;
class ADungeon;

struct FloorChunk {
    FloorChunk()
        : Id(-1)
          , ChunkType(FloorChunkType::Outside)
          , bReachable(false)
          , Priority(0)
          , bEmitGroundMarker(true)
          , bEmitCeilingMarker(true)
          , bConnectDoors(true)
          , bCreateWalls(true) {
    }

    FloorChunkId Id;
    FloorChunkType ChunkType;
    FRectangle Bounds;
    TArray<FIntVector> BoundCells;

    bool bReachable;
    float Priority;

    bool bEmitGroundMarker;
    bool bEmitCeilingMarker;
    bool bConnectDoors;
    bool bCreateWalls;

    FString WallMarker;
    FString GroundMarker;
    FString CeilingMarker;
    FString DoorMarker;
    FString CenterMarker;

    /** The marker on the center of the room, emitted at each floor it spans */
    FString PerFloorCenterMarker;

    FName VolumeId;

    FORCEINLINE int32 Area() const { return Bounds.Size.X * Bounds.Size.Y; }
    /** Gets the dimension of the largest side */
    FORCEINLINE int32 GetLength() const { return Bounds.Size.X > Bounds.Size.Y ? Bounds.Size.X : Bounds.Size.Y; }
    FORCEINLINE int32 GetWidth() const { return Bounds.Size.X < Bounds.Size.Y ? Bounds.Size.X : Bounds.Size.Y; }

    /** Sets the dimensions of the largest side */
    FORCEINLINE void SetLength(int32 Length) {
        if (Bounds.Size.X > Bounds.Size.Y) {
            Bounds.Size.X = Length;
        }
        else {
            Bounds.Size.Y = Length;
        }
    }

    FORCEINLINE void OffsetAlongLength(int32 Offset) {
        if (Bounds.Size.X > Bounds.Size.Y) {
            Bounds.Location.X += Offset;
        }
        else {
            Bounds.Location.Y += Offset;
        }
    }

    bool IsValidChunk() const {
        return GetWidth() > 0 && GetLength() > 0 && Area() > 0;
    }
};

typedef TSharedPtr<FloorChunk> FloorChunkPtr;

class FloorChunkDB {
public:
    FloorChunkDB() : IdCounter(0) {
    }

    FloorChunkPtr Create();
    void Register(FloorChunkPtr Chunk);
    void GetChunks(TArray<FloorChunkPtr>& OutChunks) const;
    void GetChunks(TArray<FloorChunkPtr>& OutChunks, FloorChunkType ChunkType) const;
    FloorChunkPtr GetChunk(FloorChunkId Id);
    FloorChunkPtr GetChunkAt(int32 x, int32 y, int32 z);
    FloorChunkPtr GetChunkAt(const FIntVector& Location);

    /** Create the cache for faster access */
    void CacheChunkPositions();

private:
    FORCEINLINE uint32 HASH(int32 x, int32 y, int32 z) {
        return GetTypeHash(FIntVector(x, y, z));
        //return  (x + 16384) << 16 | (y + 16384); 
    }

    struct FChunkCacheNode {
        FChunkCacheNode() : ChunkId(-1), Priority(0) {
        }

        FChunkCacheNode(FloorChunkId InChunkId, float InPriority) : ChunkId(InChunkId), Priority(InPriority) {
        }

        FloorChunkId ChunkId;
        float Priority;
    };

private:
    TMap<FloorChunkId, FloorChunkPtr> Chunks;
    TMap<uint32, FChunkCacheNode> CachePositionToChunk;
    int32 IdCounter;
};

typedef TSharedPtr<FloorChunkDB> FloorChunkDBPtr;
class AFloorPlanDoorVolume;

class FloorDoorManager {
public:
    void Initialize(ADungeon* InDungeon);

    void RegisterDoor(const FIntVector& A, const FIntVector& B);

    bool ContainsDoorVolume(const FVector& WorldLocation, const TArray<AFloorPlanDoorVolume*>& DoorVolumes);
    FORCEINLINE bool ContainsDoor(const FIntVector& A, const FIntVector& B) {
        if (!DoorMap.Contains(A)) {
            return false;
        }
        return DoorMap[A].Contains(B);
    }

    FORCEINLINE void Clear() {
        DoorMap.Reset();
    }

private:
    TMap<FIntVector, TSet<FIntVector>> DoorMap;
    FTransform DungeonTransform;
};


/** Floor plan model */
UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API UFloorPlanModel : public UDungeonModel {
    GENERATED_BODY()
public:
    virtual void Reset() override;
};

