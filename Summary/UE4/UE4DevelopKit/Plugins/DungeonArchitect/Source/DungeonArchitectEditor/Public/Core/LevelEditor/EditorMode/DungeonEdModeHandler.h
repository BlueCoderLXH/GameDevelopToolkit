//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Engine/EngineBaseTypes.h"
#include "InputCoreTypes.h"
#include "DungeonEdModeHandler.generated.h"

class IDungeonEdTool;
class FDungeonEdModeRenderer;
class ADungeon;
class FEditorViewportClient;
class FViewport;
class SWidget;
class FSceneView;
class FPrimitiveDrawInterface;

UCLASS(abstract)
class DUNGEONARCHITECTEDITOR_API UDungeonEdModeHandler : public UObject {
    GENERATED_BODY()

public:
    virtual void Enter();
    virtual void Exit();

    virtual void Undo();
    virtual void Redo();

    virtual bool InputKey(FEditorViewportClient* InViewportClient, FViewport* InViewport, FKey InKey,
                          EInputEvent InEvent);
    virtual void ApplyBrush(FEditorViewportClient* ViewportClient);
    virtual void SetActiveTool(TSharedPtr<IDungeonEdTool> NewTool);

    virtual TSharedPtr<FDungeonEdModeRenderer> CreateRenderer() { return nullptr; }

    void UpdateFrame(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI);
    void InitializeSettings();
    void RebuildDungeon();
    void OnDungeonSelectionChanged(ADungeon* pDungeon);
    TSharedPtr<FDungeonEdModeRenderer> GetRenderer() const;

    TSharedPtr<IDungeonEdTool> GetActiveTool() const { return ActiveTool; }
    ADungeon* GetActiveDungeon() const;
    virtual TSharedPtr<SWidget> CreateToolkitWidget() { return nullptr; }

protected:
    virtual void OnUpdate(const FSceneView* View, FViewport* Viewport);
    virtual void OnRender(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI);

protected:
    ADungeon* ActiveDungeon;
    TSharedPtr<IDungeonEdTool> ActiveTool;
    TSharedPtr<FDungeonEdModeRenderer> Renderer;
};

