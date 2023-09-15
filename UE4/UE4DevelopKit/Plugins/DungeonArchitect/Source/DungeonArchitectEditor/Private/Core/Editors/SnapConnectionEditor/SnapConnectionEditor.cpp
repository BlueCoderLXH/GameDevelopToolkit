//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/SnapConnectionEditor/SnapConnectionEditor.h"

#include "Core/Editors/SnapConnectionEditor/Preview3D/SSnapConnectionPreview3DViewport.h"
#include "Core/Editors/SnapConnectionEditor/SnapConnectionEditorCommands.h"
#include "Core/Editors/ThemeEditor/DungeonThemeGraphHandler.h"
#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonActorTemplate.h"
#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonMarker.h"
#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonMesh.h"
#include "Core/Editors/ThemeEditor/Graph/EdGraph_DungeonProp.h"
#include "Core/Editors/ThemeEditor/Widgets/SThemeEditorDropTarget.h"
#include "Frameworks/Snap/Lib/Connection/SnapConnectionActor.h"
#include "Frameworks/Snap/Lib/Connection/SnapConnectionComponent.h"
#include "Frameworks/Snap/Lib/Connection/SnapConnectionInfo.h"

#include "AssetSelection.h"
#include "ContentBrowserModule.h"
#include "Editor.h"
#include "EditorSupportDelegates.h"
#include "FileHelpers.h"
#include "Framework/Commands/GenericCommands.h"
#include "IContentBrowserSingleton.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Logging/TokenizedMessage.h"
#include "SAdvancedPreviewDetailsTab.h"
#include "SNodePanel.h"
#include "SSingleObjectDetailsPanel.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Toolkits/IToolkitHost.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "FSnapConnectionEditor"
DEFINE_LOG_CATEGORY_STATIC(SnapConnectionEditor, Log, All);

class UEdGraph_DungeonProp;
const FName LAThemeEditorAppName = FName(TEXT("LAThemeEditorApp"));

struct FSnapConnectionEditorTabs {
    // Tab identifiers
    static const FName GraphID;
    static const FName Preview3DID;
    static const FName PreviewSettingsID;
    static const FName DetailsID;
    static const FName ContentBrowserID;
};

const FName FSnapConnectionEditorTabs::GraphID(TEXT("GraphID"));
const FName FSnapConnectionEditorTabs::Preview3DID(TEXT("Preview3D"));
const FName FSnapConnectionEditorTabs::PreviewSettingsID(TEXT("PreviewSettings"));
const FName FSnapConnectionEditorTabs::DetailsID(TEXT("Details"));
const FName FSnapConnectionEditorTabs::ContentBrowserID(TEXT("ContentBrowserID"));


//////////////////////////////////////////////////////////////////////////
DECLARE_DELEGATE_OneParam(FSCThemeGraphSelectionEvent, const TSet<class UObject*>&);
class FSnapConnectionThemeGraphHandler : public FDungeonArchitectThemeGraphHandler {
public:
    virtual void OnSelectedNodesChanged(const TSet<class UObject*>& NewSelection) override {
        FDungeonArchitectThemeGraphHandler::OnSelectedNodesChanged(NewSelection);

        OnNodeSelectionChanged.ExecuteIfBound(NewSelection);
    }

    FSCThemeGraphSelectionEvent& GetOnNodeSelectionChanged() { return OnNodeSelectionChanged; }
    
private:
    FSCThemeGraphSelectionEvent OnNodeSelectionChanged;
};



