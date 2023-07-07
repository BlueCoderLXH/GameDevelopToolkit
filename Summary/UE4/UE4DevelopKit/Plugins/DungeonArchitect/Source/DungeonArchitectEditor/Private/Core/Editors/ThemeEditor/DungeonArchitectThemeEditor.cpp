//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/ThemeEditor/DungeonArchitectThemeEditor.h"

#include "Core/Common/DungeonArchitectCommands.h"
#include "Core/DungeonBuilder.h"
#include "Core/Editors/ThemeEditor/DungeonThemeGraphHandler.h"
#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonActorTemplate.h"
#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonBase.h"
#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonMarker.h"
#include "Core/Editors/ThemeEditor/Graph/EdGraph_DungeonProp.h"
#include "Core/Editors/ThemeEditor/Widgets/SDungeonEditorViewport.h"
#include "Core/Editors/ThemeEditor/Widgets/SGraphPalette_PropActions.h"
#include "Core/Editors/ThemeEditor/Widgets/SThemeEditorDropTarget.h"

#include "AssetRegistry/AssetData.h"
#include "AssetSelection.h"
#include "ContentBrowserModule.h"
#include "EdGraph/EdGraph.h"
#include "EdGraphUtilities.h"
#include "Editor/WorkspaceMenuStructure/Public/WorkspaceMenuStructureModule.h"
#include "EditorSupportDelegates.h"
#include "FileHelpers.h"
#include "GraphEditAction.h"
#include "GraphEditor.h"
#include "GraphEditorActions.h"
#include "HAL/PlatformApplicationMisc.h"
#include "IContentBrowserSingleton.h"
#include "IDetailsView.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "PropertyEditorModule.h"
#include "SAdvancedPreviewDetailsTab.h"
#include "SNodePanel.h"
#include "SSingleObjectDetailsPanel.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Toolkits/IToolkitHost.h"
#include "Toolkits/ToolkitManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FDungeonArchitectThemeEditor"

DEFINE_LOG_CATEGORY_STATIC(DungeonEditor, Log, All);

const FName DungeonEditorAppName = FName(TEXT("DungeonEditorApp"));
TSharedPtr<FDungeonEditorThumbnailPool> FDungeonEditorThumbnailPool::Instance;

struct FDungeonThemeEditorTabs {
    // Tab identifiers
    static const FName DetailsID;
    static const FName ActionsID;
    static const FName GraphEditorID;
    static const FName PreviewID;
    static const FName PreviewSettingsID;
    static const FName MarkersID;
    static const FName ContentBrowserID;
};


//////////////////////////////////////////////////////////////////////////

const FName FDungeonThemeEditorTabs::DetailsID(TEXT("Details"));
const FName FDungeonThemeEditorTabs::GraphEditorID(TEXT("GraphEditor"));
const FName FDungeonThemeEditorTabs::PreviewID(TEXT("Preview"));
const FName FDungeonThemeEditorTabs::PreviewSettingsID(TEXT("PreviewSettings"));
const FName FDungeonThemeEditorTabs::ActionsID(TEXT("Actions"));
const FName FDungeonThemeEditorTabs::MarkersID(TEXT("Markers"));
const FName FDungeonThemeEditorTabs::ContentBrowserID(TEXT("ContentBrowser"));


FName FDungeonArchitectThemeEditor::GetToolkitFName() const {
    return FName("DungeonEditor");
}

FText FDungeonArchitectThemeEditor::GetBaseToolkitName() const {
    return LOCTEXT("DungeonEditorAppLabel", "Dungeon Editor");
}

FText FDungeonArchitectThemeEditor::GetToolkitName() const {
    const bool bDirtyState = PropBeingEdited->GetOutermost()->IsDirty();

    FFormatNamedArguments Args;
    Args.Add(TEXT("DungeonName"), FText::FromString(PropBeingEdited->GetName()));
    Args.Add(TEXT("DirtyState"), bDirtyState ? FText::FromString(TEXT("*")) : FText::GetEmpty());
    return FText::Format(LOCTEXT("DungeonEditorAppLabel", "{DungeonName}{DirtyState}"), Args);
}

FString FDungeonArchitectThemeEditor::GetWorldCentricTabPrefix() const {
    return TEXT("DungeonEditor");
}

FString FDungeonArchitectThemeEditor::GetDocumentationLink() const {
    return TEXT("Dungeon/DungeonEditor");
}

void FDungeonArchitectThemeEditor::NotifyPreChange(FProperty* PropertyAboutToChange) {

}

