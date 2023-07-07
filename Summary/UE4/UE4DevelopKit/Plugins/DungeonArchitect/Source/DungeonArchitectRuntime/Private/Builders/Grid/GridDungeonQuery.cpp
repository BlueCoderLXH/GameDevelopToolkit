//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/Grid/GridDungeonQuery.h"

#include "Core/Utils/DungeonModelHelper.h"
#include "Core/Utils/MathUtils.h"

#include "Algo/Reverse.h"
#include "Containers/Queue.h"

DEFINE_LOG_CATEGORY(LogGridDungeonQuery);

void UGridDungeonQuery::InitializeImpl(UDungeonConfig* InConfig, UDungeonModel* InModel) {
    Config = Cast<UGridDungeonConfig>(InConfig);
    Model = Cast<UGridDungeonModel>(InModel);

    if (!Config) {
        UE_LOG(LogGridDungeonQuery, Error, TEXT("Invalid config passed to grid query object"));
    }
    if (!Model) {
        UE_LOG(LogGridDungeonQuery, Error, TEXT("Invalid model passed to grid query object"));
    }
}

TArray<int32> UGridDungeonQuery::GetCells() {
    TArray<int32> Result;
    if (Model) {
        for (const FCell& Cell : Model->Cells) {
            if (Cell.CellType != FCellType::Unknown) {
                Result.Add(Cell.Id);
            }
        }
    }
    return Result;
}

TArray<int32> UGridDungeonQuery::GetCellsOfType(FCellType CellType) {
    TArray<int32> Result;
    if (Model) {
        for (const FCell& Cell : Model->Cells) {
            if (Cell.CellType == CellType) {
                Result.Add(Cell.Id);
            }
        }
    }
    else {
        UE_LOG(LogGridDungeonQuery, Error, TEXT("Invalid query object state"));
    }
    return Result;
}

void UGridDungeonQuery::GetCellDimension(int32 CellId, FVector& OutCenter, FVector& OutSize) {
    if (Model && Config) {
        FVector GridSize = Config->GridCellSize;
        FCell Cell = GetCell(CellId);
        if (Cell.CellType == FCellType::Unknown) {
            UE_LOG(LogGridDungeonQuery, Error, TEXT("Cannot find cell dimensions. Invalid cell state"));
            OutCenter = FVector::ZeroVector;
            OutSize = FVector::ZeroVector;
        }
        else {
            FVector LocationWorld = FMathUtils::ToVector(Cell.Bounds.Location) * GridSize;
            FVector SizeWorld = FMathUtils::ToVector(Cell.Bounds.Size) * GridSize;

            OutCenter = LocationWorld + SizeWorld / 2.0f;
            OutSize = SizeWorld;
        }
    }
    else {
        UE_LOG(LogGridDungeonQuery, Error, TEXT("Invalid query object state"));
        OutCenter = FVector::ZeroVector;
        OutSize = FVector::ZeroVector;
    }
}

FVector GetCellCenter(const FCell& Cell, const FVector& GridSize) {
    return (FMathUtils::ToVector(Cell.Bounds.Location) + FMathUtils::ToVector(Cell.Bounds.Size) / 2.0f) * GridSize;
}

