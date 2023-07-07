//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Misc/NotifyHook.h"
#include "Tickable.h"
#include "Toolkits/AssetEditorToolkit.h"

class ADungeon;
class UDungeonConfig;
class UFlowAssetBase;
class UGridFlowExecEdGraphNodeBase;
class SGraphEditor;
class SFlowPreview3DViewport;
class FToolBarBuilder;
class UFlowEditorSettings;
typedef TSharedPtr<class IFlowDomain> IFlowDomainPtr;
typedef TSharedPtr<class IFlowDomainEditor> IFlowDomainEditorPtr;
typedef TSharedPtr<class IFlowDomainEdMediator> IFlowDomainEdMediatorPtr;
typedef TSharedPtr<class FFlowExecNodeState> FFlowExecNodeStatePtr;

class FFlowEditorBase
    : public FAssetEditorToolkit
      , public FNotifyHook
      , public FTickableGameObject
      , public FGCObject {
public:
    // IToolkit interface
    virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
    virtual void UnregisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
    // End of IToolkit interface

    // FAssetEditorToolkit
    virtual FText GetToolkitName() const override;
    virtual FLinearColor GetWorldCentricTabColorScale() const override;
    virtual FString GetDocumentationLink() const override;
    // End of FAssetEditorToolkit

    // FNotifyHook interface
    virtual void NotifyPreChange(FProperty* PropertyAboutToChange) override;
    virtual void NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged) override;
    // End of FNotifyHook interface

    // FTickableGameObject Interface
    virtual bool IsTickableInEditor() const override;
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override;
    virtual TStatId GetStatId() const override;
    // End of FTickableGameObject Interface


    // FGCObject Interface
    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
    // End of FGCObject Interface

    void UpgradeAsset() const;
    void InitEditor(EToolkitMode::Type Mode, const TSharedPtr<class IToolkitHost>& InitToolkitHost, UFlowAssetBase* PropData);
    UFlowAssetBase* GetAssetBeingEdited() const;
    UFlowEditorSettings* GetEditorSettings() const { return EditorSettings; }

    void ShowObjectDetails(UObject* ObjectProperties, bool bForceRefresh = false); 
    TSharedPtr<IDetailsView> GetPropertyEditor() const;

protected:
    virtual void CreateDomainEditors() = 0;
    virtual FName GetFlowEdAppName() const = 0;
    virtual ADungeon* CreatePreviewDungeon(UWorld* World) = 0;
    virtual void InitDungeonConfig(UDungeonConfig* Config) {}
    virtual bool RequiresCustomFrameLayout() const { return false; }
    virtual TSharedPtr<FTabManager::FLayout> CreateFrameLayout() const { return nullptr; }
    virtual FText GetEditorBrandingText() const = 0;
    virtual void ConfigureDomainObject(IFlowDomainPtr Domain) {}
    virtual UFlowEditorSettings* CreateEditorSettingsObject() const;
    virtual TSharedRef<SWidget> CreatePerfWidget(const TSharedRef<SDockTab> DockTab, TSharedPtr<SWindow> OwnerWindow);
    virtual int32 GeNumAllowedFlowExecTimeouts() const;
    virtual bool ShouldBuildPreviewDungeon() const { return true; }
    
    template<typename TWidget>
    TSharedRef<TWidget> CreatePerfWidgetImpl(const TSharedRef<SDockTab> DockTab, TSharedPtr<SWindow> OwnerWindow) {
        return SNew(TWidget, DockTab, OwnerWindow, AssetBeingEdited)
            .OnServiceStarted(this, &FFlowEditorBase::OnTestRunnerServiceStarted);
    } 
    
    void ExtendMenu();
    void ExtendToolbar();

    void CreateExecGraphEditorWidget();
    void CreatePropertyEditorWidget();
    void CreatePreviewViewport();
    void CompileExecGraph();
    void ExecuteGraph();

    void SyncEdGraphNodeStates() const;

    void OnExecNodeDoubleClicked(UEdGraphNode* InNode);
    void OnExecNodeSelectionChanged(const TSet<class UObject*>& InSelectedObjects);
    void UpdatePreviewGraphs(const FGuid& NodeId);

    /** Called when "Save" is clicked for this asset */
    virtual void SaveAsset_Execute() override;
    TSharedRef<SDockTab> SpawnTab_ExecGraph(const FSpawnTabArgs& Args);
    TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);
    TSharedRef<SDockTab> SpawnTab_Viewport(const FSpawnTabArgs& Args);
    TSharedRef<SDockTab> SpawnTab_Performance(const FSpawnTabArgs& Args);
    TSharedRef<SDockTab> SpawnTab_DomainEditor(const FSpawnTabArgs& Args, FName DomainID, FText Title);

    void BindCommands();
    void RecenterOutputGraphs(const FGuid& InNodeId);
    void RebuildDungeon();
    void UpdateAssetThumbnail();
    FFlowExecNodeStatePtr GetNodeState(const FGuid& InNodeId) const;
    UDungeonConfig* GetDungeonConfig() const;

    TSharedRef<SWidget> HandleShowSettingsDropDownMenu() const;
    void FillToolbarCore(FToolBarBuilder& ToolbarBuilder) const;
    void FillToolbarMisc(FToolBarBuilder& ToolbarBuilder) const;
    void HandleShowEditorSettings();
    void HandleShowDungeonSettings();
    void HandleShowPerformanceDialog();
    virtual void OnTestRunnerServiceStarted();
    FName ConstructLayoutName(const FString& Version) const;
    TSharedRef<FTabManager::FSplitter> CreateDomainEditorLayout() const;
    TArray<IFlowDomainPtr> GetDomainList() const;
    
protected:
    TSharedPtr<SGraphEditor> ExecGraphEditor;
    TSharedPtr<IDetailsView> PropertyEditor;
    TSharedPtr<SFlowPreview3DViewport> PreviewViewport;

    TSharedPtr<class FGridFlowExecGraphHandler> ExecGraphHandler;
    TSharedPtr<class FFlowProcessor> ExecGraphProcessor;

    UFlowAssetBase* AssetBeingEdited = nullptr;

    TSharedPtr<class FUICommandList> SettingsActionList;
    UFlowEditorSettings* EditorSettings = nullptr;

    TArray<IFlowDomainEditorPtr> DomainEditors;
    TArray<IFlowDomainEdMediatorPtr> DomainMediators;

protected:
    struct FFlowEditorTabs {
        // Tab identifiers
        static const FName ExecGraphID;
        static const FName DetailsID;
        static const FName ViewportID;
        static const FName PerformanceID;
    };
};

