//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/Grid/EditorMode/Tools/GridDungeonEdTool.h"

#include "Builders/Grid/EditorMode/GridDungeonEdModeHandler.h"
#include "Builders/Grid/EditorMode/GridDungeonEdModeRenderer.h"
#include "Builders/Grid/EditorMode/Tools/GridDungeonEdToolData.h"
#include "Builders/Grid/GridDungeonConfig.h"
#include "Core/Dungeon.h"
#include "Core/LevelEditor/EditorMode/DungeonEdModeHandler.h"

FName FGridDungeonEdTool::ToolFamily = "Grid";

FGridDungeonEdTool::FGridDungeonEdTool(UGridDungeonEdModeHandler* InModeHandler) : ModeHandler(InModeHandler) {
}

void FGridDungeonEdTool::Initialize() {
    ToolModel = CreateToolModel();
}

void FGridDungeonEdTool::Update(const FSceneView* View, FViewport* Viewport) {
    // Finds the cursor position in grid coordinates
    UpdateGridCursorPosition(View, Viewport);
}

void FGridDungeonEdTool::Destroy() {
    ToolModel = nullptr;
}

void FGridDungeonEdTool::OnUndo() {
    PaintedStrokeData.Reset();
}

void FGridDungeonEdTool::OnRedo() {
    PaintedStrokeData.Reset();
}

void FGridDungeonEdTool::AddReferencedObjects(FReferenceCollector& Collector) {
    if (ToolModel) {
        Collector.AddReferencedObject(ToolModel);
    }
}

UObject* FGridDungeonEdTool::GetToolModel() const {
    return ToolModel;
}

TSharedPtr<FGridDungeonEdModeRenderer> FGridDungeonEdTool::GetGridRenderer() {
    if (!ModeHandler) {
        return nullptr;
    }
    return StaticCastSharedPtr<FGridDungeonEdModeRenderer>(ModeHandler->GetRenderer());
}

FName FGridDungeonEdTool::GetToolFamily() const {
    return ToolFamily;
}

UGridDungeonEdToolData* FGridDungeonEdTool::CreateToolModel() {
    return NewObject<UGridDungeonEdToolData>();
}

void FGridDungeonEdTool::UpdateGridCursorPosition(const FSceneView* View, FViewport* Viewport) {

    ADungeon* ActiveDungeon = ModeHandler->GetActiveDungeon();
    int32 FloorIndex = ToolModel->FloorIndex;
    if (!ActiveDungeon) return;
    UGridDungeonConfig* GridConfig = Cast<UGridDungeonConfig>(ActiveDungeon->GetConfig());
    if (!GridConfig) {
        return;
    }
    const FVector& GridCellSize = GridConfig->GridCellSize;

    FVector2D MousePosition(Viewport->GetMouseX(), Viewport->GetMouseY());
    FVector WorldOrigin, WorldDirection;
    View->DeprojectFVector2D(MousePosition, WorldOrigin, WorldDirection);
    FVector LineStart = WorldOrigin;
    FVector LineEnd = WorldOrigin + WorldDirection * 10000;
    FVector PlaneOrigin(0, 0, FloorIndex * GridCellSize.Z);
    FVector IntersectionPoint = FMath::LinePlaneIntersection(LineStart, LineEnd, PlaneOrigin, FVector(0, 0, 1));

    // Round this off
    int32 GridX = FMath::FloorToInt(IntersectionPoint.X / GridCellSize.X);
    int32 GridY = FMath::FloorToInt(IntersectionPoint.Y / GridCellSize.Y);
    int32 GridZ = FloorIndex; // FMath::FloorToInt(IntersectionPoint.Z / GridCellSize.Z);

    GridCursorPosition = FIntVector(GridX, GridY, GridZ);
}