void FDungeonArchitectThemeEditor::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent,
                                                    FProperty* PropertyThatChanged) {

}

FLinearColor FDungeonArchitectThemeEditor::GetWorldCentricTabColorScale() const {
    return FLinearColor::White;
}

//////////////////////////////////////////////////////////////////////////
// FDungeonArchitectThemeEditor

TSharedRef<SDockTab> FDungeonArchitectThemeEditor::SpawnTab_Details(const FSpawnTabArgs& Args) {
    // Spawn the tab
    return SNew(SDockTab)
        .Label(LOCTEXT("DetailsTab_Title", "Details"))
        [
            PropertyEditor.ToSharedRef()
        ];
}

TSharedRef<SDockTab> FDungeonArchitectThemeEditor::SpawnTab_Actions(const FSpawnTabArgs& Args) {
    ActionPalette = SNew(SGraphPalette_PropActions, SharedThis(this));

    TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("Kismet.Tabs.Palette"))
		.Label(LOCTEXT("ActionsPaletteTitle", "Actions"))
    [
        SNew(SBox)
        .AddMetaData<FTagMetaData>(FTagMetaData(TEXT("ActionsPalette")))
        [
            ActionPalette.ToSharedRef()
        ]
    ];

    return SpawnedTab;
}

template <typename T>
void GetGraphNodes(UEdGraph* Graph, TArray<T*>& OutResult) {
    OutResult.Reset();
    for (UEdGraphNode* Node : Graph->Nodes) {
        if (Node && Node->IsA<T>()) {
            OutResult.Add(Cast<T>(Node));
        }
    }
}

void GetMarkerNodes(UDungeonThemeAsset* ThemeAsset, TArray<TSharedPtr<FMarkerListEntry>>& OutResult) {
    OutResult.Reset();
    if (ThemeAsset && ThemeAsset->UpdateGraph) {
        TArray<UEdGraphNode_DungeonMarker*> MarkerNodes;
        GetGraphNodes<UEdGraphNode_DungeonMarker>(ThemeAsset->UpdateGraph, MarkerNodes);
        for (UEdGraphNode_DungeonMarker* MarkerNode : MarkerNodes) {
            TSharedPtr<FMarkerListEntry> Item = MakeShareable(new FMarkerListEntry);
            Item->MarkerName = MarkerNode->MarkerName;
            Item->MarkerNode = MarkerNode;
            OutResult.Add(Item);
        }
    }
}

void FDungeonArchitectThemeEditor::RefreshMarkerListView() {
    if (MarkerListView.IsValid()) {
        TArray<TSharedPtr<FMarkerListEntry>> MarkerEntries;
        GetMarkerNodes(PropBeingEdited, MarkerEntries);
        MarkerListView->Refresh(MarkerEntries);
    }
}


void FDungeonArchitectThemeEditor::OnMarkerListDoubleClicked(TSharedPtr<FMarkerListEntry> Entry) {
    if (Entry.IsValid()) {
        UEdGraphNode* Node = Entry->MarkerNode.Get();
        if (Node) {
            GraphEditor->JumpToNode(Node);
        }
    }
}

TSharedRef<SDockTab> FDungeonArchitectThemeEditor::SpawnTab_Markers(const FSpawnTabArgs& Args) {
    MarkerListView = SNew(SMarkerListView)
        .OnMarkerDoubleClicked(this, &FDungeonArchitectThemeEditor::OnMarkerListDoubleClicked);
    RefreshMarkerListView();

    const FSlateBrush* InfoIcon = FEditorStyle::GetBrush("Icons.Info");
    FText InfoText = FText::FromString("Double click to jump to the node");
    TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("Kismet.Tabs.Palette"))
		.Label(LOCTEXT("MarkersTitle", "Markers"))
    [
        MarkerListView.ToSharedRef()
    ];

    return SpawnedTab;
}

bool FDungeonArchitectThemeEditor::IsTickableInEditor() const {
    return true;
}

void FDungeonArchitectThemeEditor::Tick(float DeltaTime) {
    if (bGraphStateChanged) {
        bGraphStateChanged = false;
        HandleGraphChanged();
    }
}

bool FDungeonArchitectThemeEditor::IsTickable() const {
    return true;
}

TStatId FDungeonArchitectThemeEditor::GetStatId() const {
    return TStatId();
}

