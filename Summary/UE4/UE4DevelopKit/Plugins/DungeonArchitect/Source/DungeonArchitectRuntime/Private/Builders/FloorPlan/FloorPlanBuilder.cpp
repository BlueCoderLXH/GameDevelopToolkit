//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/FloorPlan/FloorPlanBuilder.h"

#include "Builders/FloorPlan/FloorPlanConfig.h"
#include "Builders/FloorPlan/FloorPlanModel.h"
#include "Builders/FloorPlan/FloorPlanQuery.h"
#include "Builders/FloorPlan/FloorPlanSelectorLogic.h"
#include "Builders/FloorPlan/FloorPlanToolData.h"
#include "Builders/FloorPlan/FloorPlanTransformLogic.h"
#include "Builders/FloorPlan/Volumes/FloorPlanDoorVolume.h"
#include "Builders/FloorPlan/Volumes/FloorPlanRoomVolume.h"
#include "Builders/FloorPlan/Volumes/FloorVolumeQuery.h"
#include "Core/Dungeon.h"
#include "Core/Utils/DungeonModelHelper.h"
#include "Core/Utils/MathUtils.h"
#include "Core/Volumes/DungeonThemeOverrideVolume.h"

#include "DrawDebugHelpers.h"
#include "EngineUtils.h"
#include <stack>

DEFINE_LOG_CATEGORY(FloorPlanBuilderLog);


void UFloorPlanBuilder::BuildDungeonImpl(UWorld* World) {
    floorPlanModel = Cast<UFloorPlanModel>(model);
    floorPlanConfig = Cast<UFloorPlanConfig>(config);
    DoorManager.Initialize(Dungeon);

    if (!floorPlanModel) {
        UE_LOG(FloorPlanBuilderLog, Error, TEXT("Invalid dungeon model provided to the floorplan builder"));
        return;
    }

    if (!floorPlanConfig) {
        UE_LOG(FloorPlanBuilderLog, Error, TEXT("Invalid dungeon config provided to the floorplan builder"));
        return;
    }

    BuildLayout();

    PropSockets.Reset();
}

void UFloorPlanBuilder::SplitChunk(FloorChunkPtr Chunk, FloorChunkPtr OutLeft, FloorChunkPtr OutRight,
                                   FloorChunkPtr OutHallway) {
    int32 HallWidth = floorPlanConfig->HallWidth;
    const int32 Length = Chunk->GetLength();
    int32 RemainingLength = Length - HallWidth;
    int32 MinChunkLength = floorPlanConfig->MinRoomSize;
    const int32 LengthLeft = MinChunkLength + Random.RandRange(0, FMath::Max(0, RemainingLength - MinChunkLength * 2));
    const int32 LengthRight = RemainingLength - LengthLeft;
    OutLeft->Bounds = Chunk->Bounds;
    OutLeft->ChunkType = FloorChunkType::Room;
    OutLeft->SetLength(LengthLeft);

    OutHallway->Bounds = Chunk->Bounds;
    OutHallway->ChunkType = FloorChunkType::Hall;
    OutHallway->OffsetAlongLength(LengthLeft);
    OutHallway->SetLength(HallWidth);

    OutRight->Bounds = Chunk->Bounds;
    OutRight->ChunkType = FloorChunkType::Room;
    OutRight->OffsetAlongLength(LengthLeft + HallWidth);
    OutRight->SetLength(LengthRight);
}

void UFloorPlanBuilder::SplitChunk(FloorChunkPtr Chunk, FloorChunkPtr OutLeft, FloorChunkPtr OutRight) {
    int32 MinRoomLength = floorPlanConfig->MinRoomSize;
    const int32 Length = Chunk->GetLength();
    const int32 LengthLeft = MinRoomLength + Random.RandRange(0, FMath::Max(0, Length - MinRoomLength * 2));
    const int32 LengthRight = Length - LengthLeft;
    OutLeft->Bounds = Chunk->Bounds;
    OutLeft->ChunkType = FloorChunkType::Room;
    OutLeft->SetLength(LengthLeft);

    OutRight->Bounds = Chunk->Bounds;
    OutRight->OffsetAlongLength(LengthLeft);
    OutRight->ChunkType = FloorChunkType::Room;
    OutRight->SetLength(LengthRight);
}

bool UFloorPlanBuilder::VolumeEncompassesPoint(ADungeonVolume* Volume, const FIntVector& GridPoint) {
    FVector WorldLocation = FVector(GridPoint.X + 0.5f, GridPoint.Y + 0.5f, GridPoint.Z + 0.25f) * floorPlanConfig->
        GridSize;
    const FVector BasePosition = Dungeon ? Dungeon->GetActorLocation() : FVector::ZeroVector;
    WorldLocation += BasePosition;

    return Volume->EncompassesPoint(WorldLocation);
}

