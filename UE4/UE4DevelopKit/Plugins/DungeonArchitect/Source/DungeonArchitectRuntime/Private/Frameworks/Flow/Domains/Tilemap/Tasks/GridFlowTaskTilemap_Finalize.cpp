//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/Tilemap/Tasks/GridFlowTaskTilemap_Finalize.h"

#include "Core/Utils/MathUtils.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraph.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractNode.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Implementations/GridFlowAbstractGraph.h"
#include "Frameworks/Flow/Domains/Tilemap/GridFlowTilemap.h"
#include "Frameworks/Flow/Domains/Tilemap/GridFlowTilemapDomain.h"

#include "Containers/Queue.h"

void UGridFlowTaskTilemap_Finalize::Execute(const FFlowExecutionInput& Input, const FFlowTaskExecutionSettings& InExecSettings, FFlowExecutionOutput& Output) {
    if (Input.IncomingNodeOutputs.Num() == 0) {
        Output.ErrorMessage = "Missing Input";
        Output.ExecutionResult = EFlowTaskExecutionResult::FailHalt;
        return;
    }
    if (Input.IncomingNodeOutputs.Num() > 1) {
        Output.ErrorMessage = "Only one input allowed";
        Output.ExecutionResult = EFlowTaskExecutionResult::FailHalt;
        return;
    }

    Output.State = Input.IncomingNodeOutputs[0].State->Clone();

    
    UGridFlowAbstractGraph* Graph = Output.State->GetState<UGridFlowAbstractGraph>(UFlowAbstractGraphBase::StateTypeID);
    if (!Graph) {
        Output.ErrorMessage = "Invalid Input Layout Graph";
        Output.ExecutionResult = EFlowTaskExecutionResult::FailHalt;
        return;
    }

    UGridFlowTilemap* Tilemap = Output.State->GetState<UGridFlowTilemap>(UGridFlowTilemap::StateTypeID);
    if (!Tilemap) {
        Output.ErrorMessage = "Invalid Input Tilemap";
        Output.ExecutionResult = EFlowTaskExecutionResult::FailHalt;
        return;
    }

    TMap<FVector, TArray<FGridFlowTilemapCell*>> FreeTilesByNode;

    for (FGridFlowTilemapCell& Cell : Tilemap->GetCells()) {
        if (Cell.CellType == EGridFlowTilemapCellType::Floor) {
            FVector NodeCoord = Cell.ChunkCoord;
            TArray<FGridFlowTilemapCell*>& FreeTiles = FreeTilesByNode.FindOrAdd(NodeCoord);
            if (!Cell.bHasItem) {
                FreeTiles.Add(&Cell);
            }
        }
    }

    // Filter walkable paths on the free tiles (some free tile patches may be blocked by overlays like tree lines)
    TArray<FVector> NodeKeys;
    FreeTilesByNode.GenerateKeyArray(NodeKeys);

    for (const FVector& NodeCoord : NodeKeys) {
        TArray<FGridFlowTilemapCell*>& FreeTiles = FreeTilesByNode.FindOrAdd(NodeCoord);
        FreeTiles = FilterWalkablePath(FreeTiles);
    }

    FGridFlowTilemapDistanceField DistanceField(Tilemap);

    const FRandomStream& Random = *Input.Random;
    // Add node items
    for (UFlowAbstractNode* Node : Graph->GraphNodes) {
        if (!Node) continue;
        
        TArray<FGridFlowTilemapCell*>* SearchResult = FreeTilesByNode.Find(Node->Coord);
        if (!SearchResult) continue;

        TArray<FGridFlowTilemapCell*>& FreeTiles = *SearchResult;

        for (const UFlowGraphItem* Item : Node->NodeItems) {
            if (FreeTiles.Num() == 0) {
                Output.ErrorMessage = "Item Placement failed. Insufficient free tiles";
                Output.ExecutionResult = EFlowTaskExecutionResult::FailRetry;
                return;
            }

            const int32 FreeTileIndex = Random.RandRange(0, FreeTiles.Num() - 1);
            FGridFlowTilemapCell* FreeTile = FreeTiles[FreeTileIndex];
            FreeTile->bHasItem = true;
            FreeTile->ItemId = Item->ItemId;
            FreeTiles.Remove(FreeTile);
        }
    }

    // Cache tilemap metadata for each abstract graph node
    UpdateLayoutTilemapMetadata(Graph, Tilemap);

    Output.ExecutionResult = EFlowTaskExecutionResult::Success;
}

