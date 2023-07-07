//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/LevelEditor/EditorMode/DungeonEdModeHandler.h"
#include "SnapMapEdModeHandler.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(SnapMapEdModeHandlerLog, Log, All);

UCLASS()
class USnapMapEdModeHandler : public UDungeonEdModeHandler {
    GENERATED_BODY()

public:
    virtual TSharedPtr<FDungeonEdModeRenderer> CreateRenderer() override;
    virtual void SetActiveTool(TSharedPtr<IDungeonEdTool> NewTool) override;
    virtual TSharedPtr<SWidget> CreateToolkitWidget() override;

protected:
    virtual void OnUpdate(const FSceneView* View, FViewport* Viewport) override;
};