void UFloorPlanBuilder::GetVolumeCells(ADungeonVolume* Volume, int32 z, TArray<FIntVector>& OutCells) {
    FVector GridSize = floorPlanConfig->GridSize;
    FRectangle VolumeBounds;
    FTransform DungeonInverseTransform = Dungeon ? Dungeon->GetActorTransform().Inverse() : FTransform::Identity;

    //Volume->GetDungeonVolumeBounds(GridSize, VolumeBounds);
    {
        FVector BoxOrigin, BoxExtent;
        Volume->GetActorBounds(false, BoxOrigin, BoxExtent);

        FVector Start = BoxOrigin - BoxExtent;
        FVector End = BoxOrigin + BoxExtent;
        FVector Size = End - Start;

        FVector GStart = Start / GridSize;
        GStart = DungeonInverseTransform.TransformPosition(GStart);
        VolumeBounds.Location.X = FMath::FloorToInt(GStart.X);
        VolumeBounds.Location.Y = FMath::FloorToInt(GStart.Y);
        VolumeBounds.Location.Z = FMath::FloorToInt(GStart.Z);

        FVector GSize = Size / GridSize;
        GSize = DungeonInverseTransform.TransformVector(GSize);
        VolumeBounds.Size.X = FMath::RoundToInt(GSize.X);
        VolumeBounds.Size.Y = FMath::RoundToInt(GSize.Y);
        VolumeBounds.Size.Z = FMath::RoundToInt(GSize.Z);
    }

    int32 x0 = VolumeBounds.Location.X;
    int32 y0 = VolumeBounds.Location.Y;
    int32 x1 = x0 + VolumeBounds.Size.X;
    int32 y1 = y0 + VolumeBounds.Size.Y;

    OutCells.Reset();
    FTransform DungeonTransform = Dungeon ? Dungeon->GetActorTransform() : FTransform::Identity;
    for (int x = x0; x <= x1; x++) {
        for (int y = y0; y <= y1; y++) {
            FVector WorldLocation = FVector(x, y, z) * GridSize;
            WorldLocation = DungeonTransform.TransformPosition(WorldLocation);
            if (Volume->EncompassesPoint(WorldLocation)) {
                OutCells.Add(FIntVector(x, y, z));
            }
        }
    }
}

template <typename T>
void TagActors(UWorld* World) {
    if (World) {
        for (TActorIterator<T> It(World); It; ++It) {
            T* Actor = *It;
            if (Actor->Tags.Num() == 0) {
                FString Guid = FGuid::NewGuid().ToString();
                FName Id = *Guid;
                Actor->Tags.Add(Id);
            }
        }
    }
}

FRectangle InverseTransformBounds(ADungeon* Dungeon, const FVector& GridSize, const FRectangle& Bounds) {
    if (!Dungeon) return Bounds;
    FTransform DungeonInverseTransform = Dungeon->GetActorTransform().Inverse();

    FVector Location = FMathUtils::ToVector(Bounds.Location) * GridSize;
    Location = DungeonInverseTransform.TransformPosition(Location);
    Location /= GridSize;
    FVector Size = DungeonInverseTransform.TransformVector(FMathUtils::ToVector(Bounds.Size));

    FRectangle FixedBounds;
    FixedBounds.Location = FMathUtils::ToIntVector(Location, true);
    FixedBounds.Size = FMathUtils::ToIntVector(Size, true);
    return FixedBounds;
}

