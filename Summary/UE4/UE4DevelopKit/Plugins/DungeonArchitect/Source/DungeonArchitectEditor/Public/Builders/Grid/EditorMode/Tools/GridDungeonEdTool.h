//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/LevelEditor/EditorMode/IDungeonEdTool.h"

class UGridDungeonEdToolData;
class UGridDungeonEdModeHandler;
class FGridDungeonEdModeRenderer;


class FGridDungeonEdTool : public IDungeonEdTool, public FGCObject {
public:
    FGridDungeonEdTool(UGridDungeonEdModeHandler* ModeHandler);

    virtual ~FGridDungeonEdTool() {
    }

    virtual void Initialize() override;
    virtual void Update(const FSceneView* View, FViewport* Viewport) override;
    virtual void Destroy() override;

    virtual void ApplyBrush(FEditorViewportClient* ViewportClient) override {
    }

    virtual bool InputKey(FEditorViewportClient* InViewportClient, FViewport* InViewport, FKey InKey,
                          EInputEvent InEvent) override { return false; }

    virtual void OnUndo() override;
    virtual void OnRedo() override;
    virtual UObject* GetToolModel() const override;
    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

    FIntVector GetGridCursorPosition() const { return GridCursorPosition; }

    TArray<FIntVector> GetStrokeData() const {
        return PaintedStrokeData;
    }

    void UpdateGridCursorPosition(const FSceneView* View, FViewport* Viewport);

    TSharedPtr<FGridDungeonEdModeRenderer> GetGridRenderer();
    virtual FName GetToolFamily() const override;

    static FName ToolFamily;

protected:
    virtual UGridDungeonEdToolData* CreateToolModel();

protected:
    FIntVector GridCursorPosition;
    UGridDungeonEdToolData* ToolModel;

    /** Data that holds the paint stroke information for one drag operation (mouse down, then up) */
    TArray<FIntVector> PaintedStrokeData;

    UGridDungeonEdModeHandler* ModeHandler;

};