//////////////////////////////////////////////////////////////////////////
///
void FSnapConnectionEditor::InitSnapConnectionEditor(const EToolkitMode::Type Mode,
                                                           const TSharedPtr<class IToolkitHost>& InitToolkitHost,
                                                           USnapConnectionInfo* DoorAsset) {

    // Initialize the asset editor and spawn nothing (dummy layout)
    GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseOtherEditors(DoorAsset, this);

    AssetBeingEdited = DoorAsset;
    
    if (!AssetBeingEdited->ThemeAsset->UpdateGraph) {
        AssetBeingEdited->ThemeAsset->UpdateGraph = CreateNewThemeGraph();
        AssetBeingEdited->Modify();
    }
    
    UpgradeAsset();

    ThemeGraphHandler = MakeShareable(new FSnapConnectionThemeGraphHandler);
    ThemeGraphHandler->GetOnNodeSelectionChanged().BindRaw(this, &FSnapConnectionEditor::OnNodeSelectionChanged);
    ThemeGraphHandler->Bind();

    // Create the details view
    {
        FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
        const FDetailsViewArgs DetailsViewArgs(false, false, true, FDetailsViewArgs::HideNameArea, true, this);
        DetailsPanel = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
        DetailsPanel->SetObject(AssetBeingEdited);
    }

    UEdGraph* ThemeGraph = AssetBeingEdited->ThemeAsset->UpdateGraph;
    GraphEditor = CreateGraphEditorWidget(ThemeGraph);
    ThemeGraphHandler->Initialize(GraphEditor, DetailsPanel, ThemeGraph, AssetBeingEdited);
    
    FSnapConnectionEditorCommands::Register();
    BindCommands();
    ExtendMenu();

    PreviewViewport = SNew(SSnapConnectionPreview3DViewport);


    // Default layout
    const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout(
            "Standalone_SnapConnectionEditor_Layout_v1.0.0")
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
                ->SetSizeCoefficient(1.0f)
                ->Split
                (

                    FTabManager::NewSplitter()
                    ->SetOrientation(Orient_Vertical)
                    ->SetSizeCoefficient(0.7f)
                    ->Split
                    (
                        
                        FTabManager::NewSplitter()
                        ->SetOrientation(Orient_Horizontal)
                        ->SetSizeCoefficient(0.75f)
                        ->Split
                        (
                            FTabManager::NewStack()
                            ->SetSizeCoefficient(0.75f)
                            ->SetHideTabWell(true)
                            ->AddTab(FSnapConnectionEditorTabs::GraphID, ETabState::OpenedTab)
                        )
                        ->Split
                        (
                            FTabManager::NewStack()
                            ->SetSizeCoefficient(0.25f)
                            ->AddTab(FSnapConnectionEditorTabs::DetailsID, ETabState::OpenedTab)
                            ->AddTab(FSnapConnectionEditorTabs::PreviewSettingsID, ETabState::OpenedTab)
                            ->SetForegroundTab(FSnapConnectionEditorTabs::DetailsID)
                        )
                        
                    )
                    ->Split
                    (
                        FTabManager::NewStack()
                        ->SetSizeCoefficient(0.3f)
                        ->AddTab(FSnapConnectionEditorTabs::ContentBrowserID, ETabState::OpenedTab)
                    )

                )

                ->Split
                (
                    FTabManager::NewStack()
                    ->SetSizeCoefficient(0.4f)
                    ->SetHideTabWell(true)
                    ->AddTab(FSnapConnectionEditorTabs::Preview3DID, ETabState::OpenedTab)
                )
            )
        );

    // Initialize the asset editor and spawn nothing (dummy layout)
    InitAssetEditor(Mode, InitToolkitHost, LAThemeEditorAppName, StandaloneDefaultLayout,
                    /*bCreateDefaultStandaloneMenu=*/ true, /*bCreateDefaultToolbar=*/ true, DoorAsset);

    // Listen for graph changed event
    OnGraphChangedDelegateHandle = GraphEditor->GetCurrentGraph()->AddOnGraphChangedHandler(
                FOnGraphChanged::FDelegate::CreateRaw(this, &FSnapConnectionEditor::OnGraphChanged));
    
    PostLoadInitThemeGraph();
    
    SetPreviewMode(FSnapConnectionMarkers::Door);
}

void FSnapConnectionEditor::PostLoadInitThemeGraph() const {
    UEdGraph_DungeonProp* ThemeGraph = Cast<UEdGraph_DungeonProp>(AssetBeingEdited->ThemeAsset->UpdateGraph);
    if (!ThemeGraph) return;

    for (UEdGraphNode* Node : ThemeGraph->Nodes) {
        if (UEdGraphNode_DungeonBase* DungeonNode = Cast<UEdGraphNode_DungeonBase>(Node)) {
            // Make sure the actor node is initialized
            DungeonNode->OnThemeEditorLoaded();
        }
    }
}

void FSnapConnectionEditor::OnGraphChanged(const FEdGraphEditAction& Action) {
    RequestRebuildPreviewObject();
}

void FSnapConnectionEditor::OnNodeSelectionChanged(const TSet<UObject*>& NewSelection) {
    TArray<FString> MarkerNames;
    for (UObject* Object : NewSelection) {
        if (UEdGraphNode_DungeonMarker* MarkerNode = Cast<UEdGraphNode_DungeonMarker>(Object)) {
            MarkerNames.Add(MarkerNode->MarkerName);
        }
    }

    if (MarkerNames.Num() == 1 && MarkerNames[0] != PreviewMarkerName) {
        PreviewMarkerName = MarkerNames[0];
        RequestRebuildPreviewObject();
    }
}