void UGridDungeonQuery::GetPathBetweenCells(int32 CellA, int32 CellB, TArray<int32>& OutResult, bool& bOutSuccess) {
    // Tracks the movement of the path from one node to the next
    // This is used to reconstruct the path when we are done with BFS
    // Mapping: A -> B
    // Here, A was discovered by B, i.e. B moved to A
    TMap<int32, int32> DiscoveryMap;

    TMap<int32, FCell> CellLookup;
    GenerateCellLookup(CellLookup);

    // Perform a BFS to find the shortest path
    TQueue<int32> Queue; // Node info contains cell id
    TSet<int32> Visited; // Tracks the visited cell id

    Queue.Enqueue(CellA);
    Visited.Add(CellA);

    while (!Queue.IsEmpty()) {
        int32 TopCellId;
        Queue.Dequeue(TopCellId);

        if (!CellLookup.Contains(TopCellId)) {
            UE_LOG(LogGridDungeonQuery, Error, TEXT("GetPathBetweenCells: Invalid cell id"));
            continue;
        }

        const FCell& Cell = CellLookup[TopCellId];
        FVector GridSize = Config->GridCellSize;
        FVector CellCenter = GetCellCenter(Cell, GridSize);
        TArray<int32> AdjacentCells = Cell.AdjacentCells;
        AdjacentCells.Sort([&](int32 CellIdA, int32 CellIdB) {
            const FCell& AdjCellA = CellLookup[CellIdA];
            const FCell& AdjCellB = CellLookup[CellIdB];

            float DistASq = (CellCenter - GetCellCenter(AdjCellA, GridSize)).SizeSquared();
            float DistBSq = (CellCenter - GetCellCenter(AdjCellB, GridSize)).SizeSquared();
            return DistASq < DistBSq;
        });

        for (const int32 AdjacentCellId : AdjacentCells) {
            if (Visited.Contains(AdjacentCellId)) {
                continue;
            }

            if (!CellLookup.Contains(AdjacentCellId)) {
                UE_LOG(LogGridDungeonQuery, Error, TEXT("GetPathBetweenCells: Invalid child cell id"));
                continue;
            }
            const FCell& AdjacentCell = CellLookup[AdjacentCellId];

            // Make sure we have a valid connection to this adjacent node (i.e. no walls between it, stairs if height is different etc)
            bool bChildValid = true;

            // If any of the cells is a room, then there needs to be a door between them
            if (Cell.CellType == FCellType::Room || AdjacentCell.CellType == FCellType::Room) {
                if (!ContainsDoorBetween(Cell.Id, AdjacentCellId)) {
                    bChildValid = false;
                }
            }

            // If there is a height difference, make sure we have stairs between them
            if (Cell.Bounds.Location.Z != AdjacentCell.Bounds.Location.Z) {
                if (!ContainsStairBetween(Cell.Id, AdjacentCellId)) {
                    bChildValid = false;
                }
            }

            if (bChildValid) {
                Queue.Enqueue(AdjacentCellId);
                Visited.Add(AdjacentCellId);

                // Register the discovery path
                if (!DiscoveryMap.Contains(AdjacentCellId)) {
                    DiscoveryMap.Add(AdjacentCellId, TopCellId);
                }
            }
        }
    }

    // Build the path from the discovery list
    Visited.Reset();
    OutResult.Reset();
    bOutSuccess = true;

    // Trace the path backwards
    int32 CurrentCell = CellB;
    while (CurrentCell != CellA) {
        if (Visited.Contains(CurrentCell)) {
            // This indicates there is a loop and it should not happen
            UE_LOG(LogGridDungeonQuery, Error, TEXT("GetPathBetweenCells: Discovery path has invalid state"));
            bOutSuccess = false;
            break;
        }
        Visited.Add(CurrentCell);

        OutResult.Add(CurrentCell);

        // Move one step back along the path
        if (!DiscoveryMap.Contains(CurrentCell)) {
            UE_LOG(LogGridDungeonQuery, Error, TEXT("GetPathBetweenCells: Discovery path has invalid state"));
            bOutSuccess = false;
            break;
        }
        CurrentCell = DiscoveryMap[CurrentCell];
    }

    // Add the last node
    OutResult.Add(CellA);

    // Since we traced it backwards, reverse the result
    Algo::Reverse(OutResult);
}

struct LongestPathSearchNode {
    LongestPathSearchNode() : CellId(-1), Length(0) {
    }

    LongestPathSearchNode(int32 InCellId, int32 InLength) : CellId(InCellId), Length(InLength) {
    }

    int32 CellId;
    float Length;
};

void UGridDungeonQuery::GetFurthestRoomsRecursive(const LongestPathSearchNode& Top, int32 StartRoomId,
                                                  TMap<int32, FCell>& CellLookup,
                                                  const FVector& GridSize, TSet<int>& Visited, int32& OutBestStartRoom,
                                                  int32& OutBestEndRoom, int32& OutBestDistance) {
    Visited.Add(Top.CellId);

    if (!CellLookup.Contains(Top.CellId)) {
        UE_LOG(LogGridDungeonQuery, Warning, TEXT("GetFurthestRooms: Invalid cell state while searching"));
        return;
    }


    FCell& TopCell = CellLookup[Top.CellId];
    FVector ThisCellLocation = GetCellCenter(TopCell, GridSize);

    bool bLeafNode = true;
    for (int32 ChildId : TopCell.FixedRoomConnections) {
        if (Visited.Contains(ChildId)) {
            continue;
        }

        if (!CellLookup.Contains(ChildId)) {
            UE_LOG(LogGridDungeonQuery, Warning, TEXT("GetFurthestRooms: Invalid child id"));
            continue;
        }

        FCell& ChildCell = CellLookup[ChildId];
        FVector ChildCellLocation = GetCellCenter(ChildCell, GridSize);

        float DistanceToChildCell = 1; // (ThisCellLocation - ChildCellLocation).Size2D();

        LongestPathSearchNode ChildNode(ChildId, Top.Length + DistanceToChildCell);
        GetFurthestRoomsRecursive(ChildNode, StartRoomId, CellLookup, GridSize, Visited, OutBestStartRoom,
                                  OutBestEndRoom, OutBestDistance);
        bLeafNode = false;
    }

    if (bLeafNode) {
        // Check if we have a best match
        if (Top.Length > OutBestDistance) {
            OutBestDistance = Top.Length;
            OutBestStartRoom = StartRoomId;
            OutBestEndRoom = Top.CellId;
        }
    }
    Visited.Remove(Top.CellId);
}


