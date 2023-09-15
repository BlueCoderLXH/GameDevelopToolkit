//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Builders/Grid/EditorMode/Tools/GridDungeonEdTool.h"

class FGridDungeonEdToolBorder : public FGridDungeonEdTool {
public:
    FGridDungeonEdToolBorder(UGridDungeonEdModeHandler* ModeHandler);
    virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) override;
    virtual bool InputKey(FEditorViewportClient* InViewportClient, FViewport* InViewport, FKey InKey,
                          EInputEvent InEvent) override;
    virtual void ApplyBrush(FEditorViewportClient* ViewportClient) override;

    virtual FDungeonEdToolID GetToolType() const override { return ToolID; }
    static FName ToolID;

private:
    void BuildDungeon();

private:
    UMaterial* OverlayMaterial;
    bool bRemoveMode;

    FIntVector DragStart;
    FIntVector BrushRectStart;
    FIntVector BrushRectEnd;

    bool bDragging;
};

