//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/Grid/EditorMode/GridDungeonEdModeRenderer.h"

#include "Builders/Grid/EditorMode/GridDungeonEdModeHandler.h"
#include "Builders/Grid/EditorMode/Tools/GridDungeonEdTool.h"
#include "Builders/Grid/GridDungeonConfig.h"
#include "Builders/Grid/GridDungeonModel.h"
#include "Builders/Grid/GridDungeonToolData.h"
#include "Core/Dungeon.h"
#include "Core/LevelEditor/EditorMode/DungeonEdModeHandler.h"
#include "Core/Utils/DungeonModelHelper.h"

#include "EditorModes.h"
#include "Materials/Material.h"
#include "SceneManagement.h"
#include "UnrealClient.h"

const FVector TangentX(1, 0, 0);
const FVector TangentY(0, 1, 0);
const FVector TangentZ(0, 0, 1);

FGridDungeonEdModeRenderer::FGridDungeonEdModeRenderer(UGridDungeonEdModeHandler* pModeHandler)
    : ModeHandler(pModeHandler) {
    OverlayMaterial = LoadObject<UMaterial>(
        nullptr, TEXT("/DungeonArchitect/Core/Materials/M_DungeonEdModeOverlay.M_DungeonEdModeOverlay"), nullptr,
        LOAD_None, nullptr);
}

void FGridDungeonEdModeRenderer::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI,
                                        TSharedPtr<IDungeonEdTool> ActiveTool) {
    if (!OverlayMaterial) return;

    TArray<FIntVector> StrokeData;
    if (ActiveTool.IsValid() && ActiveTool->GetToolFamily() == FGridDungeonEdTool::ToolFamily) {
        auto GridTool = StaticCastSharedPtr<FGridDungeonEdTool>(ActiveTool);
        //ActiveTool.IsValid() ? ActiveTool->GetStrokeData() : TArray<FIntVector>();
        StrokeData = GridTool->GetStrokeData();
    }

    FLinearColor SelectedColor = FLinearColor::White;
    SelectedColor.A = .6f;

    // Allocate the material proxy and register it so it can be deleted properly once the rendering is done with it.
    FDynamicColoredMaterialRenderProxy* SelectedColorInstance = new FDynamicColoredMaterialRenderProxy(
        OverlayMaterial->GetRenderProxy(), SelectedColor);
    PDI->RegisterDynamicResource(SelectedColorInstance);

    FDynamicMeshBuilder MeshBuilder(View->FeatureLevel);
    ADungeon* Dungeon = ModeHandler->GetActiveDungeon();
    if (Dungeon) {
        UGridDungeonConfig* GridConfig = Cast<UGridDungeonConfig>(Dungeon->GetConfig());
        UGridDungeonModel* GridModel = Cast<UGridDungeonModel>(Dungeon->GetModel());
        UGridDungeonToolData* GridToolData = Cast<UGridDungeonToolData>(Dungeon->GetToolData());

        const FVector& GridCellSize = GridConfig->GridCellSize;
        DrawDungeonCells(GridModel, GridCellSize, MeshBuilder);
        DrawPaintedCells(GridToolData->PaintedCells, GridCellSize, MeshBuilder);
        DrawPaintedCells(StrokeData, FColor::Red, GridCellSize, MeshBuilder);
    }

    MeshBuilder.Draw(PDI, FMatrix::Identity, SelectedColorInstance, SDPG_Foreground, true, false);
}

void FGridDungeonEdModeRenderer::Destroy() {
}

void FGridDungeonEdModeRenderer::DrawCursor(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI,
                                            FIntVector GridCursorPosition, FDynamicMeshBuilder& MeshBuilder,
                                            FIntVector CursorSize) {
    ADungeon* Dungeon = ModeHandler->GetActiveDungeon();
    if (!Dungeon) return;

    UGridDungeonConfig* GridConfig = Cast<UGridDungeonConfig>(Dungeon->GetConfig());
    const FVector& GridCellSize = GridConfig->GridCellSize;
    const FColor CursorColor = FColor::Red;
    const FColor CursorBorderColor = FColor::Black;

    FRectangle Bounds;
    Bounds.Location = GridCursorPosition;
    Bounds.Size = FIntVector(CursorSize.X, CursorSize.Y, 1);
    AddQuad(MeshBuilder, Bounds, GridCellSize, CursorColor);
    AddQuadBorder(PDI, Bounds, GridCellSize, CursorBorderColor, 5);
}