TSharedRef<SDockTab> FDungeonArchitectThemeEditor::SpawnTab_GraphEditor(const FSpawnTabArgs& Args) {
    AssetDropTarget = SNew(SThemeEditorDropTarget)
		.OnAssetDropped(this, &FDungeonArchitectThemeEditor::HandleAssetDropped)
		.OnIsAssetAcceptableForDrop(this, &FDungeonArchitectThemeEditor::IsAssetAcceptableForDrop)
		.Visibility(EVisibility::HitTestInvisible);

    return SNew(SDockTab)
		.Label(LOCTEXT("PropMeshGraph", "Mesh Graph"))
		.TabColorScale(GetTabColorScale())
    [
        SNew(SOverlay)
        + SOverlay::Slot()
        [
            GraphEditor.ToSharedRef()
        ]
        + SOverlay::Slot()
        [
            AssetDropTarget.ToSharedRef()
        ]
    ];
}


void FDungeonArchitectThemeEditor::HandleAssetDropped(UObject* AssetObject) {
    UE_LOG(DungeonEditor, Log, TEXT("Dropped asset"));

    if (GraphEditor.IsValid()) {
        UEdGraph_DungeonProp* ThemeGraph = Cast<UEdGraph_DungeonProp>(GraphEditor->GetCurrentGraph());
        if (ThemeGraph) {
            FVector2D GridLocation = GetAssetDropGridLocation();
            ThemeGraph->CreateNewNode(AssetObject, GridLocation);
        }
    }
}

bool FDungeonArchitectThemeEditor::IsAssetAcceptableForDrop(const UObject* AssetObject) const {
    if (GraphEditor.IsValid()) {
        UEdGraph_DungeonProp* ThemeGraph = Cast<UEdGraph_DungeonProp>(GraphEditor->GetCurrentGraph());
        if (ThemeGraph) {
            bool bCanDrop = ThemeGraph->IsAssetAcceptableForDrop(AssetObject);

            if (!bCanDrop) {
                // Check if a broker can convert this asset to an actor
                FAssetData AssetData(AssetObject);
                bool bHasActorFactory = FActorFactoryAssetProxy::GetFactoryForAsset(AssetData) != nullptr;

                if (bHasActorFactory) {
                    bCanDrop = true;
                }
            }

            return bCanDrop;
        }
    }
    return false;
}

FVector2D FDungeonArchitectThemeEditor::GetAssetDropGridLocation() const {
    if (!AssetDropTarget.IsValid()) return FVector2D::ZeroVector;

    FVector2D PanelCoord = AssetDropTarget->GetPanelCoordDropPosition();
    FVector2D ViewLocation = FVector2D::ZeroVector;
    float ZoomAmount = 1.0f;
    GraphEditor->GetViewLocation(ViewLocation, ZoomAmount);
    FVector2D GridLocation = PanelCoord / ZoomAmount + ViewLocation;

    return GridLocation;
}

TSharedRef<SDockTab> FDungeonArchitectThemeEditor::SpawnTab_Preview(const FSpawnTabArgs& Args) {
    TSharedRef<SDockTab> SpawnedTab =
        SNew(SDockTab)
			.Label(LOCTEXT("PropPreview", "Preview"))
			.TabColorScale(GetTabColorScale())
        [
            PreviewViewport.ToSharedRef()
        ];

    PreviewViewport->SetParentTab(SpawnedTab);
    return SpawnedTab;
}


TSharedRef<SDockTab> FDungeonArchitectThemeEditor::SpawnTab_PreviewSettings(const FSpawnTabArgs& Args) {
    TSharedPtr<FAdvancedPreviewScene> PreviewScene;

    if (PreviewViewport.IsValid()) {
        PreviewScene = PreviewViewport->GetAdvancedPreview();
    }


    TSharedPtr<SWidget> SettingsWidget = SNullWidget::NullWidget;
    if (PreviewScene.IsValid()) {
        TSharedPtr<SAdvancedPreviewDetailsTab> PreviewSettingsWidget = SNew(
            SAdvancedPreviewDetailsTab, PreviewScene.ToSharedRef());

        PreviewSettingsWidget->Refresh();

        SettingsWidget = PreviewSettingsWidget;
    }

    TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
    .Icon(FEditorStyle::GetBrush("Kismet.Tabs.Palette"))
    .Label(LOCTEXT("PreviewSettingsTitle", "Preview Settings"))
    [
        SNew(SBox)
        .AddMetaData<FTagMetaData>(FTagMetaData(TEXT("Preview Settings")))
        [
            SettingsWidget.ToSharedRef()
        ]
    ];

    return SpawnedTab;
}

