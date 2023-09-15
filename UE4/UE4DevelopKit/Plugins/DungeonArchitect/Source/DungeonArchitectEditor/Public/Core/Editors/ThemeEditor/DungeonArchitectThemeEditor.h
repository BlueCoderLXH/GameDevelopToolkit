//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Editors/ThemeEditor/IDungeonArchitectThemeEditor.h"
#include "Core/Editors/ThemeEditor/Widgets/SGraphNode_DungeonActor.h"
#include "Core/Editors/ThemeEditor/Widgets/SMarkerListView.h"
#include "Frameworks/ThemeEngine/DungeonThemeAsset.h"

#include "CoreGlobals.h"
#include "IDetailsView.h"
#include "Tickable.h"
#include "Toolkits/AssetEditorManager.h"

class SThemeEditorDropTarget;
class UEdGraphNode_DungeonMarker;

class FDungeonEditorThumbnailPool : public FAssetThumbnailPool {
public:
    FDungeonEditorThumbnailPool(int NumObjectsInPool) : FAssetThumbnailPool(NumObjectsInPool) {
    }

    static TSharedPtr<FDungeonEditorThumbnailPool> Get() { return Instance; }

    static void Create() {
        if (!IsRunningCommandlet()) {
            Instance = MakeShareable(new FDungeonEditorThumbnailPool(512));
        }
    }

private:
    static TSharedPtr<FDungeonEditorThumbnailPool> Instance;
};

class SGraphEditor_Dungeon : public SGraphEditor {
public:
    // SWidget implementation
    virtual void Tick(const FGeometry& AllottedGeometry, double InCurrentTime, float InDeltaTime) override;
    // End SWidget implementation
};

typedef TSharedPtr<class FDungeonArchitectThemeGraphHandler> FDungeonArchitectThemeEditorActionsPtr;

/*-----------------------------------------------------------------------------
FDungeonArchitectThemeEditor
-----------------------------------------------------------------------------*/
class FDungeonArchitectThemeEditor : public IDungeonArchitectThemeEditor, public FNotifyHook,
                                     public FTickableGameObject {
public:
    ~FDungeonArchitectThemeEditor();
    // IToolkit interface
    virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
    virtual void UnregisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
    // End of IToolkit interface

    // FAssetEditorToolkit

    virtual FName GetToolkitFName() const override;
    virtual FText GetBaseToolkitName() const override;
    virtual FText GetToolkitName() const override;
    virtual FLinearColor GetWorldCentricTabColorScale() const override;
    virtual FString GetWorldCentricTabPrefix() const override;
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

    void InitDungeonEditor(EToolkitMode::Type Mode, const TSharedPtr<class IToolkitHost>& InitToolkitHost,
                           UDungeonThemeAsset* PropData);
    UDungeonThemeAsset* GetPropBeingEdited() const { return PropBeingEdited; }

    FORCEINLINE TSharedPtr<SGraphEditor> GetGraphEditor() const { return GraphEditor; }

    void ShowObjectDetails(UObject* ObjectProperties, bool bForceRefresh = false);

    void RecreateDefaultMarkerNodes();
    void HandleOpenHelpSystem();
    bool GetBoundsForSelectedNodes(class FSlateRect& Rect, float Padding) const;

protected:
    void ExtendMenu();
    void ExtendToolbar();
    TSharedRef<class SGraphEditor> CreateGraphEditorWidget(UEdGraph* InGraph);
    TSharedRef<class IDetailsView> CreatePropertyEditorWidget();


    void OnGraphChanged(const FEdGraphEditAction& Action);
    void OnNodePropertyChanged(const FEdGraphEditAction& Action);
    void HandleGraphChanged();
    void InitThemeGraph(class UEdGraph_DungeonProp* ThemeGraph);
    void OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo, UEdGraphNode* NodeBeingChanged);

protected:
    /** Called when "Save" is clicked for this asset */
    virtual void SaveAsset_Execute() override;
    void UpdateOriginalPropAsset();

    void UpdateThumbnail();

    void HandleAssetDropped(UObject* AssetObject);
    bool IsAssetAcceptableForDrop(const UObject* AssetObject) const;
    FVector2D GetAssetDropGridLocation() const;
    void RefreshMarkerListView();
    void OnMarkerListDoubleClicked(TSharedPtr<FMarkerListEntry> Entry);

protected:
    TSharedPtr<SGraphEditor> GraphEditor;
    TSharedPtr<IDetailsView> PropertyEditor;
    TSharedPtr<SThemeEditorDropTarget> AssetDropTarget;
    UDungeonThemeAsset* PropBeingEdited;
    TSharedRef<SDockTab> SpawnTab_GraphEditor(const FSpawnTabArgs& Args);
    TSharedRef<SDockTab> SpawnTab_Preview(const FSpawnTabArgs& Args);
    TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);
    TSharedRef<SDockTab> SpawnTab_Actions(const FSpawnTabArgs& Args);
    TSharedRef<SDockTab> SpawnTab_Markers(const FSpawnTabArgs& Args);
    TSharedRef<SDockTab> SpawnTab_PreviewSettings(const FSpawnTabArgs& Args);
    TSharedRef<SDockTab> SpawnTab_ContentBrowser(const FSpawnTabArgs& Args);


    /** Palette of Node actions to perform on the graph */
    TSharedPtr<class SGraphPalette_PropActions> ActionPalette;

    /** Preview Viewport widget */
    TSharedPtr<class SDungeonEditorViewport> PreviewViewport;

    TSharedPtr<class SMarkerListView> MarkerListView;

    /** Handle to the registered OnGraphChanged delegate. */
    FDelegateHandle OnGraphChangedDelegateHandle;

    /** Handle to the registered OnNodePropertyChanged delegate. */
    FDelegateHandle OnNodePropertyChangedDelegateHandle;

    FDungeonArchitectThemeEditorActionsPtr ThemeGraphHandler;

    bool bGraphStateChanged;

private:
    void BindCommands();
};

class FDungeonArchitectThemeEditorUtils {
public:
    static bool GetBoundsForSelectedNodes(const UEdGraph* Graph, class FSlateRect& Rect, float Padding = 0.0f);

private:
    static TSharedPtr<FDungeonArchitectThemeEditor> GetThemeEditorForAsset(const UEdGraph* Graph);
};