void UGridDungeonQuery::GetFurthestRooms(int32& OutRoomA, int32& OutRoomB) {
    OutRoomA = -1;
    OutRoomB = -1;

    if (!Model || !Config) {
        UE_LOG(LogGridDungeonQuery, Error, TEXT("GetFurthestRooms: Invalid state"));
        return;
    }

    FVector GridSize = Config->GridCellSize;
    int32 BestLength = 0;

    TMap<int32, FCell> CellLookup;
    GenerateCellLookup(CellLookup);

    // Start a search from each node (DFS)
    for (const FCell& StartCell : Model->Cells) {
        if (StartCell.CellType != FCellType::Room) {
            continue;
        }
        TSet<int> Visited; // Tracks the visited cell id
        LongestPathSearchNode StartNode(StartCell.Id, 0);
        //GetFurthestRoomsRecursive(StartNode, StartCell.Id, CellLookup, GridSize, Visited, OutRoomA, OutRoomB, BestLength);


        TQueue<LongestPathSearchNode> Queue;
        Queue.Enqueue(StartNode);
        while (!Queue.IsEmpty()) {
            LongestPathSearchNode Front;
            Queue.Dequeue(Front);
            Visited.Add(Front.CellId);

            if (!CellLookup.Contains(Front.CellId)) {
                UE_LOG(LogGridDungeonQuery, Warning, TEXT("GetFurthestRooms: Invalid cell state while searching"));
                continue;
            }

            // Check if we have a best match
            if (Front.Length > BestLength) {
                BestLength = Front.Length;
                OutRoomA = StartNode.CellId;
                OutRoomB = Front.CellId;
            }

            FCell& FrontCell = CellLookup[Front.CellId];
            FVector FrontCellLocation = GetCellCenter(FrontCell, GridSize);

            for (int32 ChildId : FrontCell.FixedRoomConnections) {
                if (Visited.Contains(ChildId)) {
                    continue;
                }

                if (!CellLookup.Contains(ChildId)) {
                    UE_LOG(LogGridDungeonQuery, Warning, TEXT("GetFurthestRooms: Invalid child id"));
                    continue;
                }

                FCell& ChildCell = CellLookup[ChildId];
                FVector ChildCellLocation = GetCellCenter(ChildCell, GridSize);

                float DistanceToChildCell = (FrontCellLocation - ChildCellLocation).Size();

                LongestPathSearchNode ChildNode(ChildId, Front.Length + DistanceToChildCell);
                Queue.Enqueue(ChildNode);
            }
        }

    }
}

void UGridDungeonQuery::GetCellAtLocation(const FVector& WorldLocation, int32& OutCellId, bool& bOutValid) {
    bOutValid = false;
    if (Model && Config) {
        FVector GridSize = Config->GridCellSize;
        for (const FCell& Cell : Model->Cells) {
            FVector LocationWorld = FMathUtils::ToVector(Cell.Bounds.Location) * GridSize;
            FVector SizeWorld = FMathUtils::ToVector(Cell.Bounds.Size) * GridSize;
            LocationWorld.Z = -1;
            SizeWorld.Z = 2;
            FBox Bounds(LocationWorld, LocationWorld + SizeWorld);

            if (Bounds.IsInside(FVector(WorldLocation.X, WorldLocation.Y, 0))) {
                OutCellId = Cell.Id;
                bOutValid = true;
                return;
            }
        }
    }
}

FCellType UGridDungeonQuery::GetCellType(int32 CellId) {
    return GetCell(CellId).CellType;
}