TArray<FGridFlowTilemapCell*> UGridFlowTaskTilemap_Finalize::FilterWalkablePath(
    const TArray<FGridFlowTilemapCell*>& Cells) const {
    TSet<FIntPoint> Unreachable;
    TMap<FIntPoint, FGridFlowTilemapCell*> CellsByCoord;

    TQueue<FGridFlowTilemapCell*> Queue;

    {
        for (FGridFlowTilemapCell* Cell : Cells) {
            CellsByCoord.Add(Cell->TileCoord, Cell);
            if (Cell->bMainPath) {
                Queue.Enqueue(Cell);
            }
            else {
                Unreachable.Add(Cell->TileCoord);
            }
        }
    }

    static const int ChildOffsets[] = {
        -1, 0,
        1, 0,
        0, -1,
        0, 1
    };

    {
        FGridFlowTilemapCell* Cell = nullptr;
        while (Queue.Dequeue(Cell)) {
            FIntPoint Coord = Cell->TileCoord;
            for (int i = 0; i < 4; i++) {
                int32 cx = Coord.X + ChildOffsets[i * 2 + 0];
                int32 cy = Coord.Y + ChildOffsets[i * 2 + 1];
                FIntPoint ChildCoord(cx, cy);
                if (Unreachable.Contains(ChildCoord)) {
                    bool bCanTraverse = true;
                    FGridFlowTilemapCell* ChildCell = CellsByCoord[ChildCoord];
                    if (ChildCell->bHasOverlay && ChildCell->Overlay.bTileBlockingOverlay) {
                        bCanTraverse = false;
                    }
                    if (bCanTraverse) {
                        Unreachable.Remove(ChildCoord);
                        Queue.Enqueue(CellsByCoord[ChildCoord]);
                    }
                }
            }
        }
    }

    // Tag the cells that are not reachable
    for (const FIntPoint& UnreachableCoord : Unreachable) {
        FGridFlowTilemapCell* InvalidCell = CellsByCoord[UnreachableCoord];
        InvalidCell->bUnreachable = true;
    }

    if (bDebugUnwalkableCells) {
        for (const FIntPoint& UnreachableCoord : Unreachable) {
            FGridFlowTilemapCell* InvalidCell = CellsByCoord[UnreachableCoord];
            InvalidCell->CustomColor = FLinearColor::Red;
            InvalidCell->bUseCustomColor = true;
        }
    }

    // Grab all the cells that are not in the unreachable list
    TArray<FGridFlowTilemapCell*> Result;
    for (FGridFlowTilemapCell* Cell : Cells) {
        if (!Unreachable.Contains(Cell->TileCoord)) {
            Result.Add(Cell);
        }
    }

    return Result;
}

void UGridFlowTaskTilemap_Finalize::UpdateLayoutTilemapMetadata(UGridFlowAbstractGraph* InGraph,
                                                                UGridFlowTilemap* InTilemap) const {
    // List of tilemap coords that belong to the layout graph node coord
    TMap<FVector, TArray<FIntPoint>> TileCoordsByLayoutCoord;

    for (const FGridFlowTilemapCell& Cell : InTilemap->GetCells()) {
        if (Cell.CellType == EGridFlowTilemapCellType::Empty || Cell.CellType == EGridFlowTilemapCellType::Custom) {
            continue;
        }

        if (!Cell.bLayoutCell) {
            // Only consider layout cells
            continue;
        }

        TArray<FIntPoint>& NodeTiles = TileCoordsByLayoutCoord.FindOrAdd(Cell.ChunkCoord);
        NodeTiles.Add(Cell.TileCoord);
    }

    for (UFlowAbstractNode* Node : InGraph->GraphNodes) {
        if (!Node) continue;
        
        FGridFlowAbstractNodeTilemapMetadata& Metadata = Node->FindOrAddDomainData<UFANodeTilemapDomainData>()->TilemapMetadata;

        Metadata.Tiles = TileCoordsByLayoutCoord.FindOrAdd(Node->Coord);
        FIntPoint Min, Max;
        if (Metadata.Tiles.Num() > 0) {
            Min = Metadata.Tiles[0];
            Max = Metadata.Tiles[0];
            for (int i = 1; i < Metadata.Tiles.Num(); i++) {
                const FIntPoint& Tile = Metadata.Tiles[i];
                Min.X = FMath::Min(Min.X, Tile.X);
                Min.Y = FMath::Min(Min.Y, Tile.Y);

                Max.X = FMath::Max(Max.X, Tile.X);
                Max.Y = FMath::Max(Max.Y, Tile.Y);
            }
        }
        else {
            Min = FIntPoint::ZeroValue;
            Max = FIntPoint::ZeroValue;
        }

        Metadata.TileCoordStart = Min;
        Metadata.TileCoordEnd = Max;
    }
}

