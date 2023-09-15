//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/Grid/EditorMode/GridDungeonEdModeHandler.h"

#include "Builders/Grid/EditorMode/GridDungeonEdModeRenderer.h"
#include "Builders/Grid/EditorMode/Tools/GridDungeonEdTool.h"
#include "Builders/Grid/EditorMode/UI/SGridDungeonEdit.h"
#include "Builders/Grid/GridDungeonToolData.h"
#include "Core/Dungeon.h"

#include "ScopedTransaction.h"

DEFINE_LOG_CATEGORY(GridDungeonEdModeHandlerLog);

TSharedPtr<FDungeonEdModeRenderer> UGridDungeonEdModeHandler::CreateRenderer() {
    return MakeShareable(new FGridDungeonEdModeRenderer(this));
}

void UGridDungeonEdModeHandler::OnUpdate(const FSceneView* View, FViewport* Viewport) {
    UDungeonEdModeHandler::OnUpdate(View, Viewport);
}

void UGridDungeonEdModeHandler::SetActiveTool(TSharedPtr<IDungeonEdTool> NewTool) {
    if (!NewTool.IsValid()) {
        UE_LOG(GridDungeonEdModeHandlerLog, Warning, TEXT("Tool is not valid"));
        ActiveTool = nullptr;
        return;
    }

    if (NewTool->GetToolFamily() != FGridDungeonEdTool::ToolFamily) {
        UE_LOG(GridDungeonEdModeHandlerLog, Warning, TEXT("Tool is not supported with this builder"));
        ActiveTool = nullptr;
        return;
    }

    UDungeonEdModeHandler::SetActiveTool(NewTool);
}

TSharedPtr<SWidget> UGridDungeonEdModeHandler::CreateToolkitWidget() {
    return SNew(SGridDungeonEdit);
}

void UGridDungeonEdModeHandler::ApplyPaintData(const FString& TransactionName, const TArray<FIntVector>& StrokeData,
                                               EGridPaintToolCellType CellType, bool bRemove) {
    if (!ActiveDungeon) return;

    const FScopedTransaction Transaction(FText::FromString(*TransactionName));
    ActiveDungeon->Modify();
    bool bDataModelChanged = false;

    UDungeonToolData* ToolData = ActiveDungeon->GetToolData();
    UGridDungeonToolData* GridToolData = Cast<UGridDungeonToolData>(ToolData);

    TArray<FGridToolPaintStrokeData>& PaintedCells = GridToolData->PaintedCells;
    // Remove all cells that lie in the stroke location
    for (const FIntVector& StrokeCell : StrokeData) {
        for (int i = 0; i < PaintedCells.Num();) {
            if (StrokeCell.X == PaintedCells[i].Location.X && StrokeCell.Y == PaintedCells[i].Location.Y) {
                PaintedCells.RemoveAt(i);
            }
            else {
                i++;
            }
        }
    }

    if (!bRemove) {
        for (const FIntVector& StrokeCell : StrokeData) {
            FGridToolPaintStrokeData CellData;
            CellData.Location = StrokeCell;
            CellData.CellType = CellType;
            PaintedCells.Add(CellData);
        }
    }
}