TSharedRef<SDockTab> FDungeonArchitectThemeEditor::SpawnTab_ContentBrowser(const FSpawnTabArgs& Args) {
    TSharedRef<SDockTab> SpawnedTab =
        SNew(SDockTab)
		.Label(LOCTEXT("ContentBrowserKey", "Content Browser"))
		.TabColorScale(GetTabColorScale())
        [
            SNullWidget::NullWidget
        ];

    IContentBrowserSingleton& ContentBrowserSingleton = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser").Get();

    FName ContentBrowserID = *("DA_ThemeEdtior_ContentBrowser_" + FGuid::NewGuid().ToString());
    FContentBrowserConfig ContentBrowserConfig;
    TSharedRef<SWidget, ESPMode::NotThreadSafe> ContentBrowser = ContentBrowserSingleton.CreateContentBrowser(
        ContentBrowserID, SpawnedTab, &ContentBrowserConfig);
    SpawnedTab->SetContent(ContentBrowser);

    return SpawnedTab;
}

void FDungeonArchitectThemeEditor::BindCommands() {
    const FDungeonArchitectCommands& Commands = FDungeonArchitectCommands::Get();

    ToolkitCommands->MapAction(
        Commands.OpenHelpSystem,
        FExecuteAction::CreateSP(this, &FDungeonArchitectThemeEditor::HandleOpenHelpSystem));

}

TSharedRef<SGraphEditor> FDungeonArchitectThemeEditor::CreateGraphEditorWidget(UEdGraph* InGraph) {
    // Create the appearance info
    FGraphAppearanceInfo AppearanceInfo;
    AppearanceInfo.CornerText = LOCTEXT("AppearanceCornerText", "Dungeon Architect");


    // Make title bar
    TSharedRef<SWidget> TitleBarWidget =
        SNew(SBorder)
		.BorderImage(FEditorStyle::GetBrush(TEXT("Graph.TitleBackground")))
		.HAlign(HAlign_Fill)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
              .HAlign(HAlign_Center)
              .FillWidth(1.f)
            [
                SNew(STextBlock)
				.Text(LOCTEXT("UpdateGraphLabel", "Mesh Graph"))
				.TextStyle(FEditorStyle::Get(), TEXT("GraphBreadcrumbButtonText"))
            ]
        ];

    TSharedRef<SGraphEditor> _GraphEditor = SNew(SGraphEditor_Dungeon)
		.AdditionalCommands(ThemeGraphHandler->GetCommands())
		.Appearance(AppearanceInfo)
		//.TitleBar(TitleBarWidget)
		.GraphToEdit(InGraph)
		.GraphEvents(ThemeGraphHandler->GraphEvents);


    return _GraphEditor;
}

TSharedRef<class IDetailsView> FDungeonArchitectThemeEditor::CreatePropertyEditorWidget() {
    FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(
        "PropertyEditor");
    const FDetailsViewArgs DetailsViewArgs(false, false, true, FDetailsViewArgs::HideNameArea, true, this);
    TSharedRef<IDetailsView> PropertyEditorRef = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
    return PropertyEditorRef;
}

template <typename T>
bool ContainsNodesOfType(TSet<const UEdGraphNode*> Nodes) {
    for (const UEdGraphNode* Node : Nodes) {
        if (Cast<const T>(Node)) {
            return true;
        }
    }
    return false;
}

void FDungeonArchitectThemeEditor::OnGraphChanged(const FEdGraphEditAction& Action) {
    bGraphStateChanged = true;

    if (ContainsNodesOfType<UEdGraphNode_DungeonMarker>(Action.Nodes)) {
        RefreshMarkerListView();
    }
}

void FDungeonArchitectThemeEditor::OnNodePropertyChanged(const FEdGraphEditAction& Action) {
    // The nodes that were modified and requires a clean rebuild by the scene provider
    TSet<FName> NodeObjectsToRebuild;

    // Flag the node id to generate it cleanly in the scene provider
    for (const UEdGraphNode* Node : Action.Nodes) {
        const UEdGraphNode_DungeonActorBase* ActorNode = Cast<const UEdGraphNode_DungeonActorBase>(Node);
        if (ActorNode) {
            FName NodeId(*ActorNode->NodeGuid.ToString());
            NodeObjectsToRebuild.Add(NodeId);
        }
    }

    PreviewViewport->SetNodesToRebuild(NodeObjectsToRebuild);

    if (ContainsNodesOfType<UEdGraphNode_DungeonMarker>(Action.Nodes)) {
        RefreshMarkerListView();
    }

    bGraphStateChanged = true;
}

