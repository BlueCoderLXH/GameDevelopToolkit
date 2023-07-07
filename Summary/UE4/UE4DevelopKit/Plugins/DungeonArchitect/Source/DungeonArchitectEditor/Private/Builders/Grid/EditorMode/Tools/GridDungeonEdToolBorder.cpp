//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/Grid/EditorMode/Tools/GridDungeonEdToolBorder.h"

#include "Builders/Grid/EditorMode/GridDungeonEdModeHandler.h"
#include "Builders/Grid/EditorMode/GridDungeonEdModeRenderer.h"
#include "Builders/Grid/EditorMode/Tools/GridDungeonEdTool.h"
#include "Builders/Grid/EditorMode/Tools/GridDungeonEdToolData.h"
#include "Builders/Grid/GridDungeonToolData.h"
#include "Core/Dungeon.h"
#include "Core/LevelEditor/EditorMode/DungeonEdMode.h"

#include "DynamicMeshBuilder.h"
#include "EditorModes.h"
#include "EditorViewportClient.h"
#include "Materials/Material.h"

FName FGridDungeonEdToolBorder::ToolID = "Tool_Grid_Border";

FGridDungeonEdToolBorder::FGridDungeonEdToolBorder(UGridDungeonEdModeHandler* ModeHandler)
    : FGridDungeonEdTool(ModeHandler) {
    OverlayMaterial = LoadObject<UMaterial>(
        nullptr, TEXT("/DungeonArchitect/Core/Materials/M_DungeonEdModeOverlay.M_DungeonEdModeOverlay"), nullptr,
        LOAD_None, nullptr);
    bDragging = false;
}

void FGridDungeonEdToolBorder::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) {
    if (!OverlayMaterial) return;

    // Allocate the material proxy and register it so it can be deleted properly once the rendering is done with it.
    FDynamicColoredMaterialRenderProxy* SelectedColorInstance = new FDynamicColoredMaterialRenderProxy(
        OverlayMaterial->GetRenderProxy(), FLinearColor::White);
    PDI->RegisterDynamicResource(SelectedColorInstance);

    FDynamicMeshBuilder MeshBuilder(View->FeatureLevel);

    FIntVector CursorPosition, CursorSize;
    if (bDragging) {
        CursorPosition = BrushRectStart;
        CursorSize = BrushRectEnd - BrushRectStart + FIntVector(1, 1, 0);
    }
    else {
        CursorPosition = GridCursorPosition;
        CursorSize = FIntVector(1, 1, 0);
    }

    // Draw the cursor that follows the mouse in grid coordinates
    auto GridRenderer = GetGridRenderer();
    if (GridRenderer.IsValid()) {
        GridRenderer->DrawBorderCursor(View, Viewport, PDI, CursorPosition, MeshBuilder, CursorSize,
                                       ToolModel->BrushSize);
    }

    MeshBuilder.Draw(PDI, FMatrix::Identity, SelectedColorInstance, SDPG_Foreground, true, false);
}

void FGridDungeonEdToolBorder::BuildDungeon() {
    ADungeon* Dungeon = ModeHandler->GetActiveDungeon();
    if (!Dungeon) return;

    // Clear the stroke data for next paint operation
    PaintedStrokeData.Reset();

    FIntVector RectSize = BrushRectEnd - BrushRectStart + FIntVector(1, 1, 0);
    int32 Thickness = ToolModel->BrushSize;
    FRectangle InnerRect(Thickness, Thickness, RectSize.X - Thickness * 2, RectSize.Y - Thickness * 2);

    UGridDungeonToolData* GridToolData = Cast<UGridDungeonToolData>(Dungeon->GetToolData());
    TArray<FGridToolPaintStrokeData>& PaintedCells = GridToolData->PaintedCells;
    for (int dx = 0; dx < RectSize.X; dx++) {
        for (int dy = 0; dy < RectSize.Y; dy++) {
            if (InnerRect.Contains(dx, dy)) {
                continue;
            }
            FIntVector Position = BrushRectStart + FIntVector(dx, dy, 0);
            PaintedStrokeData.Add(Position);
        }
    }

    // Apply the stroke data
    ModeHandler->ApplyPaintData(FString("Apply Border Brush"), PaintedStrokeData, ToolModel->CellType, bRemoveMode);

    Dungeon->BuildDungeon();
}

bool FGridDungeonEdToolBorder::InputKey(FEditorViewportClient* InViewportClient, FViewport* InViewport, FKey InKey,
                                        EInputEvent InEvent) {
    bRemoveMode = InViewportClient->IsShiftPressed();

    if (InKey == EKeys::LeftMouseButton) {
        if (InEvent == IE_Pressed) {
            DragStart = GridCursorPosition;
            bDragging = true;
        }
        else if (InEvent == IE_Released) {
            bDragging = false;
            // Rebuild only when the user release the left mouse button
            BuildDungeon();
        }
    }

    return false;
}

void FGridDungeonEdToolBorder::ApplyBrush(FEditorViewportClient* ViewportClient) {
    // Update the drag start / end 
    BrushRectStart.X = FMath::Min(DragStart.X, GridCursorPosition.X);
    BrushRectStart.Y = FMath::Min(DragStart.Y, GridCursorPosition.Y);
    BrushRectStart.Z = GridCursorPosition.Z;

    BrushRectEnd.X = FMath::Max(DragStart.X, GridCursorPosition.X);
    BrushRectEnd.Y = FMath::Max(DragStart.Y, GridCursorPosition.Y);
    BrushRectEnd.Z = GridCursorPosition.Z;

}