void UFloorPlanBuilder::BuildLayout() {
    ChunkDB = MakeShareable(new FloorChunkDB);
    Random.Initialize(floorPlanConfig->Seed);
    UWorld* World = Dungeon ? Dungeon->GetWorld() : nullptr;

    // Tag all the volumes in the scene
    TagActors<AFloorPlanRoomVolume>(World);

    const int32 NumFloors = floorPlanConfig->BuildingSize.Z;
    for (int z = 0; z < NumFloors; z++) {
        // Build the hallways and the intermediate floor chunks (which will hold the rooms)
        TArray<FloorChunkPtr> FloorChunks;
        {
            FloorChunkPtr InitialFloor = ChunkDB->Create();
            InitialFloor->Bounds.Location = FIntVector(0, 0, z);
            InitialFloor->Bounds.Size = FMathUtils::ToIntVector(floorPlanConfig->BuildingSize, true);

            TArray<FloorChunkPtr> Stack;
            Stack.Push(InitialFloor);
            while (Stack.Num() > 0) {
                FloorChunkPtr Top = Stack.Pop();
                float Area = Top->Area();
                if (Area <= 0) continue;
                if (Top->Area() <= floorPlanConfig->MinRoomChunkArea) {
                    FloorChunks.Add(Top);
                }
                else {
                    // Needs to be split further
                    FloorChunkPtr Left = ChunkDB->Create();
                    FloorChunkPtr Right = ChunkDB->Create();
                    FloorChunkPtr Hallway = ChunkDB->Create();

                    SplitChunk(Top, Left, Right, Hallway);
                    if (Left->IsValidChunk() && Right->IsValidChunk() && Hallway->IsValidChunk()) {
                        ChunkDB->Register(Hallway);
                        Stack.Push(Left);
                        Stack.Push(Right);
                    }
                    else {
                        FloorChunks.Add(Top);
                    }
                }
            }
        }

        // Split the floor chunks (space between the hallways) to create rooms
        for (FloorChunkPtr Chunk : FloorChunks) {
            TArray<FloorChunkPtr> Stack;
            Stack.Push(Chunk);
            const int32 MinRoomSize = FMath::Min(floorPlanConfig->MinRoomSize, floorPlanConfig->MaxRoomSize);
            const int32 MaxRoomSize = FMath::Max(floorPlanConfig->MinRoomSize, floorPlanConfig->MaxRoomSize);
            
            const int32 MinArea = FMath::Square(MinRoomSize);
            const int32 MaxArea = FMath::Square(MaxRoomSize);

            while (Stack.Num() > 0) {
                FloorChunkPtr Top = Stack.Pop();
                bool bRequiresSplit = true;
                const int32 Area = Top->Area();
                const int32 Length = Top->GetLength();
                if (Length > MaxArea) {
                    // Length is too big. force a split
                    bRequiresSplit = true;
                }
                else if (Area <= MinArea) {
                    // This room is too small and should not be split
                    bRequiresSplit = false;
                }
                else if (Area <= MaxArea) {
                    float SplitProbability = (Area - MinArea) / (MaxArea - MinArea);
                    SplitProbability += floorPlanConfig->RoomSplitProbabilityOffset;
                    if (Chunk->GetLength() >= Chunk->GetWidth() * 2) {
                        SplitProbability += floorPlanConfig->RoomSplitProbabilityOffset;
                    }
                    bRequiresSplit = (Random.FRand() < SplitProbability);
                }

                if (bRequiresSplit) {
                    FloorChunkPtr Left = ChunkDB->Create();
                    FloorChunkPtr Right = ChunkDB->Create();
                    SplitChunk(Top, Left, Right);
                    Stack.Push(Left);
                    Stack.Push(Right);
                }
                else {
                    ChunkDB->Register(Top);
                }
            }
        }

        // Get the chunks defined by the room volumes
        TArray<FloorChunkPtr> RoomVolumeChunks;
        FTransform DungeonInverseTransform = Dungeon ? Dungeon->GetActorTransform().Inverse() : FTransform::Identity;

        if (World) {
            FRectangle BuildingBounds;
            BuildingBounds.Location = FIntVector::ZeroValue;
            BuildingBounds.Size = FMathUtils::ToIntVector(floorPlanConfig->BuildingSize, true);

            for (TActorIterator<AFloorPlanRoomVolume> It(World); It; ++It) {
                AFloorPlanRoomVolume* Volume = *It;
                FName VolumeId = "";
                if (Volume->Tags.Num() > 0) {
                    VolumeId = Volume->Tags[0];
                }
                if (Volume->Dungeon != Dungeon) {
                    // This volume is not supposed to affect this dungeon
                    continue;
                }
                FVector GridSize = floorPlanConfig->GridSize;
                FRectangle VolumeBounds;
                Volume->GetDungeonVolumeBounds(GridSize, VolumeBounds);
                VolumeBounds = InverseTransformBounds(Dungeon, GridSize, VolumeBounds);

                int32 VolumeStartZ = VolumeBounds.Location.Z;
                int32 VolumeEndZ = VolumeStartZ + VolumeBounds.Size.Z;

                if (z < VolumeStartZ || z >= VolumeEndZ) {
                    // Out of the volume bounds
                    continue;
                }

                FRectangle ChunkBounds = VolumeBounds;
                ChunkBounds.Location.Z = z;
                ChunkBounds.Size.Z = 1;
                ChunkBounds.Clip(BuildingBounds);
                if (ChunkBounds.Size.Z == 0) continue;
                int32 ChunkZ = ChunkBounds.Location.Z;

                FloorChunkPtr RoomVolumeChunk = ChunkDB->Create();
                RoomVolumeChunk->Bounds = ChunkBounds;
                RoomVolumeChunk->ChunkType = FloorChunkType::Room;
                RoomVolumeChunk->bEmitGroundMarker = (ChunkZ == VolumeStartZ);
                RoomVolumeChunk->bEmitCeilingMarker = (ChunkZ == VolumeEndZ - 1);
                RoomVolumeChunk->bConnectDoors = Volume->bConnectDoors;
                RoomVolumeChunk->WallMarker = Volume->WallMarker;
                RoomVolumeChunk->GroundMarker = Volume->GroundMarker;
                RoomVolumeChunk->CeilingMarker = Volume->CeilingMarker;
                RoomVolumeChunk->DoorMarker = Volume->DoorMarker;
                RoomVolumeChunk->CenterMarker = Volume->CenterMarker;
                RoomVolumeChunk->PerFloorCenterMarker = Volume->PerFloorCenterMarker;
                RoomVolumeChunk->bCreateWalls = Volume->bCreateWalls;
                RoomVolumeChunk->VolumeId = VolumeId;

                // Since this chunk is defined by a volume, we can have a more detailed rasterized cell shape
                GetVolumeCells(Volume, z, RoomVolumeChunk->BoundCells);

                // Give a higher priority to the volume chunk so it overrides everything in its path
                RoomVolumeChunk->Priority = Volume->Priority;

                RoomVolumeChunks.Add(RoomVolumeChunk);
                ChunkDB->Register(RoomVolumeChunk);
            }
        }
    }

    ChunkDB->CacheChunkPositions();

    for (int z = 0; z < floorPlanConfig->BuildingSize.Z; z++) {
        CreateDoors(z);
    }
}

namespace {
    const FString MARKER_GROUND = "Ground";
    const FString MARKER_CEILING = "Ceiling";
    const FString MARKER_WALL = "Wall";
    const FString MARKER_WALL_SEPARATOR = "WallSeparator";
    const FString MARKER_DOOR = "Door";
    const FString MARKER_BUILDING_WALL = "BuildingWall";
    const FString MARKER_BUILDING_WALL_SEPARATOR = "BuildingWallSeparator";
}

FString GetDoorMarkerName(FloorChunkPtr ChunkA, FloorChunkPtr ChunkB) {
    if (!ChunkA.IsValid() && !ChunkB.IsValid()) {
        return MARKER_DOOR;
    }

    FloorChunkPtr PreferedChunk;
    if (!ChunkA.IsValid()) {
        PreferedChunk = ChunkB;
    }
    else if (!ChunkB.IsValid()) {
        PreferedChunk = ChunkA;
    }
    else {
        PreferedChunk = (ChunkA->Priority > ChunkB->Priority) ? ChunkA : ChunkB;
    }

    return PreferedChunk->DoorMarker.Len() > 0 ? PreferedChunk->DoorMarker : MARKER_DOOR;
}

FloorChunkPtr GetPriorityChunk(FloorChunkPtr A, FloorChunkPtr B) {
    if (!A.IsValid()) return B;
    if (!B.IsValid()) return A;
    return A->Priority > B->Priority ? A : B;
}