void FDungeonArchitectThemeEditor::HandleGraphChanged() {
    UpdateOriginalPropAsset();
    if (PreviewViewport.IsValid()) {
        PreviewViewport->RebuildMeshes();
    }
    if (ActionPalette.IsValid()) {
        ActionPalette->Refresh();
    }
}

void FDungeonArchitectThemeEditor::SaveAsset_Execute() {
    UE_LOG(DungeonEditor, Log, TEXT("Saving dungeon theme %s"), *GetEditingObjects()[0]->GetName());
    UpdateOriginalPropAsset();
    UpdateThumbnail();

    UPackage* Package = PropBeingEdited->GetOutermost();
    if (Package) {
        TArray<UPackage*> PackagesToSave;
        PackagesToSave.Add(Package);

        FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, false, false);
    }

    PreviewViewport->RebuildMeshes();
}

void FDungeonArchitectThemeEditor::UpdateOriginalPropAsset() {
    if (UEdGraph_DungeonProp* DungeonGraphEditor = Cast<UEdGraph_DungeonProp>(PropBeingEdited->UpdateGraph)) {
        TArray<FPropTypeData> Props;
        TArray<FDungeonGraphBuildError> CompileErrors;
        DungeonGraphEditor->RebuildGraph(PropBeingEdited, Props, CompileErrors);
        if (CompileErrors.Num() == 0) {
            PropBeingEdited->Props = Props;
        }
        else {
            // TODO: Display the error messages on the graph nodes
        }
    }
    FEditorDelegates::RefreshEditor.Broadcast();
    FEditorSupportDelegates::RedrawAllViewports.Broadcast();
}

void FDungeonArchitectThemeEditor::UpdateThumbnail() {
    if (!PropBeingEdited) return;
    FViewport* PreviewViewportPtr = nullptr;
    if (PreviewViewport.IsValid() && PreviewViewport->GetViewportClient().IsValid()) {
        PreviewViewportPtr = PreviewViewport->GetViewportClient()->Viewport;
    }

    if (PreviewViewportPtr) {
        FAssetData AssetData(PropBeingEdited);
        TArray<FAssetData> ThemeAssetList;
        ThemeAssetList.Add(AssetData);

        IContentBrowserSingleton& ContentBrowserSingleton = FModuleManager::LoadModuleChecked<FContentBrowserModule
        >("ContentBrowser").Get();
        ContentBrowserSingleton.CaptureThumbnailFromViewport(PreviewViewportPtr, ThemeAssetList);
    }
}

FDungeonArchitectThemeEditor::~FDungeonArchitectThemeEditor() {
    if (GraphEditor->GetCurrentGraph()) {
        GraphEditor->GetCurrentGraph()->RemoveOnGraphChangedHandler(OnGraphChangedDelegateHandle);
        UEdGraph_DungeonProp* ThemeGraph = Cast<UEdGraph_DungeonProp>(GraphEditor->GetCurrentGraph());
        if (ThemeGraph) {
            ThemeGraph->RemoveOnNodePropertyChangedHandler(OnNodePropertyChangedDelegateHandle);
        }
    }
}

