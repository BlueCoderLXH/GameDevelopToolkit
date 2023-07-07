//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Misc/NotifyHook.h"
#include "Tickable.h"
#include "Toolkits/AssetEditorManager.h"

class USnapConnectionComponent;
class UEdGraph_DungeonProp;
class USnapConnectionInfo;
typedef TSharedPtr<class FSnapConnectionThemeGraphHandler> FSnapConnectionThemeGraphHandlerPtr;

class FSnapConnectionEditor
        : public FAssetEditorToolkit
        , public FNotifyHook
        , public FTickableGameObject
        , public FGCObject
{
public:
    ~FSnapConnectionEditor();
    // IToolkit interface
    virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
    virtual void UnregisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
    // End of IToolkit interface

    // FAssetEditorToolkit Interface
    virtual FName GetToolkitFName() const override;
    virtual FText GetBaseToolkitName() const override;
    virtual FText GetToolkitName() const override;
    virtual FLinearColor GetWorldCentricTabColorScale() const override;
    virtual FString GetWorldCentricTabPrefix() const override;
    virtual FString GetDocumentationLink() const override;
    // End of FAssetEditorToolkit

    
    // FGCObject Interface
    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
    // End of FGCObject

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

    void UpgradeAsset() const;
    virtual void InitSnapConnectionEditor(EToolkitMode::Type Mode,
                                             const TSharedPtr<class IToolkitHost>& InitToolkitHost,
                                             USnapConnectionInfo* TreeTheme);
    USnapConnectionInfo* GetDoorBeingEdited() const { return AssetBeingEdited; }

protected:
    void ExtendMenu();
    void BindCommands();
    void HandleRebuildActionExecute();
    
private:
    /** Called when "Save" is clicked for this asset */
    virtual void SaveAsset_Execute() override;
    void CompileAsset() const;
    void UpdateThumbnail() const;
    void HandleAssetDropped(UObject* AssetObject);
    FVector2D GetAssetDropGridLocation() const;
    bool IsAssetAcceptableForDrop(const UObject* AssetObject) const;
    UEdGraph* CreateNewThemeGraph() const;

    void DestroyPreviewObjects();
    void RequestRebuildPreviewObject();
    void RebuildPreviewObjectImpl();
    void SetPreviewMode(const FString& InMarkerName);
    void PostLoadInitThemeGraph() const;
    void OnGraphChanged(const FEdGraphEditAction& Action);
    void OnNodeSelectionChanged(const TSet<class UObject*>& NewSelection);
    static void SetConnectionStateFromMarker(const FString& InMarkerName, USnapConnectionComponent* ConnectionComponent);
    
    TSharedRef<SDockTab> SpawnTab_Graph(const FSpawnTabArgs& Args);
    TSharedRef<SDockTab> SpawnTab_Preview3D(const FSpawnTabArgs& Args);
    TSharedRef<SDockTab> SpawnTab_PreviewSettings(const FSpawnTabArgs& Args);
    TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);
    TSharedRef<SDockTab> SpawnTab_ContentBrowser(const FSpawnTabArgs& Args);
    
    TSharedRef<class SGraphEditor> CreateGraphEditorWidget(UEdGraph* InGraph) const;

private:
    USnapConnectionInfo* AssetBeingEdited = nullptr;

    /** Preview Viewport widget */
    TSharedPtr<class SSnapConnectionPreview3DViewport> PreviewViewport;

    /** Properties widget */
    TSharedPtr<IDetailsView> DetailsPanel;
    TSharedPtr<class SThemeEditorDropTarget> AssetDropTarget;
    TSharedPtr<class SGraphEditor> GraphEditor;
    FDelegateHandle OnGraphChangedDelegateHandle;
    FSnapConnectionThemeGraphHandlerPtr ThemeGraphHandler;
    TArray<TWeakObjectPtr<AActor>> ConnectionInstanceActors;

    FString PreviewMarkerName;
    bool bRequestPreviewRebuild = false;
};

