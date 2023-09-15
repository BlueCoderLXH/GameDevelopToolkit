//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/Tilemap/GridFlowTilemap.h"

#include "Containers/Queue.h"

/////////////////////////////////// UGridFlowTilemap ///////////////////////////////////

const FName UGridFlowTilemap::StateTypeID = TEXT("TilemapObject");
void UGridFlowTilemap::Initialize(int32 InWidth, int32 InHeight) {
    Width = InWidth;
    Height = InHeight;
    Cells.SetNum(Width * Height);

    for (int y = 0; y < InHeight; y++) {
        for (int x = 0; x < InWidth; x++) {
            Cells[CELL_INDEX(x, y)].TileCoord = FIntPoint(x, y);
        }
    }

    {
        const int32 EdgesWidth = Width + 1;
        const int32 EdgesHeight = Height + 1;
        EdgesHorizontal.SetNum(EdgesWidth * EdgesHeight);
        EdgesVertical.SetNum(EdgesWidth * EdgesHeight);
        for (int y = 0; y < EdgesHeight; y++) {
            for (int x = 0; x < EdgesWidth; x++) {
                // Update the horizontal edges
                const FIntPoint Coord(x, y);
                EdgesHorizontal[EDGE_INDEX(x, y)].EdgeCoord = FGridFlowTilemapCoord(Coord, true);
                EdgesVertical[EDGE_INDEX(x, y)].EdgeCoord = FGridFlowTilemapCoord(Coord, false);
            }
        }
    }

    WallMetadataMap.Reset();
    DoorMetadataMap.Reset();
}

void UGridFlowTilemap::SetWallMetadata(const FGridFlowTilemapCoord& Coord, const FGridFlowTilemapCellWallInfo& InWallMeta) {
    FGridFlowTilemapCellWallInfo& WallMeta = WallMetadataMap.FindOrAdd(Coord);
    WallMeta = InWallMeta;
}

void UGridFlowTilemap::SetDoorMetadata(const FGridFlowTilemapCoord& Coord, const FGridFlowTilemapCellDoorInfo& InDoorMeta) {
    FGridFlowTilemapCellDoorInfo& DoorMeta = DoorMetadataMap.FindOrAdd(Coord);
    DoorMeta = InDoorMeta;
}

bool UGridFlowTilemap::GetWallMeta(const FGridFlowTilemapCoord& Coord, FGridFlowTilemapCellWallInfo& OutData) const {
    const FGridFlowTilemapCellWallInfo* SearchResult = WallMetadataMap.Find(Coord);
    if (!SearchResult) return false;
    OutData = *SearchResult;
    return true;
}

bool UGridFlowTilemap::GetDoorMeta(const FGridFlowTilemapCoord& Coord, FGridFlowTilemapCellDoorInfo& OutData) const {
    const FGridFlowTilemapCellDoorInfo* SearchResult = DoorMetadataMap.Find(Coord);
    if (!SearchResult) return false;
    OutData = *SearchResult;
    return true;

}

/////////////////////////////// FGridFlowTilemapDistanceField /////////////////////////////// 

FGridFlowTilemapDistanceField::FGridFlowTilemapDistanceField(UGridFlowTilemap* Tilemap) {
    if (Tilemap) {
        InitializeArray2D(Tilemap->GetWidth(), Tilemap->GetHeight());
    }

    FindDistanceFromEdge(Tilemap);
    FindDistanceFromDoor(Tilemap);
}

namespace {
    const int32 TileChildOffsets[] = {
        -1, 0,
        1, 0,
        0, -1,
        0, 1
    };
}