int32 UGridDungeonQuery::GetRandomCell() {
    if (Model->Cells.Num() == 0) {
        UE_LOG(LogGridDungeonQuery, Error,
               TEXT("Cannot fetch a random cell. No cells exist in the model. Please build the dungeon first"));
        return -1;
    }

    int32 Index = FMath::RandRange(0, Model->Cells.Num() - 1);
    return Model->Cells[Index].Id;
}

int32 UGridDungeonQuery::GetRandomCellOfType(FCellType CellType) {
    auto Cells = GetCellsOfType(CellType);
    if (Cells.Num() == 0) {
        UE_LOG(LogGridDungeonQuery, Error,
               TEXT("Cannot fetch a random cell of specified type as no cells exist in the model"));
        return -1;
    }

    int32 Index = FMath::RandRange(0, Cells.Num() - 1);
    return Cells[Index];
}

int32 UGridDungeonQuery::GetRandomCellFromStream(FRandomStream& RandomStream) {
    if (Model->Cells.Num() == 0) {
        UE_LOG(LogGridDungeonQuery, Error,
               TEXT("Cannot fetch a random cell. No cells exist in the model. Please build the dungeon first"));
        return -1;
    }

    int32 Index = RandomStream.RandRange(0, Model->Cells.Num() - 1);
    return Model->Cells[Index].Id;
}

int32 UGridDungeonQuery::GetRandomCellOfTypeFromStream(FCellType CellType, FRandomStream& RandomStream) {
    auto Cells = GetCellsOfType(CellType);
    if (Cells.Num() == 0) {
        UE_LOG(LogGridDungeonQuery, Error,
               TEXT("Cannot fetch a random cell of specified type as no cells exist in the model"));
        return -1;
    }

    int32 Index = RandomStream.RandRange(0, Cells.Num() - 1);
    return Cells[Index];
}

bool UGridDungeonQuery::ContainsStairBetween(int32 CellA, int32 CellB) {
    if (!Model) {
        UE_LOG(LogGridDungeonQuery, Error, TEXT("Invalid model state"));
        return false;
    }

    for (const FStairInfo& Stair : Model->Stairs) {
        if ((Stair.OwnerCell == CellA && Stair.ConnectedToCell == CellB) ||
            (Stair.OwnerCell == CellB && Stair.ConnectedToCell == CellA)) {
            return true;
        }
    }

    return false;
}

bool UGridDungeonQuery::ContainsDoorBetween(int32 CellA, int32 CellB) {
    if (!Model) {
        UE_LOG(LogGridDungeonQuery, Error, TEXT("Invalid model state"));
        return false;
    }

    for (const FCellDoor& Door : Model->Doors) {
        if ((Door.AdjacentCells[0] == CellA && Door.AdjacentCells[1] == CellB) ||
            (Door.AdjacentCells[0] == CellB && Door.AdjacentCells[1] == CellA)) {
            return true;
        }
    }
    return false;
}

FStairInfo UGridDungeonQuery::GetStairBetween(int32 CellA, int32 CellB) {
    if (Model) {
        for (const FStairInfo& Stair : Model->Stairs) {
            if ((Stair.OwnerCell == CellA && Stair.ConnectedToCell == CellB) ||
                (Stair.OwnerCell == CellB && Stair.ConnectedToCell == CellA)) {
                return Stair;
            }
        }
    }
    else {
        UE_LOG(LogGridDungeonQuery, Error, TEXT("Invalid model state"));
    }
    return FStairInfo();
}

FCellDoor UGridDungeonQuery::GetDoorBetween(int32 CellA, int32 CellB) {
    if (Model) {
        for (const FCellDoor& Door : Model->Doors) {
            if ((Door.AdjacentCells[0] == CellA && Door.AdjacentCells[1] == CellB) ||
                (Door.AdjacentCells[0] == CellB && Door.AdjacentCells[1] == CellA)) {
                return Door;
            }
        }
    }
    else {
        UE_LOG(LogGridDungeonQuery, Error, TEXT("Invalid model state"));
    }
    return FCellDoor();
}