void UFloorPlanBuilder::EmitDungeonMarkers_Implementation() {
    Super::EmitDungeonMarkers_Implementation();

    floorPlanModel = Cast<UFloorPlanModel>(model);
    floorPlanConfig = Cast<UFloorPlanConfig>(config);

    const FVector& GridSize = floorPlanConfig->GridSize;

    TArray<AFloorPlanDoorVolume*> DoorVolumes = UDungeonModelHelper::GetVolumes<AFloorPlanDoorVolume>(Dungeon);

    // Rasterize the Room volume in 3d grid space so we can check if ground / ceiling is required
    UWorld* World = Dungeon ? Dungeon->GetWorld() : nullptr;
    TArray<AFloorPlanRoomVolume*> RoomVolumes = UDungeonModelHelper::GetActors<AFloorPlanRoomVolume>(World);
    // TODO: Sort the rooms based on priority before rasterizing to correctly override overlapping volumes

    TSharedPtr<FVolumeQuery> VolumeQuery = MakeShareable(new FSpatialCacheVolumeQuery(GridSize, Dungeon));
    VolumeQuery->Initialize(RoomVolumes);

    FTransform DungeonTransform = Dungeon ? Dungeon->GetActorTransform() : FTransform::Identity;

    ClearSockets();
    const int32 NumFloors = floorPlanConfig->BuildingSize.Z;
    bool bBuildingWallLeft, bBuildingWallBottom;
    for (int z = 0; z < NumFloors; z++) {
        for (int x = -1; x < floorPlanConfig->BuildingSize.X; x++) {
            bBuildingWallLeft = (x == -1 || x == floorPlanConfig->BuildingSize.X - 1);
            for (int y = -1; y < floorPlanConfig->BuildingSize.Y; y++) {
                bBuildingWallBottom = (y == -1 || y == floorPlanConfig->BuildingSize.Y - 1);
                FloorChunkPtr Chunk00 = ChunkDB->GetChunkAt(x, y, z);
                FloorChunkPtr Chunk10 = ChunkDB->GetChunkAt(x + 1, y, z);
                FloorChunkPtr Chunk01 = ChunkDB->GetChunkAt(x, y + 1, z);

                FString GroundMarkerName = MARKER_GROUND;
                FString CeilingMarkerName = MARKER_CEILING;

                FName VolumeIdCurrent = Chunk00.IsValid() ? Chunk00->VolumeId : FName();
                /*
                int32 AboveHash = GetTypeHash(FIntVector(x, y, z + 1));
                int32 BelowHash = GetTypeHash(FIntVector(x, y, z - 1));
                FName VolumeIdAbove = LocationToVolumeMap.Contains(AboveHash) ? LocationToVolumeMap[AboveHash] : FName();
                FName VolumeIdBelow = LocationToVolumeMap.Contains(BelowHash) ? LocationToVolumeMap[BelowHash] : FName();
                */

                FVector QueryLocationAbove = FVector(x, y, z + 1) * GridSize;
                FVector QueryLocationBelow = FVector(x, y, z - 1) * GridSize;
                QueryLocationAbove = DungeonTransform.TransformPosition(QueryLocationAbove);
                QueryLocationBelow = DungeonTransform.TransformPosition(QueryLocationBelow);
                FName VolumeIdAbove = VolumeQuery->GetVolumeIdAt(QueryLocationAbove);
                FName VolumeIdBelow = VolumeQuery->GetVolumeIdAt(QueryLocationBelow);

                bool bEmitGroundMarker = true;
                bool bEmitCeilingMarker = true;
                if (!VolumeIdCurrent.IsNone() && VolumeIdCurrent == VolumeIdBelow) {
                    bEmitGroundMarker = false;
                }
                if (!VolumeIdCurrent.IsNone() && VolumeIdCurrent == VolumeIdAbove) {
                    bEmitCeilingMarker = false;
                }

                // Emit the ground marker
                if (Chunk00.IsValid() && Chunk00->ChunkType != FloorChunkType::Outside) {
                    if (bEmitGroundMarker) {
                        FVector GridLocation(x + 0.5f, y + 0.5f, z);
                        FVector WorldLocation = GridLocation * GridSize;
                        if (Chunk00->GroundMarker.Len() > 0) {
                            GroundMarkerName = Chunk00->GroundMarker;
                        }
                        EmitMarkerAt(WorldLocation, GroundMarkerName, 0);
                    }
                    if (bEmitCeilingMarker) {
                        FVector GridLocation(x + 0.5f, y + 0.5f, z + 1);
                        FVector WorldLocation = GridLocation * GridSize;
                        if (Chunk00->CeilingMarker.Len() > 0) {
                            CeilingMarkerName = Chunk00->CeilingMarker;
                        }
                        EmitMarkerAt(WorldLocation, CeilingMarkerName, FQuat::MakeFromEuler(FVector(180, 0, 0)));
                    }
                }

                int32 Chunk00Id = (Chunk00.IsValid() ? Chunk00->Id : -1);
                int32 Chunk10Id = (Chunk10.IsValid() ? Chunk10->Id : -1);
                int32 Chunk01Id = (Chunk01.IsValid() ? Chunk01->Id : -1);

                bool bEmitLeftWall = (Chunk00Id != Chunk10Id);
                bool bEmitBottomWall = (Chunk00Id != Chunk01Id);
                bool bLeftDoor = DoorManager.ContainsDoor(FIntVector(x, y, z), FIntVector(x + 1, y, z)) ||
                    DoorManager.ContainsDoorVolume(FVector(x + 1, y + 0.5f, z) * GridSize, DoorVolumes);
                bool bBottomDoor = DoorManager.ContainsDoor(FIntVector(x, y, z), FIntVector(x, y + 1, z)) ||
                    DoorManager.ContainsDoorVolume(FVector(x + 0.5f, y + 1, z) * GridSize, DoorVolumes);

                if (Chunk00.IsValid() && Chunk10.IsValid() && Chunk00->ChunkType == FloorChunkType::Hall && Chunk10->
                    ChunkType == FloorChunkType::Hall) {
                    // Do not block the halls with a wall
                    bEmitLeftWall = false;
                }
                if (Chunk00.IsValid() && Chunk01.IsValid() && Chunk00->ChunkType == FloorChunkType::Hall && Chunk01->
                    ChunkType == FloorChunkType::Hall) {
                    // Do not block the halls with a wall
                    bEmitBottomWall = false;
                }

                if (Chunk00.IsValid() && Chunk10.IsValid() && (!Chunk00->bEmitGroundMarker || !Chunk10->
                    bEmitGroundMarker)) {
                    // We don't have ground in one of the adjacent chunks. Can't have doors
                    bLeftDoor = false;
                }
                if (Chunk00.IsValid() && Chunk01.IsValid() && (!Chunk00->bEmitGroundMarker || !Chunk01->
                    bEmitGroundMarker)) {
                    // We don't have ground in one of the adjacent chunks. Can't have doors
                    bBottomDoor = false;
                }

                float AngleLeft = 90;
                if (bEmitLeftWall) {
                    FloorChunkPtr PriorityChunk = GetPriorityChunk(Chunk00, Chunk10);
                    bEmitLeftWall = PriorityChunk.IsValid() ? PriorityChunk->bCreateWalls : true;
                    AngleLeft = -90;
                    if (bEmitLeftWall && PriorityChunk == Chunk10) {
                        AngleLeft = 90;
                    }
                }
                float AngleBottom = 0;
                if (bEmitBottomWall) {
                    FloorChunkPtr PriorityChunk = GetPriorityChunk(Chunk00, Chunk01);
                    bEmitBottomWall = PriorityChunk.IsValid() ? PriorityChunk->bCreateWalls : true;
                    AngleBottom = 0;
                    if (bEmitBottomWall && PriorityChunk == Chunk01) {
                        AngleBottom = 180;
                    }
                }

                FString DoorMarkerName = MARKER_DOOR;
                if (Chunk00.IsValid() && Chunk00->DoorMarker.Len() > 0) {
                    DoorMarkerName = Chunk00->DoorMarker;
                }

                if (bEmitLeftWall) {
                    FVector GridLocation(x + 1, y + 0.5f, z);
                    FVector WorldLocation = GridLocation * GridSize;

                    FString MarkerName;
                    if (bLeftDoor) {
                        MarkerName = GetDoorMarkerName(Chunk00, Chunk10);
                    }
                    else {
                        MarkerName = MARKER_WALL;
                        if (bBuildingWallLeft) {
                            MarkerName = MARKER_BUILDING_WALL;
                        }
                        else {
                            if (Chunk00.IsValid() && Chunk10.IsValid()) {
                                FloorChunkPtr PriorityChunk = (Chunk00->Priority > Chunk10->Priority)
                                                                  ? Chunk00
                                                                  : Chunk10;
                                if (PriorityChunk->WallMarker.Len() > 0) {
                                    MarkerName = PriorityChunk->WallMarker;
                                }
                            }
                        }
                    }

                    EmitMarkerAt(WorldLocation, MarkerName, AngleLeft);
                }
                if (bEmitBottomWall) {
                    FVector GridLocation(x + 0.5f, y + 1, z);
                    FVector WorldLocation = GridLocation * GridSize;

                    FString MarkerName;
                    if (bBottomDoor) {
                        MarkerName = GetDoorMarkerName(Chunk00, Chunk01);
                    }
                    else {
                        MarkerName = MARKER_WALL;
                        if (bBuildingWallBottom) {
                            MarkerName = MARKER_BUILDING_WALL;
                        }
                        else {
                            if (Chunk00.IsValid() && Chunk01.IsValid()) {
                                FloorChunkPtr PriorityChunk = (Chunk00->Priority > Chunk01->Priority)
                                                                  ? Chunk00
                                                                  : Chunk01;
                                if (PriorityChunk->WallMarker.Len() > 0) {
                                    MarkerName = PriorityChunk->WallMarker;
                                }
                            }
                        }
                    }

                    EmitMarkerAt(WorldLocation, MarkerName, AngleBottom);
                }
            }
        }
    }


    // Emit center marker if specified
    TArray<FloorChunkPtr> Chunks;
    ChunkDB->GetChunks(Chunks);
    for (FloorChunkPtr Chunk : Chunks) {
        FString CenterMarker = Chunk->CenterMarker;
        FString PerFloorCenterMarker = Chunk->PerFloorCenterMarker;

        if (CenterMarker.Len() > 0 || PerFloorCenterMarker.Len() > 0) {
            FVector ChunkSize = FMathUtils::ToVector(Chunk->Bounds.Size) / 2.0f;
            ChunkSize.Z = 0;
            FVector GridLocation = FMathUtils::ToVector(Chunk->Bounds.Location) + ChunkSize;
            FVector WorldLocation = GridLocation * GridSize;

            if (Chunk->bEmitGroundMarker && CenterMarker.Len() > 0) {
                EmitMarkerAt(WorldLocation, CenterMarker, 0);
            }
            if (PerFloorCenterMarker.Len() > 0) {
                EmitMarkerAt(WorldLocation, PerFloorCenterMarker, 0);
            }
        }
    }

    // Emit Wall separators
    EmitSeparatorMarkers(MARKER_WALL, MARKER_WALL_SEPARATOR, GridSize);
    EmitSeparatorMarkers(MARKER_BUILDING_WALL, MARKER_BUILDING_WALL_SEPARATOR, GridSize);
}