void FDungeonArchitectThemeEditor::RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager) {
    WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(
        LOCTEXT("WorkspaceMenu_DungeonEditor", "Dungeon Editor"));
    auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

    FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

    InTabManager->RegisterTabSpawner(FDungeonThemeEditorTabs::DetailsID,
                                     FOnSpawnTab::CreateSP(this, &FDungeonArchitectThemeEditor::SpawnTab_Details))
                .SetDisplayName(LOCTEXT("DetailsTabLabel", "Details"))
                .SetGroup(WorkspaceMenuCategoryRef)
                .SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

    InTabManager->RegisterTabSpawner(FDungeonThemeEditorTabs::GraphEditorID,
                                     FOnSpawnTab::CreateSP(this, &FDungeonArchitectThemeEditor::SpawnTab_GraphEditor))
                .SetDisplayName(LOCTEXT("ViewportTab", "Viewport"))
                .SetGroup(WorkspaceMenuCategoryRef)
                .SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports"));

    InTabManager->RegisterTabSpawner(FDungeonThemeEditorTabs::PreviewID,
                                     FOnSpawnTab::CreateSP(this, &FDungeonArchitectThemeEditor::SpawnTab_Preview))
                .SetDisplayName(LOCTEXT("PreviewTab", "Preview"))
                .SetGroup(WorkspaceMenuCategoryRef)
                .SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports"));

    InTabManager->RegisterTabSpawner(FDungeonThemeEditorTabs::PreviewSettingsID,
                                     FOnSpawnTab::CreateSP(
                                         this, &FDungeonArchitectThemeEditor::SpawnTab_PreviewSettings))
                .SetDisplayName(LOCTEXT("PreviewSettingsTabLabel", "Preview Settings"))
                .SetGroup(WorkspaceMenuCategoryRef)
                .SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

    InTabManager->RegisterTabSpawner(FDungeonThemeEditorTabs::ActionsID,
                                     FOnSpawnTab::CreateSP(this, &FDungeonArchitectThemeEditor::SpawnTab_Actions))
                .SetDisplayName(LOCTEXT("ActionsTabLabel", "Actions"))
                .SetGroup(WorkspaceMenuCategoryRef)
                .SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

    InTabManager->RegisterTabSpawner(FDungeonThemeEditorTabs::MarkersID,
                                     FOnSpawnTab::CreateSP(this, &FDungeonArchitectThemeEditor::SpawnTab_Markers))
                .SetDisplayName(LOCTEXT("MarkersTabLabel", "Markers"))
                .SetGroup(WorkspaceMenuCategoryRef)
                .SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

    InTabManager->RegisterTabSpawner(FDungeonThemeEditorTabs::ContentBrowserID,
                                     FOnSpawnTab::CreateSP(
                                         this, &FDungeonArchitectThemeEditor::SpawnTab_ContentBrowser))
                .SetDisplayName(LOCTEXT("ContentBrowserLabel", "Content Browser"))
                .SetGroup(WorkspaceMenuCategoryRef)
                .SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.ContentBrowser"));
}

void FDungeonArchitectThemeEditor::UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager) {
    FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

    InTabManager->UnregisterTabSpawner(FDungeonThemeEditorTabs::GraphEditorID);
    InTabManager->UnregisterTabSpawner(FDungeonThemeEditorTabs::PreviewID);
    InTabManager->UnregisterTabSpawner(FDungeonThemeEditorTabs::PreviewSettingsID);
    InTabManager->UnregisterTabSpawner(FDungeonThemeEditorTabs::ActionsID);
    InTabManager->UnregisterTabSpawner(FDungeonThemeEditorTabs::MarkersID);
    InTabManager->UnregisterTabSpawner(FDungeonThemeEditorTabs::DetailsID);
    InTabManager->UnregisterTabSpawner(FDungeonThemeEditorTabs::ContentBrowserID);
}

void FDungeonArchitectThemeEditor::ExtendMenu() {

}

void FDungeonArchitectThemeEditor::ExtendToolbar() {
    struct Local {
        static void FillToolbar(FToolBarBuilder& ToolbarBuilder) {
            ToolbarBuilder.BeginSection("Support");
            {
                ToolbarBuilder.AddToolBarButton(FDungeonArchitectCommands::Get().OpenHelpSystem);
            }
            ToolbarBuilder.EndSection();
        }
    };

    TSharedPtr<FExtender> ToolbarExtender(new FExtender);
    ToolbarExtender->AddToolBarExtension(
        "Asset",
        EExtensionHook::After,
        GetToolkitCommands(),
        FToolBarExtensionDelegate::CreateStatic(&Local::FillToolbar)
    );
    AddToolbarExtender(ToolbarExtender);
}