FVector UGridDungeonQuery::GetOpeningPointBetweenAdjacentCells(int32 CellAId, int32 CellBId) {
    FCell CellA = GetCell(CellAId);
    FCell CellB = GetCell(CellBId);
    if (!Model || !Config) {
        UE_LOG(LogGridDungeonQuery, Error, TEXT("Invalid model state"));
        return FVector::ZeroVector;
    }

    // If any of the cells is a room, then there needs to be a door between them
    if (CellA.CellType == FCellType::Room || CellB.CellType == FCellType::Room) {
        if (ContainsDoorBetween(CellAId, CellBId)) {
            FCellDoor Door = GetDoorBetween(CellAId, CellBId);
            FVector LocA = FMathUtils::ToVector(Door.AdjacentTiles[0]) + FVector(0.5f, 0.5f, 0);
            FVector LocB = FMathUtils::ToVector(Door.AdjacentTiles[1]) + FVector(0.5f, 0.5f, 0);
            FVector Loc = (LocA + LocB) / 2.0f;
            Loc.Z = FMath::Max(CellA.Bounds.Location.Z, CellB.Bounds.Location.Z);
            return Loc * Config->GridCellSize;
        }
    }
        // If there is a height difference, make sure we have stairs between them
    else if (CellA.Bounds.Location.Z != CellB.Bounds.Location.Z) {
        if (ContainsStairBetween(CellAId, CellBId)) {
            FStairInfo Stair = GetStairBetween(CellAId, CellBId);
            FVector StairOffset(-.5, 0, 1);
            StairOffset *= Config->GridCellSize;
            StairOffset = Stair.Rotation * StairOffset;
            return Stair.Position + StairOffset;
        }
    }

    // Get the center of their intersection
    FRectangle Intersection = FRectangle::Intersect(CellA.Bounds, CellB.Bounds);
    FVector InterLocation = FMathUtils::ToVector(Intersection.Location);
    FVector InterSize = FMathUtils::ToVector(Intersection.Size);
    InterLocation.Z = (CellA.Bounds.Location.Z + CellB.Bounds.Location.Z) / 2.0f;
    InterSize.Z = 0;
    FVector Center = (InterLocation + InterSize / 2.0f) * Config->GridCellSize;
    return Center;
}

void UGridDungeonQuery::GetAdjacentCellsOnEdge(const FTransform& WallMarkerTransform, int32& CellA, int32& CellB) {
    if (!Config) {
        CellA = CellB = -1;
        return;
    }

    FVector SamplePointOffset = WallMarkerTransform.GetRotation().RotateVector(FVector(0, 0.5f, 0)) * Config->
        GridCellSize;
    FVector BaseLocation = WallMarkerTransform.GetLocation();
    FVector SamplePoints[2] = {BaseLocation + SamplePointOffset, BaseLocation - SamplePointOffset};
    int32 ResultCells[2];

    // Find the first sample point
    for (int i = 0; i < 2; i++) {
        bool bValid = false;
        GetCellAtLocation(SamplePoints[i], ResultCells[i], bValid);
        if (!bValid) {
            ResultCells[i] = -1;
        }
    }

    CellA = ResultCells[0];
    CellB = ResultCells[1];
}

void UGridDungeonQuery::IsNearMarker(const FTransform& CurrentMarkerTransform, const FString& NearbyMarkerName,
                                     float NearbyDistance, UDungeonBuilder* Builder, bool& bIsNear, int32& NumFound) {
    NumFound = 0;
    if (Builder) {
        float QueryDistanceSq = NearbyDistance * NearbyDistance;
        FVector CurrentLocation = CurrentMarkerTransform.GetLocation();

        for (const FPropSocket& Socket : Builder->GetMarkerList()) {
            float DistanceSq = (Socket.Transform.GetLocation() - CurrentLocation).SizeSquared();
            if (DistanceSq < QueryDistanceSq && Socket.SocketType == NearbyMarkerName) {
                NumFound++;
            }
        }
    }
    bIsNear = NumFound > 0;
}

void UGridDungeonQuery::GetAdjacentCells(int32 CellId, TArray<int32>& OutAdjacentCells) {
    OutAdjacentCells.Reset();
    if (Model) {
        for (const FCell& Cell : Model->Cells) {
            if (Cell.Id == CellId) {
                OutAdjacentCells = Cell.AdjacentCells;
                return;
            }
        }
    }
}

FCell UGridDungeonQuery::GetCell(int32 CellId) {
    if (Model) {
        for (const FCell& Cell : Model->Cells) {
            if (Cell.Id == CellId) {
                return Cell;
            }
        }
    }

    // Not found. Return an invalid cell
    return FCell();
}

void UGridDungeonQuery::GenerateCellLookup(TMap<int32, FCell>& OutLookup) {
    OutLookup.Reset();
    if (!Model) {
        UE_LOG(LogGridDungeonQuery, Error, TEXT("GenerateCellLookup: Invalid model state"));
        return;
    }

    for (const FCell& Cell : Model->Cells) {
        OutLookup.Add(Cell.Id, Cell);
    }
}