void FGridDungeonEdModeRenderer::DrawBorderCursor(const FSceneView* View, FViewport* Viewport,
                                                  FPrimitiveDrawInterface* PDI, FIntVector GridCursorPosition,
                                                  FDynamicMeshBuilder& MeshBuilder, FIntVector CursorSize,
                                                  int32 Thickness) {
    ADungeon* Dungeon = ModeHandler->GetActiveDungeon();
    if (!Dungeon) return;
    UGridDungeonConfig* GridConfig = Cast<UGridDungeonConfig>(Dungeon->GetConfig());
    const FVector& GridCellSize = GridConfig->GridCellSize;
    const FColor CursorColor = FColor::Red;
    const FColor CursorBorderColor = FColor::Black;

    FRectangle Bounds;
    Bounds.Location = GridCursorPosition;
    Bounds.Size = FIntVector(CursorSize.X, CursorSize.Y, 1);
    AddQuadBorder(PDI, Bounds, GridCellSize, CursorBorderColor, 5);

    if (CursorSize.X <= Thickness * 2 || CursorSize.Y <= Thickness * 2) {
        // If the rectangle is too small to have a hole, then don't draw the inner hole
        AddQuad(MeshBuilder, Bounds, GridCellSize, CursorColor);
    }
    else {
        AddQuad(MeshBuilder, Bounds, GridCellSize, CursorColor);

        // Draw the border for the inner hole
        Bounds.Location += FIntVector(Thickness, Thickness, 0);
        Bounds.Size -= FIntVector(Thickness, Thickness, 0) * 2;
        AddQuadBorder(PDI, Bounds, GridCellSize, CursorBorderColor, 5);
    }
}

void FGridDungeonEdModeRenderer::DrawDungeonCells(UGridDungeonModel* GridModel, const FVector& GridCellSize,
                                                  FDynamicMeshBuilder& MeshBuilder) {
    const FColor CellColor = FColor::Blue;
    for (const FCell& Cell : GridModel->Cells) {
        AddQuad(MeshBuilder, Cell.Bounds, GridCellSize, CellColor);
    }
}

void FGridDungeonEdModeRenderer::DrawPaintedCells(const TArray<FGridToolPaintStrokeData>& PaintedCells,
                                                  const FVector& GridCellSize, FDynamicMeshBuilder& MeshBuilder) {
    const FColor CellColor = FColor::Green;
    for (const FGridToolPaintStrokeData& Cell : PaintedCells) {
        FRectangle Bounds;
        Bounds.Location = Cell.Location;
        Bounds.Size = FIntVector(1, 1, 1);
        AddQuad(MeshBuilder, Bounds, GridCellSize, CellColor);
    }
}

void FGridDungeonEdModeRenderer::DrawPaintedCells(const TArray<FIntVector>& PaintedCells, const FColor& CellColor,
                                                  const FVector& GridCellSize, FDynamicMeshBuilder& MeshBuilder) {
    for (const FIntVector& CellLocation : PaintedCells) {
        FRectangle Bounds;
        Bounds.Location = CellLocation;
        Bounds.Size = FIntVector(1, 1, 1);
        AddQuad(MeshBuilder, Bounds, GridCellSize, CellColor);
    }
}

void GetQuadVertices(const FRectangle& GridBounds, const FVector& GridCellSize, FVector& V0, FVector& V1, FVector& V2,
                     FVector& V3) {
    FVector Location = UDungeonModelHelper::MakeVector(GridBounds.Location) * GridCellSize;
    FVector Size = UDungeonModelHelper::MakeVector(GridBounds.Size) * GridCellSize;
    const float X = Size.X;
    const float Y = Size.Y;
    V0 = Location;
    V1 = Location + FVector(0, Y, 0);
    V2 = Location + FVector(X, Y, 0);
    V3 = Location + FVector(X, 0, 0);
}

void FGridDungeonEdModeRenderer::AddQuad(FDynamicMeshBuilder& MeshBuilder, const FRectangle& GridBounds,
                                         const FVector& GridCellSize, const FColor& Color) {
    FVector V0, V1, V2, V3;
    GetQuadVertices(GridBounds, GridCellSize, V0, V1, V2, V3);
    AddQuad(MeshBuilder, V0, V1, V2, V3, Color);
}

void FGridDungeonEdModeRenderer::AddQuadBorder(FPrimitiveDrawInterface* PDI, const FRectangle& GridBounds,
                                               const FVector& GridCellSize, const FColor& Color, float Thickness) {
    FVector V[4];
    GetQuadVertices(GridBounds, GridCellSize, V[0], V[1], V[2], V[3]);
    for (int i = 0; i < 4; i++) {
        const FVector& V0 = V[i];
        const FVector& V1 = V[(i + 1) % 4];
        PDI->DrawLine(V0, V1, Color, SDPG_Foreground, Thickness, 0, true);
    }
}

void FGridDungeonEdModeRenderer::AddQuad(FDynamicMeshBuilder& MeshBuilder, const FVector& V0, const FVector& V1,
                                         const FVector& V2, const FVector& V3, const FColor& Color) {
    const int32 VertexOffset = MeshBuilder.AddVertex(V0, FVector2D::ZeroVector, TangentX, TangentY, TangentZ, Color);
    MeshBuilder.AddVertex(V1, FVector2D::ZeroVector, TangentX, TangentY, TangentZ, Color);
    MeshBuilder.AddVertex(V2, FVector2D::ZeroVector, TangentX, TangentY, TangentZ, Color);
    MeshBuilder.AddVertex(V3, FVector2D::ZeroVector, TangentX, TangentY, TangentZ, Color);

    MeshBuilder.AddTriangle(VertexOffset + 1, VertexOffset, VertexOffset + 2);
    MeshBuilder.AddTriangle(VertexOffset + 2, VertexOffset, VertexOffset + 3);
}