namespace {
    void CreateThemeNodeUnderMarker(const FSnapConnectionVisualInfo_DEPRECATED& VisualInfo, UEdGraphNode_DungeonMarker* MarkerNode) {
        if (!MarkerNode) return;
        UEdGraph_DungeonProp* ThemeGraph = Cast<UEdGraph_DungeonProp>(MarkerNode->GetGraph());
        if (!ThemeGraph) return;
        
        const FVector2D NodePosition = FVector2D(MarkerNode->NodePosX - 40, MarkerNode->NodePosY + 100);
        UEdGraphNode_DungeonActorBase* SpawnedNode = nullptr;
        
        if (VisualInfo.bStaticMesh) {
            UEdGraphNode_DungeonMesh* MeshNode = ThemeGraph->CreateNewNode<UEdGraphNode_DungeonMesh>(NodePosition);
            MeshNode->Mesh = VisualInfo.MeshInfo.StaticMesh;
            if (VisualInfo.MeshInfo.MaterialOverride) {
                FMaterialOverride& OverrideInfo = MeshNode->MaterialOverrides.AddDefaulted_GetRef();
                OverrideInfo.index = 0;
                OverrideInfo.Material = VisualInfo.MeshInfo.MaterialOverride; 
            }
            MeshNode->Offset = VisualInfo.MeshInfo.Offset;
            
            SpawnedNode = MeshNode;
        }
        else {
            UEdGraphNode_DungeonActorTemplate* ActorNode = ThemeGraph->CreateNewNode<UEdGraphNode_DungeonActorTemplate>(NodePosition);
            ActorNode->ClassTemplate = VisualInfo.BlueprintInfo.BlueprintClass;
            ActorNode->Offset = VisualInfo.BlueprintInfo.Offset;
            
            SpawnedNode = ActorNode;
        }

        if (SpawnedNode) {
            const UEdGraphSchema* Schema = ThemeGraph->GetSchema();
            Schema->TryCreateConnection(MarkerNode->GetOutputPin(), SpawnedNode->GetInputPin());
        }
        
    }
}

void FSnapConnectionEditor::UpgradeAsset() const {
    if (AssetBeingEdited->Version == static_cast<int32>(ESnapConnectionInfoVersion::LatestVersion)) {
        // Already in the latest version. No need to upgrade
        return;
    }
    
    if (AssetBeingEdited->Version == static_cast<int32>(ESnapConnectionInfoVersion::InitialVersion)) {
        // Upgrade to Version: ThemeGraphSupport

        // Grab all the marker nodes
        TArray<UEdGraphNode_DungeonMarker*> MarkerNodes;
        for (UEdGraphNode* Node : AssetBeingEdited->ThemeAsset->UpdateGraph->Nodes) {
            if (UEdGraphNode_DungeonMarker* MarkerNode = Cast<UEdGraphNode_DungeonMarker>(Node)) {
                MarkerNodes.Add(MarkerNode);
            }
        }

        // Add visual nodes in the theme editor under the door and the wall markers
        for (UEdGraphNode_DungeonMarker* MarkerNode : MarkerNodes) {
            if (MarkerNode->MarkerName == FSnapConnectionMarkers::Door) {
                // Create a door theme node, if needed
                CreateThemeNodeUnderMarker(AssetBeingEdited->DoorVisuals, MarkerNode);
            }
            else if (MarkerNode->MarkerName == FSnapConnectionMarkers::Wall) {
                // Create a wall theme node, if needed
                CreateThemeNodeUnderMarker(AssetBeingEdited->WallVisuals, MarkerNode);
            }
        }
        
        AssetBeingEdited->Version = static_cast<int32>(ESnapConnectionInfoVersion::ThemeGraphSupport);
    }

    AssetBeingEdited->Version = static_cast<int32>(ESnapConnectionInfoVersion::LatestVersion);
    AssetBeingEdited->Modify();
}

FName FSnapConnectionEditor::GetToolkitFName() const {
    return FName("SnapConnectionEditor");
}

FText FSnapConnectionEditor::GetBaseToolkitName() const {
    return LOCTEXT("SnapConnectionEditorAppLabel", "Snap Connection Editor");
}

