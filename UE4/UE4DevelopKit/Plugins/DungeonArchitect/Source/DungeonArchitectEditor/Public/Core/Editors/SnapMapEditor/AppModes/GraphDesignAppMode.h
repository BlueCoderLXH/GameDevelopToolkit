//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Editors/SnapMapEditor/AppModes/FlowEdAppModeBase.h"
#include "Core/Editors/SnapMapEditor/SnapMapGraphEditorHandlers.h"

#include "Misc/NotifyHook.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"
#include "GraphDesignAppMode.generated.h"

class SSnapMapTestRunner;
class SGrammarEditor;
class SGrammarErrorList;
class IDetailsView;
class FUICommandList;
class UEdGraph_Grammar;

UCLASS()
class USnapMapEditor_GraphDesignAppModeSettings : public UObject {
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category = Settings)
    int32 Seed = 0;
    
    UPROPERTY(EditAnywhere, Category = Settings)
    bool bRandomizeSeed = true;

    /** Automatically zoom's and fits the result graph when clicking build */
    UPROPERTY(EditAnywhere, Category = Settings)
    bool bAutoZoomResultGraph = true;

};


//////////////////////// Graph Design Mode ////////////////////////
class FSnapMapEditor_GraphDesignAppMode
    : public FSnapMapEdAppModeBase
      , public FGCObject
      , public FNotifyHook {
public:
    FSnapMapEditor_GraphDesignAppMode(TSharedPtr<class FSnapMapEditor> InFlowEditor);
    virtual void RegisterTabFactories(TSharedPtr<class FTabManager> InTabManager) override;
    virtual void PreDeactivateMode() override;
    virtual void PostActivateMode() override;

    void OnGrammarStateChanged();

    // FNotifyHook interface
    virtual void NotifyPreChange(FProperty* PropertyAboutToChange) override;
    virtual void NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged) override;
    // End of FNotifyHook interface

    // FFlowEdAppModeBase interface
    virtual void Tick(float DeltaTime) override;
    // End of FFlowEdAppModeBase interface

    // FGCObject interface
    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
    // End of FGCObject interface

    void BindCommands(TSharedRef<FUICommandList> ToolkitCommands);
    void ValidateGrammar();

    void OnClick_ValidateGrammar();
    void OnClick_ExecuteDesignGraph();
    void OnClick_Settings();
    void OnClick_Performance();

protected:
    void FlashEditorTab(const FName& InTabId) const;
    
    TSharedRef<FTabManager::FLayout> BuildEditorFrameLayout(TSharedPtr<class FSnapMapEditor> InFlowEditor);
    
protected:
    TWeakPtr<FSnapMapEditor> FlowEditor;

    // Set of spawnable tabs in behavior tree editing mode
    FWorkflowAllowedTabSet FlowEditorTabFactories;

    UEdGraph_Grammar* ResultGraph;
    TSharedPtr<IDetailsView> PropertyEditor;
    TSharedPtr<SGrammarEditor> GrammarEditor;
    TSharedPtr<SGrammarErrorList> ErrorListView;
    TSharedPtr<SWidget> ExecutionGraphPanel;
    FSnapMapExecGraphEditorHandler ExecutionGraphEditor;
    FSnapMapResultGraphEditorHandler ResultGraphEditor;
    USnapMapEditor_GraphDesignAppModeSettings* Settings = nullptr;

    bool bRequestValidation;
};

