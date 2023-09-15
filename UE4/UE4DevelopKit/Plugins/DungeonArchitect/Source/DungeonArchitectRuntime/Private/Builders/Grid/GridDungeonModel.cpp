//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/Grid/GridDungeonModel.h"


void UGridDungeonModel::BuildCellLookup() {
    CellLookup.Reset();
    FCell* CellArray = Cells.GetData();
    for (int i = 0; i < Cells.Num(); i++) {
        FCell* cell = CellArray + i;
        if (cell) {
            CellLookup.Add(cell->Id, cell);
        }
    }
}

bool UGridDungeonModel::ContainsStairAtLocation(int x, int y) {
    for (const auto& Stair : Stairs) {
        if (Stair.IPosition.X == x && Stair.IPosition.Y == y) {
            return true;
        }
    }
    return false;
    return false;
}

bool UGridDungeonModel::ContainsStairBetweenCells(int32 CellIdA, int32 CellIdB) {
    if (CellStairsLookup.Contains(CellIdA)) {
        for (const FStairInfo& stair : CellStairsLookup[CellIdA]) {
            if (stair.ConnectedToCell == CellIdB) {
                return true;
            }
        }
    }

    if (CellStairsLookup.Contains(CellIdB)) {
        for (const FStairInfo& stair : CellStairsLookup[CellIdB]) {
            if (stair.ConnectedToCell == CellIdA) {
                return true;
            }
        }
    }

    return false;
}

FGridCellInfo UGridDungeonModel::GetGridCellLookup(int x, int y) const {
    if (!GridCellInfoLookup.Contains(x) || !GridCellInfoLookup[x].Contains(y)) {
        return FGridCellInfo();
    }
    return GridCellInfoLookup[x][y];
}

void UGridDungeonModel::Cleanup() {
    CellLookup.Reset();
    CellStairsLookup.Reset();
}

void UGridDungeonModel::Reset() {
    Cells.Reset();
    Doors.Reset();
    Stairs.Reset();
    CellStairsLookup.Reset();
    CellLookup.Reset();
    GridCellInfoLookup.Reset();
    DoorManager = FDoorManager();
    BuildState = DungeonModelBuildState::Initial;
}

bool operator==(const FCellDoor& A, const FCellDoor& B) {
    return A.AdjacentCells[0] == B.AdjacentCells[0]
        && A.AdjacentCells[1] == B.AdjacentCells[1]
        && A.AdjacentTiles[0] == B.AdjacentTiles[0]
        && A.AdjacentTiles[1] == B.AdjacentTiles[1];
}