FText FSnapConnectionEditor::GetToolkitName() const {
    const bool bDirtyState = AssetBeingEdited->GetOutermost()->IsDirty();
    FFormatNamedArguments Args;
    Args.Add(TEXT("ThemeName"), FText::FromString(AssetBeingEdited->GetName()));
    return FText::Format(LOCTEXT("SnapConnectionEditorAppLabel", "{ThemeName}"), Args);
}

FString FSnapConnectionEditor::GetWorldCentricTabPrefix() const {
    return TEXT("SnapConnectionEditor");
}

FString FSnapConnectionEditor::GetDocumentationLink() const {
    return TEXT("DungeonArchitect/SnapConnectionEditor");
}

void FSnapConnectionEditor::AddReferencedObjects(FReferenceCollector& Collector) {
    Collector.AddReferencedObject(AssetBeingEdited);
}

void FSnapConnectionEditor::NotifyPreChange(FProperty* PropertyAboutToChange) {

}

void FSnapConnectionEditor::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent,
                                                FProperty* PropertyThatChanged) {
    RequestRebuildPreviewObject();
}

FLinearColor FSnapConnectionEditor::GetWorldCentricTabColorScale() const {
    return FLinearColor::White;
}

bool FSnapConnectionEditor::IsTickableInEditor() const {
    return true;
}

void FSnapConnectionEditor::Tick(float DeltaTime) {
    if (bRequestPreviewRebuild) {
        RebuildPreviewObjectImpl();
    }
}

bool FSnapConnectionEditor::IsTickable() const {
    return true;
}

TStatId FSnapConnectionEditor::GetStatId() const {
    return TStatId();
}

