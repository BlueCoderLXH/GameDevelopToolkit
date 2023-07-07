//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Editors/SnapMapEditor/AppModes/FlowEdAppModeBase.h"
#include "Core/Editors/SnapMapEditor/SnapMapGraphEditorHandlers.h"

#include "GameFramework/Actor.h"
#include "Misc/NotifyHook.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"
#include "DebugAppMode.generated.h"

class UEdGraph_Grammar;
class UEdGraph_DebugGrammar;
class SSnapMapEditorViewport;
class FSnapMapEditor;
class SAdvancedPreviewDetailsTab;
class IDetailsView;

namespace SnapLib {
    class FDiagnostics;
    struct FDiagStep;
    struct IPayload;
}

class FSnapMapEditor_DebugAppMode
    : public FSnapMapEdAppModeBase
      , public FGCObject
      , public FNotifyHook {
public:
    FSnapMapEditor_DebugAppMode(TSharedPtr<class FSnapMapEditor> InFlowEditor);
    virtual ~FSnapMapEditor_DebugAppMode();
    virtual void RegisterTabFactories(TSharedPtr<class FTabManager> InTabManager) override;
    virtual void PreDeactivateMode() override;
    virtual void PostActivateMode() override;

    // FGCObject Interface
    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
    // End of FGCObject Interface

    // FNotifyHook Interface
    virtual void NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged) override;
    // End of FNotifyHook Interface

    void BindCommands(TSharedRef<FUICommandList> ToolkitCommands);
    void OnResultNodeDoubleClicked(UEdGraphNode* InNode);
    void HandleDebugRestart();
    void HandleDebugStepForward();

protected:
    TSharedRef<FTabManager::FLayout> BuildEditorFrameLayout(TSharedPtr<class FSnapMapEditor> InFlowEditor);

protected:
    TWeakPtr<FSnapMapEditor> FlowEditor;
    FWorkflowAllowedTabSet TabFactories;

    FSnapMapResultGraphEditorHandler DebugGraphEditor;
    TSharedPtr<SSnapMapEditorViewport> Viewport;
    TSharedPtr<SAdvancedPreviewDetailsTab> ViewportSceneSettings;
    TSharedPtr<IDetailsView> PropertyEditor;

    class ASnapMapFlowEditorVisualization* VisualizationActor;
};

/////////////////////// ASnapMapFlowEditorVisualization ///////////////////////

UCLASS(HideDropdown)
class ASnapMapFlowEditorVisualization : public AActor {
    GENERATED_BODY()

public:
    ASnapMapFlowEditorVisualization();
    void LoadLevel(const FGuid& InNodeId, TSoftObjectPtr<UWorld> ModuleLevel,
                             const FBox& ModuleBounds, const FTransform& InWorldTransform);
    void UnloadLevel(const FGuid& InNodeId);
    void UpdateConnectionState(const FGuid& InNodeId, const FGuid& InConnectionId, bool bIsDoor);
    void UnloadAllLevels();
    void SetDebugBox(const FBox& InDebugDrawBounds, const FColor& InColor);

    virtual void Tick(float DeltaSeconds) override;
    virtual bool ShouldTickIfViewportsOnly() const override { return true; }

private:
    void DebugDraw();

private:
    UPROPERTY(Transient)
    TMap<FGuid, class ULevelStreamingDynamic*> LoadedLevels;

    UPROPERTY(Transient)
    TMap<FGuid, UPackage*> LoadedPackages;

    UPROPERTY(Transient)
    FBox DebugDrawBounds;

    UPROPERTY(Transient)
    FColor DebugDrawColor;

    int32 InstanceIdCounter;
};

