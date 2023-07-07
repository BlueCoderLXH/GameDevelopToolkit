//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/LevelEditor/EditorMode/DungeonEdModeRenderer.h"

#include "DynamicMeshBuilder.h"

struct FRectangle;
class UGridDungeonModel;
class UGridDungeonEdModeHandler;
struct FGridToolPaintStrokeData;

class DUNGEONARCHITECTEDITOR_API FGridDungeonEdModeRenderer : public FDungeonEdModeRenderer {
public:
    FGridDungeonEdModeRenderer(UGridDungeonEdModeHandler* pModeHandler);
    virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI,
                        TSharedPtr<IDungeonEdTool> ActiveTool) override;
    virtual void Destroy() override;

    void DrawCursor(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI,
                    FIntVector GridCursorPosition, FDynamicMeshBuilder& MeshBuilder, FIntVector CursorSize);
    void DrawBorderCursor(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI,
                          FIntVector GridCursorPosition, FDynamicMeshBuilder& MeshBuilder, FIntVector CursorSize,
                          int32 Thickness);

private:
    void DrawDungeonCells(UGridDungeonModel* GridModel, const FVector& GridCellSize, FDynamicMeshBuilder& MeshBuilder);
    void DrawPaintedCells(const TArray<FGridToolPaintStrokeData>& PaintedCells, const FVector& GridCellSize,
                          FDynamicMeshBuilder& MeshBuilder);
    void DrawPaintedCells(const TArray<FIntVector>& PaintedCells, const FColor& CellColor, const FVector& GridCellSize,
                          FDynamicMeshBuilder& MeshBuilder);

    void AddQuad(FDynamicMeshBuilder& MeshBuilder, const FVector& V0, const FVector& V1, const FVector& V2,
                 const FVector& V3, const FColor& Color);
    void AddQuad(FDynamicMeshBuilder& MeshBuilder, const FRectangle& GridBounds, const FVector& GridCellSize,
                 const FColor& Color);
    void AddQuadBorder(FPrimitiveDrawInterface* PDI, const FRectangle& GridBounds, const FVector& GridCellSize,
                       const FColor& Color, float Thickness);

private:
    UGridDungeonEdModeHandler* ModeHandler;
    UMaterial* OverlayMaterial;

};