void UFloorPlanBuilder::EmitMarkerAt(const FVector& WorldLocation, const FString& MarkerName, float Angle) {
    FQuat Rotation = FQuat::MakeFromEuler(FVector(0, 0, Angle));
    EmitMarkerAt(WorldLocation, MarkerName, Rotation);
}


void UFloorPlanBuilder::EmitMarkerAt(const FVector& WorldLocation, const FString& MarkerName, const FQuat& Rotation) {
    FTransform Transform = FTransform::Identity;
    Transform.SetLocation(WorldLocation);
    Transform.SetRotation(Rotation);

    if (Dungeon) {
        // Transform the marker relative to the dungeon
        Transform = Transform * Dungeon->GetActorTransform();
    }

    EmitMarker(MarkerName, Transform);
}

void UFloorPlanBuilder::EmitSeparatorMarkers(const FString& WallMarkerName, const FString& SepratorMarkerName,
                                             const FVector& GridSize) {
    TArray<FTransform> WallTransforms;

    for (const FPropSocket& Marker : PropSockets) {
        if (Marker.SocketType == WallMarkerName) {
            WallTransforms.Add(Marker.Transform);
        }
    }

    FVector BaseOffset(-GridSize.X / 2.0f, 0, 0);
    for (const FTransform& WallTransform : WallTransforms) {
        FVector Offset = WallTransform.GetRotation() * BaseOffset;
        FVector SeparatorLocation = WallTransform.GetLocation() + Offset;
        FTransform SepratorTransform = WallTransform;
        SepratorTransform.SetLocation(SeparatorLocation);

        EmitMarker(SepratorMarkerName, SepratorTransform);
    }
}