void FDungeonArchitectThemeEditor::InitDungeonEditor(const EToolkitMode::Type Mode,
                                                     const TSharedPtr<class IToolkitHost>& InitToolkitHost,
                                                     UDungeonThemeAsset* PropData) {
    // Initialize the asset editor and spawn nothing (dummy layout)
    GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseOtherEditors(PropData, this);

    PropBeingEdited = PropData;
    if (!PropBeingEdited->UpdateGraph) {
        UEdGraph_DungeonProp* DungeonGraph = NewObject<UEdGraph_DungeonProp>(
            PropBeingEdited, UEdGraph_DungeonProp::StaticClass(), NAME_None, RF_Transactional);
        DungeonGraph->RecreateDefaultMarkerNodes(PropBeingEdited->PreviewViewportProperties->BuilderClass);
        PropBeingEdited->UpdateGraph = DungeonGraph;
    }

    ThemeGraphHandler = MakeShareable(new FDungeonArchitectThemeGraphHandler);
    ThemeGraphHandler->Bind();
    ThemeGraphHandler->GetGraphEvents().OnTextCommitted = FOnNodeTextCommitted::CreateSP(this, &FDungeonArchitectThemeEditor::OnNodeTitleCommitted);

    GraphEditor = CreateGraphEditorWidget(PropBeingEdited->UpdateGraph);
    PropertyEditor = CreatePropertyEditorWidget();
    ThemeGraphHandler->Initialize(GraphEditor, PropertyEditor, PropBeingEdited->UpdateGraph, PropBeingEdited->PreviewViewportProperties);

    PreviewViewport = SNew(SDungeonEditorViewport)
		.DungeonEditor(SharedThis(this))
		.ObjectToEdit(PropBeingEdited);

    BindCommands();
    ExtendMenu();
    ExtendToolbar();


    // Default layout
    const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout(
            "Standalone_DungeonPropEditor_Layout_v2.3.0")
        ->AddArea
        (
            FTabManager::NewPrimaryArea()
            ->SetOrientation(Orient_Vertical)
            ->Split
            (
                FTabManager::NewStack()
                ->SetSizeCoefficient(0.1f)
                ->SetHideTabWell(true)
                ->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
            )
            ->Split
            (
                FTabManager::NewSplitter()
                ->SetOrientation(Orient_Horizontal)
                ->SetSizeCoefficient(0.63f)
                ->Split
                (

                    FTabManager::NewSplitter()
                    ->SetOrientation(Orient_Vertical)
                    ->SetSizeCoefficient(0.66f)
                    ->Split // Graph Widget
                    (
                        FTabManager::NewStack()
                        ->SetSizeCoefficient(0.66f)
                        ->SetHideTabWell(true)
                        ->AddTab(FDungeonThemeEditorTabs::GraphEditorID, ETabState::OpenedTab)
                    )
                    ->Split // ContentBrowser
                    (
                        FTabManager::NewStack()
                        ->SetSizeCoefficient(0.33f)
                        ->AddTab(FDungeonThemeEditorTabs::ContentBrowserID, ETabState::OpenedTab)
                    )
                )
                ->Split
                (
                    FTabManager::NewSplitter()
                    ->SetOrientation(Orient_Vertical)
                    ->SetSizeCoefficient(0.37f)
                    ->Split
                    (
                        //FTabManager::NewStack()
                        FTabManager::NewSplitter()
                        ->SetOrientation(Orient_Horizontal)
                        ->SetSizeCoefficient(0.33f)
                        ->Split
                        (
                            FTabManager::NewStack()
                            ->SetSizeCoefficient(0.6f)
                            ->AddTab(FDungeonThemeEditorTabs::DetailsID, ETabState::OpenedTab)
                        )
                        ->Split
                        (
                            FTabManager::NewStack()
                            ->SetSizeCoefficient(0.6f)
                            ->AddTab(FDungeonThemeEditorTabs::PreviewSettingsID, ETabState::OpenedTab)
                            ->AddTab(FDungeonThemeEditorTabs::ActionsID, ETabState::OpenedTab)
                            ->AddTab(FDungeonThemeEditorTabs::MarkersID, ETabState::OpenedTab)
                            ->SetForegroundTab(FDungeonThemeEditorTabs::MarkersID)
                        )
                    )
                    ->Split
                    (
                        FTabManager::NewStack()
                        ->SetSizeCoefficient(0.667f)
                        ->AddTab(FDungeonThemeEditorTabs::PreviewID, ETabState::OpenedTab)
                    )
                )
            )
        );

    // Initialize the asset editor and spawn nothing (dummy layout)
    InitAssetEditor(Mode, InitToolkitHost, DungeonEditorAppName, StandaloneDefaultLayout,
                    /*bCreateDefaultStandaloneMenu=*/ true, /*bCreateDefaultToolbar=*/ true, PropData);

    // Listen for graph changed event
    OnGraphChangedDelegateHandle = GraphEditor->GetCurrentGraph()->AddOnGraphChangedHandler(
        FOnGraphChanged::FDelegate::CreateRaw(
            this, &FDungeonArchitectThemeEditor::OnGraphChanged));
    bGraphStateChanged = true;

    UEdGraph_DungeonProp* ThemeGraph = Cast<UEdGraph_DungeonProp>(GraphEditor->GetCurrentGraph());
    if (ThemeGraph) {
        OnNodePropertyChangedDelegateHandle = ThemeGraph->AddOnNodePropertyChangedHandler(
            FOnGraphChanged::FDelegate::CreateRaw(this, &FDungeonArchitectThemeEditor::OnNodePropertyChanged));
    }

    InitThemeGraph(ThemeGraph);

    // Show the dungeon properties
    if (PropertyEditor.IsValid()) {
        PropertyEditor->SetObject(PropBeingEdited ? PropBeingEdited->PreviewViewportProperties : nullptr, false);
    }
}