void FGridFlowTilemapDistanceField::FindDistanceFromEdge(UGridFlowTilemap* Tilemap) {
    TQueue<FIntPoint> Queue;
    for (int y = 0; y < Tilemap->GetHeight(); y++) {
        for (int x = 0; x < Tilemap->GetWidth(); x++) {
            FGridFlowTilemapCell& Cell = Tilemap->Get(x, y);
            if (Cell.CellType == EGridFlowTilemapCellType::Floor) {
                bool bAllNeighborsWalkable = true;
                for (int i = 0; i < 4; i++) {
                    int32 cx = x + TileChildOffsets[i * 2 + 0];
                    int32 cy = y + TileChildOffsets[i * 2 + 1];
                    FGridFlowTilemapCell* ncell = Tilemap->GetSafe(cx, cy);
                    if (!ncell) continue;

                    if (ncell->CellType != EGridFlowTilemapCellType::Floor) {
                        bAllNeighborsWalkable = false;
                        break;
                    }

                    // Check if there's a blocking overlay
                    if (Cell.Overlay.bEnabled && Cell.Overlay.bTileBlockingOverlay) {
                        bAllNeighborsWalkable = false;
                        break;
                    }
                }

                if (!bAllNeighborsWalkable) {
                    Queue.Enqueue(Cell.TileCoord);
                    Array[INDEX(x, y)].DistanceFromEdge = 0;
                }
            }
        }
    }

    FIntPoint CellCoord;
    while (Queue.Dequeue(CellCoord)) {
        FGridFlowTilemapCell& Cell = Tilemap->Get(CellCoord.X, CellCoord.Y);

        int32 x = Cell.TileCoord.X;
        int32 y = Cell.TileCoord.Y;
        int32 NDist = Get(x, y).DistanceFromEdge + 1;

        for (int i = 0; i < 4; i++) {
            int32 nx = x + TileChildOffsets[i * 2 + 0];
            int32 ny = y + TileChildOffsets[i * 2 + 1];
            FGridFlowTilemapCell* NCell = Tilemap->GetSafe(nx, ny);
            if (!NCell) continue;
            bool bWalkableTile = (NCell->CellType == EGridFlowTilemapCellType::Floor);
            if (bWalkableTile && Cell.Overlay.bEnabled && Cell.Overlay.bTileBlockingOverlay) {
                bWalkableTile = false;
            }

            if (bWalkableTile && NDist < Array[INDEX(nx, ny)].DistanceFromEdge) {
                Array[INDEX(nx, ny)].DistanceFromEdge = NDist;
                Queue.Enqueue(NCell->TileCoord);
            }
        }
    }
}

void FGridFlowTilemapDistanceField::FindDistanceFromDoor(UGridFlowTilemap* Tilemap) {
    TQueue<FIntPoint> Queue;
    for (int y = 0; y < Tilemap->GetHeight(); y++) {
        for (int x = 0; x < Tilemap->GetWidth(); x++) {
            FGridFlowTilemapCell& Cell = Tilemap->Get(x, y);
            if (Cell.CellType == EGridFlowTilemapCellType::Door) {
                Queue.Enqueue(Cell.TileCoord);
                Array[INDEX(x, y)].DistanceFromDoor = 0;
            }
        }
    }

    FIntPoint CellCoord;
    while (Queue.Dequeue(CellCoord)) {
        FGridFlowTilemapCell& Cell = Tilemap->Get(CellCoord.X, CellCoord.Y);

        int32 x = CellCoord.X;
        int32 y = CellCoord.Y;
        int32 ndist = Array[INDEX(x, y)].DistanceFromDoor + 1;

        for (int i = 0; i < 4; i++) {
            int32 nx = x + TileChildOffsets[i * 2 + 0];
            int32 ny = y + TileChildOffsets[i * 2 + 1];
            FGridFlowTilemapCell* ncell = Tilemap->GetSafe(nx, ny);
            if (!ncell) continue;
            bool bWalkableTile = (ncell->CellType == EGridFlowTilemapCellType::Floor);
            if (bWalkableTile && Cell.Overlay.bEnabled && Cell.Overlay.bTileBlockingOverlay) {
                bWalkableTile = false;
            }

            if (bWalkableTile && ndist < Array[INDEX(nx, ny)].DistanceFromDoor) {
                Array[INDEX(nx, ny)].DistanceFromDoor = ndist;
                Queue.Enqueue(ncell->TileCoord);
            }
        }
    }
}

