//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/Grid/GridDungeonModelHelper.h"

#include "Builders/Grid/GridDungeonConfig.h"
#include "Core/Dungeon.h"

DEFINE_LOG_CATEGORY(DungeonModelHelperLog);

UGridDungeonModelHelper::UGridDungeonModelHelper(const FObjectInitializer& ObjectInitializer) : Super(
    ObjectInitializer) {
}

void UGridDungeonModelHelper::GetCellConnectedRooms(const FCell& Cell, TArray<int32>& ConnectedRooms) {
    ConnectedRooms = Cell.ConnectedRooms;
}

void UGridDungeonModelHelper::GetCellMSTRooms(const FCell& Cell, TArray<int32>& MSTRooms) {
    MSTRooms = Cell.FixedRoomConnections;
}

void UGridDungeonModelHelper::GetCellCenter(const FCell& Cell, FVector& Center) {
    Center = MakeVector(Cell.Bounds.Location) + MakeVector(Cell.Bounds.Size) / 2.0f;
}


void UGridDungeonModelHelper::ToWorldCoords(const FRectangle& Bounds, const FVector& GridSize, FVector& Location,
                                            FVector& Size) {
    Location = MakeVector(Bounds.Location) * GridSize;
    Size = MakeVector(Bounds.Size) * GridSize;
}

void UGridDungeonModelHelper::GetBoundingBox(const TArray<FCell>& Cells, FRectangle& Bounds) {
    int32 minX = INT_MAX;
    int32 maxX = -INT_MAX;
    int32 minY = INT_MAX;
    int32 maxY = -INT_MAX;
    int32 minZ = INT_MAX;
    int32 maxZ = -INT_MAX;

    for (const FCell& Cell : Cells) {
        FIntVector CellLocation = Cell.Bounds.Location;
        FIntVector CellSize = Cell.Bounds.Size;

        minX = FMath::Min(minX, CellLocation.X);
        minY = FMath::Min(minY, CellLocation.Y);
        minZ = FMath::Min(minZ, CellLocation.Z);

        maxX = FMath::Max(maxX, CellLocation.X + CellSize.X);
        maxY = FMath::Max(maxY, CellLocation.Y + CellSize.Y);
        maxZ = FMath::Max(maxZ, CellLocation.Z + CellSize.Z);
    }

    Bounds.Location.X = minX;
    Bounds.Location.Y = minY;
    Bounds.Location.Z = minZ;
    Bounds.Size.X = maxX - minX;
    Bounds.Size.Y = maxY - minY;
    Bounds.Size.Z = maxZ - minZ;
}

bool DoorExists(const TArray<FIntVector>& DoorExitPoints, int32 x1, int32 y1, int32 x2, int32 y2) {
    for (int x = x1; x <= x2; x++) {
        for (int y = y1; y <= y2; y++) {
            if (DoorExitPoints.Contains(FIntVector(x, y, 0))) {
                return true;
            }
        }
    }
    return false;
}

