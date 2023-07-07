//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/LevelEditor/EditorMode/IDungeonEdTool.h"

class USnapMapEdToolData;
class USnapMapEdModeHandler;
class FSnapMapEdModeRenderer;

class FSnapMapEdTool : public IDungeonEdTool, public FGCObject {
public:
    FSnapMapEdTool(USnapMapEdModeHandler* ModeHandler);

    virtual ~FSnapMapEdTool() {
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

    TSharedPtr<FSnapMapEdModeRenderer> GetSnapMapRenderer();
    virtual FName GetToolFamily() const override;

    static FName ToolFamily;

protected:
    USnapMapEdModeHandler* ModeHandler;

};

