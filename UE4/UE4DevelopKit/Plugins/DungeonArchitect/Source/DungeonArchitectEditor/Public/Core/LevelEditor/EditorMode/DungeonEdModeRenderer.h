//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "DynamicMeshBuilder.h"

class IDungeonEdTool;

class DUNGEONARCHITECTEDITOR_API FDungeonEdModeRenderer {
public:
    virtual ~FDungeonEdModeRenderer() {
    }

    virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI,
                        TSharedPtr<IDungeonEdTool> ActiveTool) = 0;
    virtual void Destroy();

};