void UGridDungeonModelHelper::GetRoomFreeEdge(UGridDungeonModel* Model, const FCell& RoomCell,
                                              TArray<FVector>& FreeEdgeCenters, TArray<float>& FreeEdgeAngles) {
    const TArray<FCellDoor>& Doors = Model->DoorManager.GetDoors();
    TArray<FIntVector> DoorExitPoints;
    for (const FCellDoor& Door : Doors) {
        FIntVector P0 = Door.AdjacentTiles[0];
        FIntVector P1 = Door.AdjacentTiles[1];
        P0.Z = 0;
        P1.Z = 0;
        DoorExitPoints.AddUnique(P0);
        DoorExitPoints.AddUnique(P1);
    }

    FRectangle Rectangle = RoomCell.Bounds;
    int32 z = Rectangle.Location.Z;

    int32 x1 = Rectangle.Location.X;
    int32 x2 = Rectangle.Location.X + Rectangle.Size.X;

    int32 y1 = Rectangle.Location.Y;
    int32 y2 = Rectangle.Location.Y + Rectangle.Size.Y;

    if (!DoorExists(DoorExitPoints, x1, y1, x2, y1)) {
        FreeEdgeCenters.Add(FVector((x2 + x1) / 2.0f, y1, z));
        FreeEdgeAngles.Add(90);
    }
    if (!DoorExists(DoorExitPoints, x1, y2, x2, y2)) {
        FreeEdgeCenters.Add(FVector((x2 + x1) / 2.0f, y2, z));
        FreeEdgeAngles.Add(270);
    }

    if (!DoorExists(DoorExitPoints, x1, y1, x1, y2)) {
        FreeEdgeCenters.Add(FVector(x1, (y2 + y1) / 2.0f, z));
        FreeEdgeAngles.Add(0);
    }
    if (!DoorExists(DoorExitPoints, x2, y1, x2, y2)) {
        FreeEdgeCenters.Add(FVector(x2, (y2 + y1) / 2.0f, z));
        FreeEdgeAngles.Add(180);
    }

}

void UGridDungeonModelHelper::GetDoorExits(const FCellDoor& Door, FVector& ExitA, FVector& ExitB) {
    ExitA = MakeVector(Door.AdjacentTiles[0]);
    ExitB = MakeVector(Door.AdjacentTiles[1]);
}


FVector UGridDungeonModelHelper::GetRandomCellLocation(UGridDungeonModel* Model, UGridDungeonConfig* Config) {
    if (!Model || !Config) {
        UE_LOG(DungeonModelHelperLog, Error, TEXT("Invalid input parameters"));
        return FVector::ZeroVector;
    }

    int32 CellCount = Model->Cells.Num();
    if (CellCount == 0) {
        UE_LOG(DungeonModelHelperLog, Error, TEXT("No cells on the dungeon. Please build it first"));
        return FVector::ZeroVector;
    }

    FCell Cell = Model->Cells[FMath::Rand() % CellCount];
    FVector Center;
    GetCellCenter(Cell, Center);
    Center *= Config->GridCellSize;
    return Center;
}


void UGridDungeonModelHelper::AddPaintCell(const FGridToolPaintStrokeData& CellData, ADungeon* Dungeon,
                                           bool bAutomaticRebuild) {
    if (!Dungeon)
    {
        return;
    }
    
    UGridDungeonToolData* ToolData = Cast<UGridDungeonToolData>(Dungeon->ToolData);
    bool bOverlappingCell = false;
    const FIntVector& CellLocation = CellData.Location;
    for (FGridToolPaintStrokeData& OtherCellData : ToolData->PaintedCells) {
        FIntVector& OtherCellLocation = OtherCellData.Location;
        if (OtherCellLocation.X == CellLocation.X && OtherCellLocation.Y == CellLocation.Y) {
            OtherCellData.CellType = CellData.CellType;
            if (OtherCellLocation.Z != CellLocation.Z) {
                OtherCellLocation.Z = CellLocation.Z;
                bOverlappingCell = true;
                break;
            }
            // Cell with this data already exists. ignore the request
            return;
        }
    }
    if (!bOverlappingCell) {
        ToolData->PaintedCells.Add(CellData);
    }
    if (bAutomaticRebuild && Dungeon) {
        Dungeon->BuildDungeon();
    }
    Dungeon->MarkPackageDirty();
}

void UGridDungeonModelHelper::RemovePaintCell(const FGridToolPaintStrokeData& CellData, ADungeon* Dungeon,
                                              bool bAutomaticRebuild) {
    if (!Dungeon)
    {
        return;
    }
    
    UGridDungeonToolData* ToolData = Cast<UGridDungeonToolData>(Dungeon->ToolData);
    if (ToolData->PaintedCells.Contains(CellData)) {
        ToolData->PaintedCells.Remove(CellData);
        if (bAutomaticRebuild && Dungeon) {
            Dungeon->BuildDungeon();
        }
    }
    Dungeon->MarkPackageDirty();
}

