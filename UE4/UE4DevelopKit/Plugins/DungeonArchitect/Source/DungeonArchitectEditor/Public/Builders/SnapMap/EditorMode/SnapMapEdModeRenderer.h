//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/LevelEditor/EditorMode/DungeonEdModeRenderer.h"

class USnapMapModel;
class USnapMapEdModeHandler;

class DUNGEONARCHITECTEDITOR_API FSnapMapEdModeRenderer : public FDungeonEdModeRenderer {
public:
    FSnapMapEdModeRenderer(USnapMapEdModeHandler* pModeHandler);
    virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI,
                        TSharedPtr<IDungeonEdTool> ActiveTool) override;
    virtual void Destroy() override;

private:
    USnapMapEdModeHandler* ModeHandler;
};

