//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/Grid/EditorMode/Tools/GridDungeonEdToolRectangle.h"

#include "Builders/Grid/EditorMode/GridDungeonEdModeHandler.h"
#include "Builders/Grid/EditorMode/GridDungeonEdModeRenderer.h"
#include "Builders/Grid/EditorMode/Tools/GridDungeonEdTool.h"
#include "Builders/Grid/EditorMode/Tools/GridDungeonEdToolData.h"
#include "Core/Dungeon.h"
#include "Core/LevelEditor/EditorMode/DungeonEdMode.h"

#include "DynamicMeshBuilder.h"
#include "EditorModes.h"
#include "EditorViewportClient.h"
#include "Materials/Material.h"

FName FGridDungeonEdToolRectangle::ToolID = "Tool_Grid_Rectangle";

FGridDungeonEdToolRectangle::FGridDungeonEdToolRectangle(UGridDungeonEdModeHandler* ModeHandler)
    : FGridDungeonEdTool(ModeHandler) {
    OverlayMaterial = LoadObject<UMaterial>(
        nullptr, TEXT("/DungeonArchitect/Core/Materials/M_DungeonEdModeOverlay.M_DungeonEdModeOverlay"), nullptr,
        LOAD_None, nullptr);
    bDragging = false;
}

void FGridDungeonEdToolRectangle::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) {
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
        GridRenderer->DrawCursor(View, Viewport, PDI, CursorPosition, MeshBuilder, CursorSize);
    }

    MeshBuilder.Draw(PDI, FMatrix::Identity, SelectedColorInstance, SDPG_Foreground, true, false);
}

void FGridDungeonEdToolRectangle::BuildDungeon() {
    ADungeon* Dungeon = ModeHandler->GetActiveDungeon();
    if (!Dungeon) return;

    FIntVector RectSize = BrushRectEnd - BrushRectStart + FIntVector(1, 1, 0);

    // Clear the stroke data for next paint operation
    PaintedStrokeData.Reset();

    for (int dx = 0; dx < RectSize.X; dx++) {
        for (int dy = 0; dy < RectSize.Y; dy++) {
            FIntVector Position = BrushRectStart + FIntVector(dx, dy, 0);
            PaintedStrokeData.Add(Position);
        }
    }

    // Apply the stroke data
    ModeHandler->ApplyPaintData(FString("Apply Rectangle Brush"), PaintedStrokeData, ToolModel->CellType, bRemoveMode);

    Dungeon->BuildDungeon();
}

bool FGridDungeonEdToolRectangle::InputKey(FEditorViewportClient* InViewportClient, FViewport* InViewport, FKey InKey,
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

void FGridDungeonEdToolRectangle::ApplyBrush(FEditorViewportClient* ViewportClient) {
    // Update the drag start / end 
    BrushRectStart.X = FMath::Min(DragStart.X, GridCursorPosition.X);
    BrushRectStart.Y = FMath::Min(DragStart.Y, GridCursorPosition.Y);
    BrushRectStart.Z = GridCursorPosition.Z;

    BrushRectEnd.X = FMath::Max(DragStart.X, GridCursorPosition.X);
    BrushRectEnd.Y = FMath::Max(DragStart.Y, GridCursorPosition.Y);
    BrushRectEnd.Z = GridCursorPosition.Z;

}