void DrawChunk(FloorChunkPtr Chunk, const FVector& GridSize, UWorld* InWorld, bool bPersistant, float lifeTime) {
    FColor Color = (Chunk->ChunkType == FloorChunkType::Hall) ? FColor::Yellow : FColor::Green;
    FVector Location = FMathUtils::ToVector(Chunk->Bounds.Location) * GridSize;
    FVector Size = FMathUtils::ToVector(Chunk->Bounds.Size) * GridSize;
    Location += Size / 2.0f; // Center the location for the drawing api
    DrawDebugSolidBox(InWorld, Location, Size / 2.0f, Color.WithAlpha(4), bPersistant, lifeTime);
    DrawDebugBox(InWorld, Location, Size / 2.0f, Color, bPersistant, lifeTime);
}

void UFloorPlanBuilder::DrawDebugData(UWorld* InWorld, bool bPersistant /*= false*/, float lifeTime /*= 0*/) {
    if (!ChunkDB.IsValid()) return;

    const FVector GridSize = floorPlanConfig->GridSize;
    TArray<FloorChunkPtr> Chunks;
    ChunkDB->GetChunks(Chunks);

    for (FloorChunkPtr Chunk : Chunks) {
        DrawChunk(Chunk, GridSize, InWorld, bPersistant, lifeTime);
    }
}

TSubclassOf<UDungeonModel> UFloorPlanBuilder::GetModelClass() {
    return UFloorPlanModel::StaticClass();
}

TSubclassOf<UDungeonConfig> UFloorPlanBuilder::GetConfigClass() {
    return UFloorPlanConfig::StaticClass();
}

TSubclassOf<UDungeonToolData> UFloorPlanBuilder::GetToolDataClass() {
    return UFloorPlanToolData::StaticClass();
}

TSubclassOf<UDungeonQuery> UFloorPlanBuilder::GetQueryClass() {
    return UFloorPlanQuery::StaticClass();
}

bool UFloorPlanBuilder::ProcessSpatialConstraint(UDungeonSpatialConstraint* SpatialConstraint,
                                                 const FTransform& Transform, FQuat& OutRotationOffset) {
    return false;
}

void UFloorPlanBuilder::GetDefaultMarkerNames(TArray<FString>& OutMarkerNames) {
    OutMarkerNames.Reset();
    OutMarkerNames.Add(MARKER_GROUND);
    OutMarkerNames.Add(MARKER_CEILING);
    OutMarkerNames.Add(MARKER_WALL);
    OutMarkerNames.Add(MARKER_WALL_SEPARATOR);
    OutMarkerNames.Add(MARKER_DOOR);
    OutMarkerNames.Add(MARKER_BUILDING_WALL);
    OutMarkerNames.Add(MARKER_BUILDING_WALL_SEPARATOR);
}

bool UFloorPlanBuilder::PerformSelectionLogic(const TArray<UDungeonSelectorLogic*>& SelectionLogics,
                                              const FPropSocket& socket) {
    for (UDungeonSelectorLogic* SelectionLogic : SelectionLogics) {
        UFloorPlanSelectorLogic* FloorSelectionLogic = Cast<UFloorPlanSelectorLogic>(SelectionLogic);
        if (!FloorSelectionLogic) {
            UE_LOG(FloorPlanBuilderLog, Warning,
                   TEXT("Invalid selection logic specified.  FloorPlanSelectorLogic expected"));
            return false;
        }

        // Perform blueprint based selection logic
        FVector Location = socket.Transform.GetLocation();
        FVector GridSize = floorPlanConfig->GridSize;
        int32 GridX = FMath::FloorToInt(Location.X / GridSize.X);
        int32 GridY = FMath::FloorToInt(Location.Y / GridSize.Y);
        int32 GridZ = FMath::FloorToInt(Location.Z / GridSize.Z);
        FloorChunkPtr Chunk = ChunkDB->GetChunkAt(GridX, GridY, GridZ);
        bool selected = FloorSelectionLogic->SelectNode(floorPlanModel, floorPlanConfig, Random, GridX, GridY, GridZ);
        if (!selected) {
            return false;
        }
    }
    return true;
}

