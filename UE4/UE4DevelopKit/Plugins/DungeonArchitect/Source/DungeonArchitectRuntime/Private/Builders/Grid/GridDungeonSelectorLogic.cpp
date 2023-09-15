//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/Grid/GridDungeonSelectorLogic.h"


FCell* UGridDungeonSelectorLogic::GetCell(UGridDungeonModel* Model, int32 GridX, int32 GridY) {
    FGridCellInfo BaseCellInfo = Model->GetGridCellLookup(GridX, GridY);
    return Model->GetCell(BaseCellInfo.CellId);
}

bool UGridDungeonSelectorLogic::IsDifferentCell(FCell* Cell0, FCell* Cell1) {
    if (!Cell0 || !Cell1) return true;
    if (Cell0->Id == Cell1->Id) {
        return false;
    }

    FCellType CellType0 = Cell0->CellType;
    if (CellType0 == FCellType::CorridorPadding) {
        CellType0 = FCellType::Corridor;
    }

    FCellType CellType1 = Cell1->CellType;
    if (CellType1 == FCellType::CorridorPadding) {
        CellType1 = FCellType::Corridor;
    }

    if (CellType0 != CellType1) {
        return true;
    }

    if (Cell0->Bounds.Location.Z != Cell1->Bounds.Location.Z) {
        return true;
    }

    return false;
}

bool UGridDungeonSelectorLogic::IsOnCorner(UGridDungeonModel* Model, int32 GridX, int32 GridY) {
    if (!Model) return false;
    FCell* BaseCell = GetCell(Model, GridX, GridY);
    if (!BaseCell) return false;

    for (int32 dx = -1; dx <= 1; dx++) {
        for (int32 dy = -1; dy <= 1; dy++) {
            if (!dx && !dy) continue;
            int32 x = GridX + dx;
            int32 y = GridY + dy;
            FCell* Cell = GetCell(Model, x, y);
            if (IsDifferentCell(BaseCell, Cell)) {
                return true;
            }
        }
    }

    return false;
}

bool UGridDungeonSelectorLogic::IsPillarOnCorner(UGridDungeonModel* Model, int32 GridX, int32 GridY, FTransform& OutCornerOffset) {
    if (!Model) return false;
    FCell* BaseCell = GetCell(Model, GridX, GridY);
    if (!BaseCell) return false;
    int32 dx[] = {0, -1, -1, 0};
    int32 dy[] = {0, 0, -1, -1};


    // Cell Hash -> Count mapping
    TMap<int32, int32> CellHashCount;
    for (int32 i = 0; i < 4; i++) {
        int32 x = GridX + dx[i];
        int32 y = GridY + dy[i];
        FCell* Cell = GetCell(Model, x, y);
        int32 Hash = 0;
        if (Cell) {
            FCellType CellType = Cell->CellType;
            if (CellType == FCellType::CorridorPadding) {
                CellType = FCellType::Corridor;
            }

            Hash = Cell->Bounds.Location.Z * 100 + static_cast<int32>(CellType);
            if (!CellHashCount.Contains(Hash)) {
                CellHashCount.Add(Hash, 0);
            }
            CellHashCount[Hash]++;
        }
    }
    if (CellHashCount.Num() == 2) {
        TArray<int32> HashCounter;
        CellHashCount.GenerateValueArray(HashCounter);
        int32 a = HashCounter[0];
        int32 b = HashCounter[1];

        if ((a == 1 && b == 3) || (a == 3 && b == 1)) {
            return true;
        }
    }
    return false;
}

bool UGridDungeonSelectorLogic::IsPassageTooNarrow(UGridDungeonModel* Model, int32 GridX, int32 GridY) {
    FCell* BaseCell = GetCell(Model, GridX, GridY);
    if (!BaseCell) return false;

    int32 dx[] = {1, -1, 0, 0};
    int32 dy[] = {0, 0, 1, -1};
    bool corners[4];
    for (int32 i = 0; i < 4; i++) {
        int32 x = GridX + dx[i];
        int32 y = GridY + dy[i];
        FCell* Cell = GetCell(Model, x, y);
        corners[i] = IsDifferentCell(BaseCell, Cell);
    }
    return (corners[0] && corners[1]) || (corners[2] && corners[3]);
}

bool UGridDungeonSelectorLogic::ContainsStair(UGridDungeonModel* Model, const FCell& Cell, int32 GridX, int32 GridY) {
    if (Model->CellStairsLookup.Contains(Cell.Id)) {
        const TArray<FStairInfo>& Stairs = Model->CellStairsLookup[Cell.Id];
        for (const FStairInfo& Stair : Stairs) {
            if (Stair.IPosition.X == GridX && Stair.IPosition.Y == GridY) {
                return true;
            }
        }
    }
    return false;
}

bool UGridDungeonSelectorLogic::SelectNode_Implementation(UGridDungeonModel* Model, UGridDungeonConfig* Config,
                                                          UGridDungeonBuilder* Builder, UGridDungeonQuery* Query,
                                                          const FCell& Cell, const FRandomStream& RandomStream,
                                                          int32 GridX, int32 GridY, const FTransform& MarkerTransform) {
    return false;
}

