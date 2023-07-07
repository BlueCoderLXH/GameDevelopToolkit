//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

typedef FName FDungeonEdToolID;
class ADungeon;

class IDungeonEdTool {
public:
    virtual ~IDungeonEdTool() {
    }

    virtual FDungeonEdToolID GetToolType() const = 0;
    virtual void Initialize() = 0;
    virtual void Destroy() = 0;
    virtual void Update(const FSceneView* View, FViewport* Viewport) = 0;
    virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) = 0;

    virtual void ApplyBrush(FEditorViewportClient* ViewportClient) {
    }

    virtual bool InputKey(FEditorViewportClient* InViewportClient, FViewport* InViewport, FKey InKey,
                          EInputEvent InEvent) { return false; }

    virtual void OnUndo() {
    }

    virtual void OnRedo() {
    }

    virtual UObject* GetToolModel() const { return nullptr; }

    virtual FName GetToolFamily() const = 0;
};

