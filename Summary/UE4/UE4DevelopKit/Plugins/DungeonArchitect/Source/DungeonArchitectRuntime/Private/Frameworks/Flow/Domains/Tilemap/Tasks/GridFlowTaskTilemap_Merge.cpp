//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/Tilemap/Tasks/GridFlowTaskTilemap_Merge.h"

#include "Frameworks/Flow/Domains/Tilemap/GridFlowTilemap.h"
#include "Frameworks/Flow/Domains/Tilemap/GridFlowTilemapDomain.h"

void UGridFlowTaskTilemap_Merge::Execute(const FFlowExecutionInput& Input, const FFlowTaskExecutionSettings& InExecSettings, FFlowExecutionOutput& Output) {
    if (Input.IncomingNodeOutputs.Num() == 0) {
        Output.ErrorMessage = "Missing Input";
        Output.ExecutionResult = EFlowTaskExecutionResult::FailHalt;
        return;
    }

    Output.State = Input.IncomingNodeOutputs[0].State->Clone();
    UGridFlowTilemap* Tilemap = Output.State->GetState<UGridFlowTilemap>(UGridFlowTilemap::StateTypeID);
    
    if (!Tilemap) {
        Output.ErrorMessage = "Invalid Input Tilemap";
        Output.ExecutionResult = EFlowTaskExecutionResult::FailHalt;
        return;
    }

    TArray<UGridFlowTilemap*> IncomingTilemaps;
    for (const FFlowExecutionOutput& IncomingNodeOutput : Input.IncomingNodeOutputs) {
        UGridFlowTilemap* IncomingTilemap = IncomingNodeOutput.State->GetState<UGridFlowTilemap>(UGridFlowTilemap::StateTypeID);
        if (IncomingTilemap) {
            IncomingTilemaps.Add(IncomingTilemap);
        }
    }


    for (int y = 0; y < Tilemap->GetHeight(); y++) {
        for (int x = 0; x < Tilemap->GetWidth(); x++) {
            int32 BestWeight = 0;
            FGridFlowTilemapCell* BestCell = nullptr;
            TArray<FGridFlowTilemapCellOverlay*> IncomingOverlays;
            for (UGridFlowTilemap* IncomingTilemap : IncomingTilemaps) {
                int32 Weight = 0;
                FGridFlowTilemapCell& IncomingCell = IncomingTilemap->Get(x, y);
                if (IncomingCell.CellType == EGridFlowTilemapCellType::Empty) {
                    Weight = 1;
                }
                else if (IncomingCell.CellType == EGridFlowTilemapCellType::Custom) {
                    Weight = 2;
                }
                else {
                    Weight = 3;
                }

                if (IncomingCell.bHasOverlay) {
                    IncomingOverlays.Add(&IncomingCell.Overlay);
                }

                bool bUseResult = false;
                if (Weight > BestWeight) {
                    bUseResult = true;
                }
                else if (Weight == BestWeight) {
                    if (BestCell && IncomingCell.Height > BestCell->Height) {
                        bUseResult = true;
                    }
                }

                if (bUseResult) {
                    BestCell = &IncomingCell;
                    BestWeight = Weight;
                }
            }

            FGridFlowTilemapCell& Cell = Tilemap->Get(x, y);
            if (!BestCell) {
                Cell.CellType = EGridFlowTilemapCellType::Empty;
                continue;
            }

            Cell = *BestCell;

            FGridFlowTilemapCellOverlay* BestOverlay = nullptr;
            float BestOverlayWeight = 0.0f;
            for (FGridFlowTilemapCellOverlay* IncomingOverlay : IncomingOverlays) {
                bool bValid = Cell.Height >= IncomingOverlay->MergeConfig.MinHeight
                    && Cell.Height <= IncomingOverlay->MergeConfig.MaxHeight;

                if (!bValid) continue;

                if (!BestOverlay || IncomingOverlay->NoiseValue > BestOverlayWeight) {
                    BestOverlay = IncomingOverlay;
                    BestOverlayWeight = IncomingOverlay->NoiseValue;
                }

            }

            if (BestOverlay) {
                Cell.bHasOverlay = true;
                Cell.Overlay = *BestOverlay;
            }

            if (Cell.CellType == EGridFlowTilemapCellType::Wall && Cell.bHasOverlay) {
                EGridFlowTilemapCellOverlayMergeWallOverlayRule WallOverlayRule = Cell
                                                                                  .Overlay.MergeConfig.WallOverlayRule;
                if (WallOverlayRule == EGridFlowTilemapCellOverlayMergeWallOverlayRule::KeepOverlayRemoveWall) {
                    Cell.CellType = EGridFlowTilemapCellType::Floor;
                    Cell.bUseCustomColor = true;
                }
                else if (WallOverlayRule == EGridFlowTilemapCellOverlayMergeWallOverlayRule::KeepWallRemoveOverlay) {
                    Cell.bHasOverlay = false;
                }
            }
        }
    }

    
    // Merge the edges
    for (int y = 0; y <= Tilemap->GetHeight(); y++) {
        for (int x = 0; x <= Tilemap->GetWidth(); x++) {
            
            const FGridFlowTilemapEdge* BestEdgeH = nullptr;
            const FGridFlowTilemapEdge* BestEdgeV = nullptr;
            for (UGridFlowTilemap* IncomingTilemap : IncomingTilemaps) {
                {
                    const FGridFlowTilemapEdge& IncomingEdgeH = IncomingTilemap->GetEdgeH(x, y);
                    if (IncomingEdgeH.EdgeType != EGridFlowTilemapEdgeType::Empty) {
                        BestEdgeH = &IncomingEdgeH;
                    }
                }
                {
                    FGridFlowTilemapEdge& IncomingEdgeV = IncomingTilemap->GetEdgeV(x, y);
                    if (IncomingEdgeV.EdgeType != EGridFlowTilemapEdgeType::Empty) {
                        BestEdgeV = &IncomingEdgeV;
                    }
                }
            }
            if (BestEdgeH != nullptr) {
                Tilemap->SetEdgeH(x, y, *BestEdgeH);
            }
            if (BestEdgeV != nullptr) {
                Tilemap->SetEdgeV(x, y, *BestEdgeV);
            }
        }
    }
    
    Output.ExecutionResult = EFlowTaskExecutionResult::Success;
}

