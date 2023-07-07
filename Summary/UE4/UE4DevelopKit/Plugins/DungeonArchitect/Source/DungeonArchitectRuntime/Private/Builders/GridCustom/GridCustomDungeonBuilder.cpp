//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/GridCustom/GridCustomDungeonBuilder.h"

#include "Builders/Grid/GridDungeonConfig.h"

DEFINE_LOG_CATEGORY(GridCustomDungeonBuilderLog);

UGridCustomDungeonBuilder::UGridCustomDungeonBuilder() {
    bUseHeightVariation = false;
}

void UGridCustomDungeonBuilder::RegisterRoom(int32 X, int32 Y, int32 Z, int32 Width, int32 Height, int32& RoomID) {
    RoomID = ++_CellIdCounter;
    FCustomGridRoomInfo RoomInfo;
    RoomInfo.Bounds = FRectangle(X, Y, Width, Height);
    RoomInfo.Bounds.Location.Z = Z;
    RoomInfo.RoomId = RoomID;
    RoomBounds.Add(RoomInfo);

}

void UGridCustomDungeonBuilder::RegisterRoomAt(int32 X, int32 Y, int32 Z, int32& RoomID) {
    const int32 Min = gridConfig->MinCellSize;
    const int32 Max = gridConfig->MaxCellSize;

    int32 Width = Min + FMath::RoundToInt((Max - Min) * random.FRand());
    int32 Height = Min + FMath::RoundToInt((Max - Min) * random.FRand());
    RegisterRoom(X, Y, Z, Width, Height, RoomID);
}

void UGridCustomDungeonBuilder::ConnectRooms(int32 Room1, int32 Room2) {
    RoomConnections.AddUnique(FIntVector(Room1, Room2, 0));
}

void UGridCustomDungeonBuilder::GetRandomOffset(int32 X, int32 Y, float Radius, int32& OutX, int32& OutY) {
    float Angle = random.FRand() * PI * 2;
    int32 DX = FMath::RoundToInt(FMath::Cos(Angle) * Radius);
    int32 DY = FMath::RoundToInt(FMath::Sin(Angle) * Radius);
    OutX = X + DX;
    OutY = Y + DY;
}

void UGridCustomDungeonBuilder::GenerateCustomLayout_Implementation(UGridDungeonConfig* GridConfig) {
    // Test layout for demo.  Create a blueprint to override and provide your own layout
    float Radius = 15;
    int32 NumRooms = 10;
    int32 Width = 4, Height = 4;
    int32 PreviousRoomId = -1, FirstRoomId = -1;
    for (int i = 0; i < NumRooms; i++) {
        float Angle = i / static_cast<float>(NumRooms) * PI * 2;
        int32 X = FMath::RoundToInt(FMath::Cos(Angle) * Radius);
        int32 Y = FMath::RoundToInt(FMath::Sin(Angle) * Radius);
        int32 Z = 0;
        if (bUseHeightVariation) {
            Z = random.FRand() > 0.5 ? 1 : 0;
        }
        int32 RoomID;
        RegisterRoom(X, Y, Z, Width, Height, RoomID);
        if (i > 0) {
            ConnectRooms(RoomID, PreviousRoomId);
        }
        else {
            FirstRoomId = RoomID;
        }
        PreviousRoomId = RoomID;
    }
    // Connect the last room
    ConnectRooms(PreviousRoomId, FirstRoomId);
}

void UGridCustomDungeonBuilder::BuildDungeonCells() {
    _CellIdCounter = 0;
    RoomBounds.Reset();
    RoomConnections.Reset();

    GenerateCustomLayout(gridConfig);
    TArray<FCell> Cells;

    for (const FCustomGridRoomInfo& RoomInfo : RoomBounds) {
        FCell Cell;
        Cell.Id = RoomInfo.RoomId;
        Cell.Bounds = RoomInfo.Bounds;
        Cell.UserDefined = false;
        Cell.CellType = FCellType::Room;
        Cells.Add(Cell);
    }

    gridModel->Cells = Cells;
    gridModel->BuildCellLookup();
    gridModel->BuildState = DungeonModelBuildState::Triangulation;
}

void UGridCustomDungeonBuilder::GenerateRoomConnections() {
    if (!gridModel) return;

    for (const FIntVector& ConnectionInfo : RoomConnections) {
        int32 RoomAId = ConnectionInfo.X;
        int32 RoomBId = ConnectionInfo.Y;

        FCell* RoomA = gridModel->GetCell(RoomAId);
        FCell* RoomB = gridModel->GetCell(RoomBId);
        if (!RoomA || !RoomB) continue;

        AddUnique<int32>(RoomA->FixedRoomConnections, RoomB->Id);
        AddUnique<int32>(RoomB->FixedRoomConnections, RoomA->Id);

        AddUnique<int32>(RoomA->ConnectedRooms, RoomB->Id);
        AddUnique<int32>(RoomB->ConnectedRooms, RoomA->Id);
    }
    //FCell* cell = gridModel->GetCell(cellInfo.CellId);
    gridModel->BuildState = DungeonModelBuildState::Corridors;
}

void UGridCustomDungeonBuilder::GenerateDungeonHeights() {
    // Do not affect the height procedurally. The user can manually set the height from blueprints while registering the rooms
}

