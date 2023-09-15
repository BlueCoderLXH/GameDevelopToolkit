//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Editors/SnapMapEditor/AppModes/FlowEdAppModeBase.h"
#include "Core/Editors/SnapMapEditor/SnapMapGraphEditorHandlers.h"

#include "WorkflowOrientedApp/WorkflowTabManager.h"

class ADungeon;
class FSnapMapEditor;
class IDetailsView;
class UEdGraph_Grammar;
class SSnapMapEditorViewport;
class SAdvancedPreviewDetailsTab;
class USnapMapEditorCallbackHandler;

//////////////////////// Visualize Mode ////////////////////////
class FSnapMapEditor_VisualizeAppMode
    : public FSnapMapEdAppModeBase
      , public FGCObject {
public:
    FSnapMapEditor_VisualizeAppMode(TSharedPtr<class FSnapMapEditor> InFlowEditor);
    virtual ~FSnapMapEditor_VisualizeAppMode();

    virtual void RegisterTabFactories(TSharedPtr<class FTabManager> InTabManager) override;
    virtual void PreDeactivateMode() override;
    virtual void PostActivateMode() override;

    // FFlowEdAppModeBase Interface
    virtual void OnAssetSave() override;
    // End of FFlowEdAppModeBase Interface

    // FGCObject Interface
    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
    // End of FGCObject Interface

protected:
    TSharedRef<FTabManager::FLayout> BuildEditorFrameLayout(TSharedPtr<class FSnapMapEditor> InFlowEditor);

    ADungeon* GetDungeonActor() const;
    void LoadDungeonConfig();
    void SaveDungeonConfig();
    void OnDungeonBuilt(ADungeon* Dungeon);
    void OnDungeonDestroyed(ADungeon* Dungeon);
    void OnResultNodeDoubleClicked(UEdGraphNode* InNode);

    void ResetResultGraph();
    void InitDungeon(UWorld* World);
    void OnMapChanged(UWorld* InWorld, EMapChangeType InChangeType);

    void BindCommands(TSharedRef<FUICommandList> ToolkitCommands);
    
protected:
    TWeakPtr<FSnapMapEditor> FlowEditor;
    FWorkflowAllowedTabSet TabFactories;
    USnapMapEditorCallbackHandler* CallbackHandler = nullptr;

    FSnapMapResultGraphEditorHandler VisualizationGraphEditor;
    TSharedPtr<SSnapMapEditorViewport> Viewport;
    TSharedPtr<SAdvancedPreviewDetailsTab> ViewportSceneSettings;
    TSharedPtr<IDetailsView> PropertyEditor;

    ADungeon* Dungeon;
    FDelegateHandle OnMapChangedDelegateHandle;
};