FTransform UFloorPlanBuilder::PerformTransformLogic(const TArray<UDungeonTransformLogic*>& TransformLogics,
                                                    const FPropSocket& socket) {
    FTransform result = FTransform::Identity;

    for (UDungeonTransformLogic* TransformLogic : TransformLogics) {
        UFloorPlanTransformLogic* FloorTransformLogic = Cast<UFloorPlanTransformLogic>(TransformLogic);
        if (!FloorTransformLogic) {
            UE_LOG(FloorPlanBuilderLog, Warning,
                   TEXT("Invalid transform logic specified.  FloorPlanTransformLogic expected"));
            continue;
        }

        FVector Location = socket.Transform.GetLocation();
        FVector GridSize = floorPlanConfig->GridSize;
        int32 GridX = FMath::FloorToInt(Location.X / GridSize.X);
        int32 GridY = FMath::FloorToInt(Location.Y / GridSize.Y);
        FTransform LogicOffset;
        if (TransformLogic) {
            FloorTransformLogic->GetNodeOffset(floorPlanModel, floorPlanConfig, Random, GridX, GridY, LogicOffset);
        }
        else {
            LogicOffset = FTransform::Identity;
        }

        FTransform out;
        FTransform::Multiply(&out, &LogicOffset, &result);
        result = out;
    }
    return result;

}

//////////////////////////// Door connection //////////////////////////
int32 GetChunkDoorConnectionScore(FloorChunkPtr Chunk) {
    if (!Chunk.IsValid()) return -1000;
    if (Chunk->bReachable) return -500;
    if (!Chunk->bConnectDoors) return -1000;
    if (Chunk->ChunkType == FloorChunkType::Hall) return 1000;
    if (Chunk->ChunkType == FloorChunkType::Room) return 500;
    return 0;
}

bool FloorChunkPriorityPredicate(FloorChunkPtr ChunkA, FloorChunkPtr ChunkB) {
    return GetChunkDoorConnectionScore(ChunkA) > GetChunkDoorConnectionScore(ChunkB);
}

struct FloorIslandNode {
    FloorIslandNode() : IslandId(-1), Location(FIntVector::ZeroValue) {
    }

    int32 IslandId;
    FloorChunkPtr Chunk;
    FIntVector Location;
};

typedef TArray<FloorIslandNode> FloorIslandNodeList;

void FloodFill(const FIntVector& StartLocation, int32 IslandId, TSet<FIntVector>& Visited,
               FloorIslandNodeList& IslandNodes, FloorChunkDBPtr ChunkDB) {

    FloorChunkPtr PreferedChunk = ChunkDB->GetChunkAt(StartLocation);
    if (!PreferedChunk->bConnectDoors) {
        // We don't want doors here
        return;
    }
    if (!PreferedChunk.IsValid()) {
        return;
    }

    TQueue<FIntVector> Queue;
    Queue.Enqueue(StartLocation);

    while (!Queue.IsEmpty()) {
        FIntVector Location;
        Queue.Dequeue(Location);

        FloorChunkPtr CurrentChunk = ChunkDB->GetChunkAt(Location);
        if (CurrentChunk != PreferedChunk) {
            continue;
        }

        // Create a node here
        FloorIslandNode Node;
        Node.IslandId = IslandId;
        Node.Chunk = CurrentChunk;
        Node.Location = Location;

        IslandNodes.Add(Node);

        // Add the neighbors to the queue
        TArray<FIntVector> Neighbors;
        Neighbors.Add(Location + FIntVector(-1, 0, 0));
        Neighbors.Add(Location + FIntVector(1, 0, 0));
        Neighbors.Add(Location + FIntVector(0, 1, 0));
        Neighbors.Add(Location + FIntVector(0, -1, 0));

        for (const FIntVector& Neighbor : Neighbors) {
            if (Visited.Contains(Neighbor)) {
                continue;
            }
            FloorChunkPtr NeighborChunk = ChunkDB->GetChunkAt(Neighbor);
            if (NeighborChunk.IsValid() && NeighborChunk->Id == CurrentChunk->Id) {
                Queue.Enqueue(Neighbor);
                Visited.Add(Neighbor);
            }
        }
    }
}

struct FloorIslandAdjacency {
    FloorIslandNode A;
    FloorIslandNode B;
};

struct IslandNodePriorityPredicate {
    IslandNodePriorityPredicate(TMap<int32, FloorChunkPtr> InIslandToChunkMap) : IslandToChunkMap(InIslandToChunkMap) {
    }

    bool operator()(const int32& IslandA, const int32& IslandB) const {
        FloorChunkPtr ChunkA = IslandToChunkMap.Contains(IslandA) ? IslandToChunkMap[IslandA] : nullptr;
        FloorChunkPtr ChunkB = IslandToChunkMap.Contains(IslandB) ? IslandToChunkMap[IslandB] : nullptr;

        return GetChunkDoorConnectionScore(ChunkA) > GetChunkDoorConnectionScore(ChunkB);
    }

    TMap<int32, FloorChunkPtr> IslandToChunkMap;

};