TSharedRef<SDockTab> FSnapConnectionEditor::SpawnTab_Graph(const FSpawnTabArgs& Args) {
    AssetDropTarget = SNew(SThemeEditorDropTarget)
		.OnAssetDropped(this, &FSnapConnectionEditor::HandleAssetDropped)
		.OnIsAssetAcceptableForDrop(this, &FSnapConnectionEditor::IsAssetAcceptableForDrop)
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

TSharedRef<SDockTab> FSnapConnectionEditor::SpawnTab_Preview3D(const FSpawnTabArgs& Args) {
    TSharedRef<SDockTab> SpawnedTab =
        SNew(SDockTab)
		.Label(LOCTEXT("Preview3D", "Preview 3D"))
		.TabColorScale(GetTabColorScale())
        [
            PreviewViewport.ToSharedRef()
        ];

    PreviewViewport->SetParentTab(SpawnedTab);
    return SpawnedTab;
}

TSharedRef<SDockTab> FSnapConnectionEditor::SpawnTab_PreviewSettings(const FSpawnTabArgs& Args) {
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

TSharedRef<SDockTab> FSnapConnectionEditor::SpawnTab_Details(const FSpawnTabArgs& Args) {
    // Spawn the tab
    return SNew(SDockTab)
        .Label(LOCTEXT("DetailsTab_Title", "Details"))
        [
            DetailsPanel.ToSharedRef()
        ];
}

TSharedRef<SDockTab> FSnapConnectionEditor::SpawnTab_ContentBrowser(const FSpawnTabArgs& Args) {
    TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
        .Label(LOCTEXT("ContentBrowserKey", "Content Browser"))
        .TabColorScale(GetTabColorScale());
    
    IContentBrowserSingleton& ContentBrowserSingleton = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser").Get();
    const FName ContentBrowserInstanceID = *("DA_SnapConnectionEd_ContentBrowser_" + (AssetBeingEdited ? AssetBeingEdited->GetFullName() : "[NONE]")); 
    FContentBrowserConfig ContentBrowserConfig;
    TSharedRef<SWidget, ESPMode::NotThreadSafe> ContentBrowser = ContentBrowserSingleton.CreateContentBrowser(
            ContentBrowserInstanceID, SpawnedTab, &ContentBrowserConfig);
    
    SpawnedTab->SetContent(ContentBrowser);
    return SpawnedTab;
}

TSharedRef<SGraphEditor> FSnapConnectionEditor::CreateGraphEditorWidget(UEdGraph* InGraph) const {
    // Create the appearance info
    FGraphAppearanceInfo AppearanceInfo;
    AppearanceInfo.CornerText = LOCTEXT("AppearanceCornerText", "Snap Connection");


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

    TSharedRef<SGraphEditor> _GraphEditor = SNew(SGraphEditor)
        .AdditionalCommands(ThemeGraphHandler->GetCommands())
        .Appearance(AppearanceInfo)
        .GraphToEdit(InGraph)
        .GraphEvents(ThemeGraphHandler->GraphEvents);

    return _GraphEditor;
}

void FSnapConnectionEditor::SaveAsset_Execute() {
    UE_LOG(SnapConnectionEditor, Log, TEXT("Saving snap door asset %s"), *GetEditingObjects()[0]->GetName());

    CompileAsset();
    UpdateThumbnail();
    AssetBeingEdited->Version = (int32)ESnapConnectionInfoVersion::LatestVersion;

    // Refresh the editors
    {
        FEditorDelegates::RefreshEditor.Broadcast();
        FEditorSupportDelegates::RedrawAllViewports.Broadcast();
    }
    
    UPackage* Package = AssetBeingEdited->GetOutermost();
    if (!Package) return;

    TArray<UPackage*> PackagesToSave;
    PackagesToSave.Add(Package);

    FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, false, false);
}

void FSnapConnectionEditor::CompileAsset() const {
    if (AssetBeingEdited && AssetBeingEdited->ThemeAsset->UpdateGraph) {
        if (UEdGraph_DungeonProp* ThemeGraph = Cast<UEdGraph_DungeonProp>(AssetBeingEdited->ThemeAsset->UpdateGraph)) {
            TArray<FDungeonGraphBuildError> CompileErrors;
            ThemeGraph->RebuildGraph(AssetBeingEdited, AssetBeingEdited->ThemeAsset->Props, CompileErrors);
            ThemeGraph->Modify();
        }
    }
}

void FSnapConnectionEditor::UpdateThumbnail() const {
    if (!AssetBeingEdited) return;
    FViewport* PreviewViewportPtr = nullptr;
    if (PreviewViewport.IsValid() && PreviewViewport->GetViewportClient().IsValid()) {
        PreviewViewportPtr = PreviewViewport->GetViewportClient()->Viewport;
    }

    if (PreviewViewportPtr) {
        const FAssetData AssetData(AssetBeingEdited);
        TArray<FAssetData> ThemeAssetList;
        ThemeAssetList.Add(AssetData);

        IContentBrowserSingleton& ContentBrowserSingleton = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser").Get();
        ContentBrowserSingleton.CaptureThumbnailFromViewport(PreviewViewportPtr, ThemeAssetList);
    }
}

FVector2D FSnapConnectionEditor::GetAssetDropGridLocation() const {
    if (!AssetDropTarget.IsValid()) return FVector2D::ZeroVector;

    const FVector2D PanelCoord = AssetDropTarget->GetPanelCoordDropPosition();
    FVector2D ViewLocation = FVector2D::ZeroVector;
    float ZoomAmount = 1.0f;
    GraphEditor->GetViewLocation(ViewLocation, ZoomAmount);
    const FVector2D GridLocation = PanelCoord / ZoomAmount + ViewLocation;

    return GridLocation;
}

void FSnapConnectionEditor::HandleAssetDropped(UObject* AssetObject) {
    if (GraphEditor.IsValid()) {
        UEdGraph_DungeonProp* ThemeGraph = Cast<UEdGraph_DungeonProp>(GraphEditor->GetCurrentGraph());
        if (ThemeGraph) {
            const FVector2D GridLocation = GetAssetDropGridLocation();
            ThemeGraph->CreateNewNode(AssetObject, GridLocation);
        }
    }
}

bool FSnapConnectionEditor::IsAssetAcceptableForDrop(const UObject* AssetObject) const {
    if (GraphEditor.IsValid()) {
        UEdGraph_DungeonProp* ThemeGraph = Cast<UEdGraph_DungeonProp>(GraphEditor->GetCurrentGraph());
        if (ThemeGraph) {
            bool bCanDrop = ThemeGraph->IsAssetAcceptableForDrop(AssetObject);

            if (!bCanDrop) {
                // Check if a broker can convert this asset to an actor
                const FAssetData AssetData(AssetObject);
                const bool bHasActorFactory = FActorFactoryAssetProxy::GetFactoryForAsset(AssetData) != nullptr;

                if (bHasActorFactory) {
                    bCanDrop = true;
                }
            }

            return bCanDrop;
        }
    }
    return false;
}

void FSnapConnectionEditor::SetPreviewMode(const FString& InMarkerName) {
    if (PreviewMarkerName != InMarkerName) {
        PreviewMarkerName = InMarkerName;
        RequestRebuildPreviewObject();
    }
}

FSnapConnectionEditor::~FSnapConnectionEditor() {
    // Remove the graph editor change notification callbacks
    if (GraphEditor.IsValid() && GraphEditor->GetCurrentGraph()) {
        GraphEditor->GetCurrentGraph()->RemoveOnGraphChangedHandler(OnGraphChangedDelegateHandle);
    }
}

void FSnapConnectionEditor::RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager) {
    WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(
        LOCTEXT("WorkspaceMenu_SnapConnectionEditor", "Snap Connection Editor"));
    auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

    FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

    InTabManager->RegisterTabSpawner(FSnapConnectionEditorTabs::GraphID,
                FOnSpawnTab::CreateSP(this, &FSnapConnectionEditor::SpawnTab_Graph))
                .SetDisplayName(LOCTEXT("GraphTab", "Graph"))
                .SetGroup(WorkspaceMenuCategoryRef)
                .SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports"));

    InTabManager->RegisterTabSpawner(FSnapConnectionEditorTabs::Preview3DID,
                FOnSpawnTab::CreateSP(this, &FSnapConnectionEditor::SpawnTab_Preview3D))
                .SetDisplayName(LOCTEXT("Preview3DTab", "Preview 3D"))
                .SetGroup(WorkspaceMenuCategoryRef)
                .SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports"));
    
    InTabManager->RegisterTabSpawner(FSnapConnectionEditorTabs::PreviewSettingsID,
                FOnSpawnTab::CreateSP(this, &FSnapConnectionEditor::SpawnTab_PreviewSettings))
                .SetDisplayName(LOCTEXT("PreviewSettingsTab", "Preview Settings"))
                .SetGroup(WorkspaceMenuCategoryRef)
                .SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));
    
    InTabManager->RegisterTabSpawner(FSnapConnectionEditorTabs::DetailsID,
                FOnSpawnTab::CreateSP(this, &FSnapConnectionEditor::SpawnTab_Details))
                .SetDisplayName(LOCTEXT("DetailsTabLabel", "Details"))
                .SetGroup(WorkspaceMenuCategoryRef)
                .SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

    InTabManager->RegisterTabSpawner(FSnapConnectionEditorTabs::ContentBrowserID,
            FOnSpawnTab::CreateSP(this, &FSnapConnectionEditor::SpawnTab_ContentBrowser))
            .SetDisplayName(LOCTEXT("GraphTab", "Graph"))
            .SetGroup(WorkspaceMenuCategoryRef)
            .SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports"));

}