void FDungeonArchitectThemeEditor::InitThemeGraph(UEdGraph_DungeonProp* ThemeGraph) {
    if (!ThemeGraph) return;

    for (UEdGraphNode* Node : ThemeGraph->Nodes) {
        UEdGraphNode_DungeonBase* DungeonNode = Cast<UEdGraphNode_DungeonBase>(Node);
        if (DungeonNode) {
            // Make sure the actor node is initialized
            DungeonNode->OnThemeEditorLoaded();
        }
    }
}

void FDungeonArchitectThemeEditor::OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo, UEdGraphNode* NodeBeingChanged) {
    if (NodeBeingChanged)
    {
        const FScopedTransaction Transaction( LOCTEXT( "RenameNode", "Rename Node" ) );
        NodeBeingChanged->Modify();
        NodeBeingChanged->OnRenameNode(NewText.ToString());
    }
}

void FDungeonArchitectThemeEditor::ShowObjectDetails(UObject* ObjectProperties, bool bForceRefresh) {
    if (!ObjectProperties) {
        // No object selected.  Show the dungeon properties by default
        ObjectProperties = PropBeingEdited ? PropBeingEdited->PreviewViewportProperties : nullptr;
    }

    if (PropertyEditor.IsValid()) {
        PropertyEditor->SetObject(ObjectProperties, bForceRefresh);
    }
}

void FDungeonArchitectThemeEditor::RecreateDefaultMarkerNodes() {
    if (PropBeingEdited && PropBeingEdited->UpdateGraph && PropBeingEdited->PreviewViewportProperties) {
        if (UEdGraph_DungeonProp* DungeonGraph = Cast<UEdGraph_DungeonProp>(PropBeingEdited->UpdateGraph)) {
            TSubclassOf<UDungeonBuilder> BuilderClass = PropBeingEdited->PreviewViewportProperties->BuilderClass;
            if (BuilderClass) {
                DungeonGraph->RecreateDefaultMarkerNodes(BuilderClass);
            }
        }
    }
}

void FDungeonArchitectThemeEditor::HandleOpenHelpSystem() {
    static const FName HelpSystemApp = FName(TEXT("DAHelpSystemApp"));
    FGlobalTabmanager::Get()->TryInvokeTab(HelpSystemApp);
}

bool FDungeonArchitectThemeEditor::GetBoundsForSelectedNodes(FSlateRect& Rect, float Padding) const {
    if (!GraphEditor.IsValid()) {
        return false;   
    }

    return GraphEditor->GetBoundsForSelectedNodes(Rect, Padding);
}

void SGraphEditor_Dungeon::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime,
                                const float InDeltaTime) {
    SGraphEditor::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

    //FDungeonEditorThumbnailPool::Get()->Tick(InDeltaTime);
}

//////////////////////////////////////// FDungeonArchitectThemeEditorUtils ////////////////////////////////////////  
bool FDungeonArchitectThemeEditorUtils::GetBoundsForSelectedNodes(const UEdGraph* Graph, FSlateRect& Rect, float Padding) {
    TSharedPtr<FDungeonArchitectThemeEditor> ThemeEditor = GetThemeEditorForAsset(Graph);
    if (ThemeEditor.IsValid()) {
        return ThemeEditor->GetBoundsForSelectedNodes(Rect, Padding);
    }
    return false;
}

TSharedPtr<FDungeonArchitectThemeEditor> FDungeonArchitectThemeEditorUtils::GetThemeEditorForAsset(const UEdGraph* Graph) {
    if (!Graph) {
        return nullptr;
    }
    
    UDungeonThemeAsset* ThemeAsset = Cast<UDungeonThemeAsset>(Graph->GetOuter());
    TSharedPtr<FDungeonArchitectThemeEditor> ThemeEditor;
    if (ThemeAsset) {
        TSharedPtr< IToolkit > FoundAssetEditor = FToolkitManager::Get().FindEditorForAsset(ThemeAsset);
        if (FoundAssetEditor.IsValid()) {
            ThemeEditor = StaticCastSharedPtr<FDungeonArchitectThemeEditor>(FoundAssetEditor);
        }
    }
    return ThemeEditor;
}

#undef LOCTEXT_NAMESPACE