void ConnectIslandRecursive(int32 IslandId, const TMap<int32, TArray<FloorIslandAdjacency>>& AdjacencyByIslands,
                            TSet<int32>& IslandVisited, FRandomStream& Random, FloorDoorManager& DoorManager,
                            TMap<int32, FloorChunkPtr> IslandToChunkMap) {
    if (IslandVisited.Contains(IslandId)) {
        return;
    }
    IslandVisited.Add(IslandId);

    if (!AdjacencyByIslands.Contains(IslandId)) {
        // No adjacent islands
        return;
    }

    const TArray<FloorIslandAdjacency>& AdjacentNodes = AdjacencyByIslands[IslandId];
    TSet<int32> AdjacentIslands;
    for (const FloorIslandAdjacency& AdjacentNode : AdjacentNodes) {
        AdjacentIslands.Add(AdjacentNode.A.IslandId);
        AdjacentIslands.Add(AdjacentNode.B.IslandId);
    }
    AdjacentIslands.Remove(IslandId);
    IslandNodePriorityPredicate SortPredicate(IslandToChunkMap);
    AdjacentIslands.Sort(SortPredicate);
    for (int32 AdjacentIsland : AdjacentIslands) {
        if (IslandVisited.Contains(AdjacentIsland)) {
            continue;
        }

        // Find all the adjacent cells between these two islands
        TArray<FloorIslandAdjacency> EdgeNodes;
        for (const FloorIslandAdjacency& AdjacentNode : AdjacentNodes) {
            if (AdjacentNode.A.IslandId == AdjacentIsland || AdjacentNode.B.IslandId == AdjacentIsland) {
                EdgeNodes.Add(AdjacentNode);
            }
        }

        // Connect a door in any one of the edge nodes
        if (EdgeNodes.Num() > 0) {
            int Index = Random.RandRange(0, EdgeNodes.Num() - 1);
            FloorIslandAdjacency DoorEdge = EdgeNodes[Index];
            // Create a door here
            DoorManager.RegisterDoor(DoorEdge.A.Location, DoorEdge.B.Location);

            // Move into this room now
            ConnectIslandRecursive(AdjacentIsland, AdjacencyByIslands, IslandVisited, Random, DoorManager,
                                   IslandToChunkMap);
        }
    }
}

void UFloorPlanBuilder::CreateDoors(int z) {
    // Tag all islands
    // Create adjacency list
    // Do a DFS on the tagged islands and connect the islands with doors

    FloorIslandNodeList IslandNodes;
    int32 TotalIslands = 0;

    // Tag islands with a flood fill.  This helps if custom volume split existing
    // rooms into multiple parts and needs to be treated separately (islands)
    {
        TSet<FIntVector> FFVisited;
        int32 IslandId = 0;
        for (int32 x = 0; x < floorPlanConfig->BuildingSize.X; x++) {
            for (int32 y = 0; y < floorPlanConfig->BuildingSize.Y; y++) {
                FIntVector Location(x, y, z);
                if (!FFVisited.Contains(Location)) {
                    // Flood fill from here
                    FFVisited.Add(Location);
                    FloodFill(Location, IslandId, FFVisited, IslandNodes, ChunkDB);
                    IslandId++;
                }
            }
        }
        TotalIslands = IslandId;
    }

    // Create a node map for faster access
    TMap<FIntVector, FloorIslandNode> IslandNodeByLocation;
    for (const FloorIslandNode& Node : IslandNodes) {
        if (Node.IslandId == -1) continue;
        IslandNodeByLocation.Add(Node.Location, Node);
    }

    // Create adjacency list for each island
    TArray<FloorIslandAdjacency> AdjacencyList;
    for (int32 x = 0; x < floorPlanConfig->BuildingSize.X; x++) {
        for (int32 y = 0; y < floorPlanConfig->BuildingSize.Y; y++) {
            FIntVector Loc00 = FIntVector(x, y, z);
            if (!IslandNodeByLocation.Contains(Loc00)) {
                continue;
            }
            const FloorIslandNode& Node00 = IslandNodeByLocation[Loc00];
            if (Node00.IslandId == -1) {
                continue;
            }

            // Test along the left cell
            {
                FIntVector Loc10 = FIntVector(x + 1, y, z);
                if (IslandNodeByLocation.Contains(Loc10)) {
                    const FloorIslandNode& Node10 = IslandNodeByLocation[Loc10];
                    if (Node10.IslandId != -1 && Node00.IslandId != Node10.IslandId) {
                        // Different adjacent nodes.  Add to the list
                        FloorIslandAdjacency Adjacency;
                        Adjacency.A = Node00;
                        Adjacency.B = Node10;
                        AdjacencyList.Add(Adjacency);
                    }
                }
            }

            // Test along the bottom cell
            {
                FIntVector Loc01 = FIntVector(x, y + 1, z);
                if (IslandNodeByLocation.Contains(Loc01)) {
                    const FloorIslandNode& Node01 = IslandNodeByLocation[Loc01];
                    if (Node01.IslandId != -1 && Node00.IslandId != Node01.IslandId) {
                        // Different adjacent nodes.  Add to the list
                        FloorIslandAdjacency Adjacency;
                        Adjacency.A = Node00;
                        Adjacency.B = Node01;
                        AdjacencyList.Add(Adjacency);
                    }
                }
            }
        }
    }

    // Create another lookup for faster access
    TMap<int32, TArray<FloorIslandAdjacency>> AdjacencyByIsland;
    for (const FloorIslandAdjacency& Adjacency : AdjacencyList) {
        int32 IslandA = Adjacency.A.IslandId;
        int32 IslandB = Adjacency.B.IslandId;
        if (!AdjacencyByIsland.Contains(IslandA)) AdjacencyByIsland.Add(IslandA, TArray<FloorIslandAdjacency>());
        if (!AdjacencyByIsland.Contains(IslandB)) AdjacencyByIsland.Add(IslandB, TArray<FloorIslandAdjacency>());

        AdjacencyByIsland[IslandA].Add(Adjacency);
        AdjacencyByIsland[IslandB].Add(Adjacency);
    }

    TMap<int32, FloorChunkPtr> IslandToChunkMap;
    for (const FloorIslandNode& IslandNode : IslandNodes) {
        if (IslandToChunkMap.Contains(IslandNode.IslandId)) {
            continue;
        }
        IslandToChunkMap.Add(IslandNode.IslandId, IslandNode.Chunk);
    }

    // Connect the islands to the main network with doors
    TSet<int32> IslandVisited;
    for (int IslandId = 0; IslandId < TotalIslands; IslandId++) {
        ConnectIslandRecursive(IslandId, AdjacencyByIsland, IslandVisited, Random, DoorManager, IslandToChunkMap);
    }
}