void FSnapConnectionEditor::UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager) {
    FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

    InTabManager->UnregisterTabSpawner(FSnapConnectionEditorTabs::GraphID);
    InTabManager->UnregisterTabSpawner(FSnapConnectionEditorTabs::Preview3DID);
    InTabManager->UnregisterTabSpawner(FSnapConnectionEditorTabs::DetailsID);
    InTabManager->UnregisterTabSpawner(FSnapConnectionEditorTabs::ContentBrowserID);
}

void FSnapConnectionEditor::ExtendMenu() {
    struct Local {
        static void FillToolbar(FToolBarBuilder& ToolbarBuilder) {
            ToolbarBuilder.BeginSection("SnapConnection");
            {
                ToolbarBuilder.AddToolBarButton(FSnapConnectionEditorCommands::Get().Rebuild);
            }
            ToolbarBuilder.EndSection();
        }
    };

    TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
    ToolbarExtender->AddToolBarExtension(
        "Asset",
        EExtensionHook::After,
        GetToolkitCommands(),
        FToolBarExtensionDelegate::CreateStatic(&Local::FillToolbar)
    );
    AddToolbarExtender(ToolbarExtender);
}


void FSnapConnectionEditor::BindCommands() {

    const FSnapConnectionEditorCommands& Commands = FSnapConnectionEditorCommands::Get();

    ToolkitCommands->MapAction(
        Commands.Rebuild,
        FExecuteAction::CreateSP(this, &FSnapConnectionEditor::HandleRebuildActionExecute));
}

void FSnapConnectionEditor::HandleRebuildActionExecute() {
    RequestRebuildPreviewObject();
}

void FSnapConnectionEditor::DestroyPreviewObjects() {
    for (TWeakObjectPtr<AActor> ConnectionInstanceActor : ConnectionInstanceActors) {
        if (ConnectionInstanceActor.IsValid()) {
            ConnectionInstanceActor->Destroy();
        }
    }
    
    ConnectionInstanceActors.Reset();
}

void FSnapConnectionEditor::SetConnectionStateFromMarker(const FString& InMarkerName, USnapConnectionComponent* ConnectionComponent) {
    if (!ConnectionComponent) return;

    if (InMarkerName == FSnapConnectionMarkers::Wall) {
        ConnectionComponent->ConnectionState = ESnapConnectionState::Wall;
    }
    else {
        ConnectionComponent->ConnectionState = ESnapConnectionState::Door;
        ConnectionComponent->DoorType = ESnapConnectionDoorType::CustomDoor;    // We don't care about the door type meta-data while previewing
        ConnectionComponent->MarkerName = InMarkerName;
    }
}


void FSnapConnectionEditor::RequestRebuildPreviewObject() {
    bRequestPreviewRebuild = true;
}

void FSnapConnectionEditor::RebuildPreviewObjectImpl() {
    // Destroy the old connection instance actor
    DestroyPreviewObjects();
    
    CompileAsset();

    // Rebuild the connection instance
    {
        ASnapConnectionActor* ConnectionActor = PreviewViewport->GetConnectionActor();
        USnapConnectionComponent* ConnectionComponent = ConnectionActor->ConnectionComponent;
        ConnectionComponent->ConnectionInfo = AssetBeingEdited;
        SetConnectionStateFromMarker(PreviewMarkerName, ConnectionComponent);
        ConnectionActor->BuildConnectionInstance();
        ConnectionInstanceActors = ConnectionActor->GetSpawnedInstancesPtr();
    }

    // Highlight the appropriate marker node
    if (AssetBeingEdited && AssetBeingEdited->ThemeAsset && AssetBeingEdited->ThemeAsset->UpdateGraph) {
        UEdGraph* ThemeGraph = AssetBeingEdited->ThemeAsset->UpdateGraph;
        TArray<FString> SelectedMarkers;
        for (UEdGraphNode* Node : ThemeGraph->Nodes) {
            if (UEdGraphNode_DungeonMarker* MarkerNode = Cast<UEdGraphNode_DungeonMarker>(Node)) {
                if (MarkerNode->MarkerName == "") continue;
                
                if (MarkerNode->MarkerName == PreviewMarkerName) {
                    MarkerNode->ErrorType = EMessageSeverity::Error;
                    MarkerNode->ErrorMsg = "PREVIEWING";
                    GraphEditor->RefreshNode(*MarkerNode);
                }
                else if (MarkerNode->ErrorMsg != "") {
                    MarkerNode->ErrorMsg = "";
                    GraphEditor->RefreshNode(*MarkerNode);
                }
            }
        }
    }
    
    bRequestPreviewRebuild = false;
}

UEdGraph* FSnapConnectionEditor::CreateNewThemeGraph() const {
    UEdGraph_DungeonProp* NewGraph = NewObject<UEdGraph_DungeonProp>(
            AssetBeingEdited, UEdGraph_DungeonProp::StaticClass(), NAME_None, RF_Transactional);

    static const TArray<FString> DefaultMarkers = {
        FSnapConnectionMarkers::Wall,
        FSnapConnectionMarkers::Door,
        FSnapConnectionMarkers::OneWayDoor,
        FSnapConnectionMarkers::OneWayDoorUp,
        FSnapConnectionMarkers::OneWayDoorDown,
    };
    
    // Recreate default markers
    {
        TArray<FString> NewGraphMarkers = DefaultMarkers;
        NewGraphMarkers.Add("MyLockedDoor1");
        NewGraph->RecreateDefaultMarkerNodes(NewGraphMarkers);
    }

    // Make the default markers read-only
    for (UEdGraphNode* Node : NewGraph->Nodes) {
        if (UEdGraphNode_DungeonMarker* MarkerNode = Cast<UEdGraphNode_DungeonMarker>(Node)) {
            const bool bReadOnly = DefaultMarkers.Contains(MarkerNode->MarkerName); 
            MarkerNode->bUserDefined = !bReadOnly;
            MarkerNode->bBuilderEmittedMarker = bReadOnly;
        }
    }
    return NewGraph;
}

#undef LOCTEXT_NAMESPACE

