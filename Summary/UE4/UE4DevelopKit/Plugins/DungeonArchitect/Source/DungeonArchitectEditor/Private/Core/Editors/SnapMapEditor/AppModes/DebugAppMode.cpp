//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/SnapMapEditor/AppModes/DebugAppMode.h"

#include "Builders/SnapMap/SnapMapDungeonConfig.h"
#include "Core/Dungeon.h"
#include "Core/Editors/SnapMapEditor/SnapMapEditor.h"
#include "Core/Editors/SnapMapEditor/SnapMapEditorCommands.h"
#include "Core/Editors/SnapMapEditor/SnapMapEditorTabFactories.h"
#include "Core/Editors/SnapMapEditor/SnapMapEditorTabs.h"
#include "Core/Editors/SnapMapEditor/SnapMapEditorToolbar.h"
#include "Core/Editors/SnapMapEditor/Viewport/SSnapMapEditorViewport.h"
#include "Core/Utils/EditorService/IDungeonEditorService.h"
#include "Frameworks/GraphGrammar/Editor/SGrammarRuleGraph.h"
#include "Frameworks/GraphGrammar/RuleGraph/Debugging/EdGraphNode_DebugGrammarNode.h"
#include "Frameworks/GraphGrammar/RuleGraph/Debugging/EdGraph_DebugGrammar.h"
#include "Frameworks/GraphGrammar/RuleGraph/EdGraph_Grammar.h"
#include "Frameworks/LevelStreaming/DungeonLevelStreamingModel.h"
#include "Frameworks/Snap/Lib/Connection/SnapConnectionActor.h"
#include "Frameworks/Snap/Lib/Connection/SnapConnectionComponent.h"

#include "AdvancedPreviewScene.h"
#include "DrawDebugHelpers.h"
#include "Engine/LevelStreaming.h"
#include "Engine/LevelStreamingDynamic.h"
#include "IDetailsView.h"
#include "Modules/ModuleManager.h"
#include "PreviewScene.h"
#include "PropertyEditorModule.h"
#include "SAdvancedPreviewDetailsTab.h"

////////////////////// FSnapMapEditor_DebugAppMode //////////////////////

#define LOCTEXT_NAMESPACE "SnapMapEditor_DebugAppMode"

DEFINE_LOG_CATEGORY_STATIC(LogFlowDebugAppMode, Log, All);

FSnapMapEditor_DebugAppMode::FSnapMapEditor_DebugAppMode(TSharedPtr<class FSnapMapEditor> InFlowEditor)
    : FSnapMapEdAppModeBase(FSnapMapEditor::DebugModeID)
      , FlowEditor(InFlowEditor) {
    USnapMapAsset* AssetBeingEdited = InFlowEditor->GetAssetBeingEdited();

    // Initialize the visualization graph editor
    {
        UEdGraph_DebugGrammar* DebugGraph = InFlowEditor->GetDebugGraph();
        DebugGraphEditor.Initialize(DebugGraph, false);

        TSharedPtr<FGrammarRuleGraphHandler> GraphHandler = DebugGraphEditor.GetGraphWidget()->GetGraphHandler();
        if (GraphHandler.IsValid()) {
            GraphHandler->OnNodeDoubleClicked.BindRaw(this, &FSnapMapEditor_DebugAppMode::OnResultNodeDoubleClicked);
        }
    }

    // Create the 3D viewport
    Viewport = SNew(SSnapMapEditorViewport)
		.DungeonFlowEditor(InFlowEditor)
		.ObjectToEdit(AssetBeingEdited);
    ViewportSceneSettings = SNew(SAdvancedPreviewDetailsTab, Viewport->GetAdvancedPreview().ToSharedRef());

    // Create the visualization actor
    {
        UWorld* World = Viewport->GetAdvancedPreview()->GetWorld();
        VisualizationActor = World->SpawnActor<ASnapMapFlowEditorVisualization>();

        if (VisualizationActor) {
            UEdGraph_DebugGrammar* DebugGraph = InFlowEditor->GetDebugGraph();
            DebugGraph->GetOnVisualizeLoadLevel().BindUObject(VisualizationActor, &ASnapMapFlowEditorVisualization::LoadLevel);
            DebugGraph->GetOnVisualizeUnloadLevel().BindUObject(VisualizationActor, &ASnapMapFlowEditorVisualization::UnloadLevel);
            DebugGraph->GetOnVisualizeSetConnectionState().BindUObject(VisualizationActor, &ASnapMapFlowEditorVisualization::UpdateConnectionState);
            DebugGraph->GetOnSnapMapDiagVisSetDebugBox().BindUObject(VisualizationActor, &ASnapMapFlowEditorVisualization::SetDebugBox);
        }
    }


    // Create the visualization details property editor widget
    {
        FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
        const FDetailsViewArgs DetailsViewArgs(false, false, true, FDetailsViewArgs::HideNameArea, true, this);
        const TSharedRef<IDetailsView> PropertyEditorRef = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
        PropertyEditor = PropertyEditorRef;
        PropertyEditor->SetObject(InFlowEditor->GetDebugAppModeSettings());
    }

    BindCommands(InFlowEditor->GetToolkitCommands());

    // Register the tab factories
    TabFactories.RegisterFactory(MakeShareable(new FSnapMapEditorTabFactory_VisualizeDetails(InFlowEditor, PropertyEditor)));
    TabFactories.RegisterFactory(MakeShareable(new FSnapMapEditorTabFactory_Viewport(InFlowEditor, Viewport)));
    TabFactories.RegisterFactory(MakeShareable(new FSnapMapEditorTabFactory_ViewportSceneSettings(InFlowEditor, ViewportSceneSettings)));
    TabFactories.RegisterFactory(MakeShareable(new FSnapMapEditorTabFactory_VisualizeResultGraph(InFlowEditor, DebugGraphEditor.GetGraphWidget())));

    TabLayout = BuildEditorFrameLayout(InFlowEditor);

    InFlowEditor->GetToolbarBuilder()->AddModesToolbar(ToolbarExtender);
    InFlowEditor->GetToolbarBuilder()->AddFlowDebugToolbar(ToolbarExtender);
}

FSnapMapEditor_DebugAppMode::~FSnapMapEditor_DebugAppMode() {
}

void FSnapMapEditor_DebugAppMode::RegisterTabFactories(TSharedPtr<class FTabManager> InTabManager) {
    check(FlowEditor.IsValid());
    TSharedPtr<FSnapMapEditor> FlowEditorPtr = FlowEditor.Pin();

    FlowEditorPtr->RegisterToolbarTab(InTabManager.ToSharedRef());

    FlowEditorPtr->PushTabFactories(TabFactories);

    FApplicationMode::RegisterTabFactories(InTabManager);
}

void FSnapMapEditor_DebugAppMode::PreDeactivateMode() {
    FApplicationMode::PreDeactivateMode();

    check(FlowEditor.IsValid());
    TSharedPtr<FSnapMapEditor> FlowEditorPtr = FlowEditor.Pin();

}

void FSnapMapEditor_DebugAppMode::PostActivateMode() {
    check(FlowEditor.IsValid());
    TSharedPtr<FSnapMapEditor> FlowEditorPtr = FlowEditor.Pin();

    FApplicationMode::PostActivateMode();
}

void FSnapMapEditor_DebugAppMode::AddReferencedObjects(FReferenceCollector& Collector) {
    Collector.AddReferencedObject(VisualizationActor);
}

void FSnapMapEditor_DebugAppMode::OnResultNodeDoubleClicked(UEdGraphNode* InNode) {
    if (UEdGraphNode_DebugGrammarNode* DebugNode = Cast<UEdGraphNode_DebugGrammarNode>(InNode)) {
        if (DebugNode->bModuleAssigned) {
            const FBox BoundsToFocus = DebugNode->ModuleBounds.TransformBy(DebugNode->WorldTransform);
            Viewport->GetViewportClient()->FocusViewportOnBox(BoundsToFocus);
        }
    }
    else if (UEdGraphNode_DebugGrammarDoorNode* DoorNode = Cast<UEdGraphNode_DebugGrammarDoorNode>(InNode)) {
        if (DoorNode->Outgoing) {
            const FBox BoundsToFocus = DoorNode->Outgoing->IncomingDoorBounds;
            Viewport->GetViewportClient()->FocusViewportOnBox(BoundsToFocus);
        }
    }
}

void FSnapMapEditor_DebugAppMode::HandleDebugRestart() {
    check(FlowEditor.IsValid());
    TSharedPtr<FSnapMapEditor> FlowEditorPtr = FlowEditor.Pin();
    UEdGraph_DebugGrammar* DebugGraph = FlowEditorPtr->GetDebugGraph();
    if (DebugGraph) {
        DebugGraph->Rewind();
    }
}

void FSnapMapEditor_DebugAppMode::HandleDebugStepForward() {
    check(FlowEditor.IsValid());
    TSharedPtr<FSnapMapEditor> FlowEditorPtr = FlowEditor.Pin();
    UEdGraph_DebugGrammar* DebugGraph = FlowEditorPtr->GetDebugGraph();
    if (DebugGraph) {
        DebugGraph->ExecuteStep();
    }
}

void FSnapMapEditor_DebugAppMode::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent,
                                                       FProperty* PropertyThatChanged) {
    FName PropertyName = PropertyThatChanged ? PropertyThatChanged->GetFName() : NAME_None;

    bool bLayoutSettingsChanged = (PropertyName == GET_MEMBER_NAME_CHECKED(
            UFlowEditorDebugAppModeSettings, SpacingHorizontal))
        || (PropertyName == GET_MEMBER_NAME_CHECKED(UFlowEditorDebugAppModeSettings, SpacingVertical));


    if (bLayoutSettingsChanged && FlowEditor.IsValid()) {
        TSharedPtr<FSnapMapEditor> FlowEditorPtr = FlowEditor.Pin();
        UFlowEditorDebugAppModeSettings* Settings = FlowEditorPtr->GetDebugAppModeSettings();

        UEdGraph_DebugGrammar* DebugGraph = FlowEditorPtr->GetDebugGraph();
        DebugGraph->LayoutGraph(Settings->SpacingHorizontal, Settings->SpacingVertical);
        DebugGraph->NotifyGraphChanged();
    }
}

void FSnapMapEditor_DebugAppMode::BindCommands(TSharedRef<FUICommandList> ToolkitCommands) {
    const FSnapMapEditorCommands& Commands = FSnapMapEditorCommands::Get();

    ToolkitCommands->MapAction(
        Commands.DebugRestart,
        FExecuteAction::CreateRaw(this, &FSnapMapEditor_DebugAppMode::HandleDebugRestart));

    ToolkitCommands->MapAction(
        Commands.DebugStepForward,
        FExecuteAction::CreateRaw(this, &FSnapMapEditor_DebugAppMode::HandleDebugStepForward));
}

TSharedRef<FTabManager::FLayout> FSnapMapEditor_DebugAppMode::BuildEditorFrameLayout(
    TSharedPtr<class FSnapMapEditor> InFlowEditor) {

    return FTabManager::NewLayout("Standalone_DungeonFlowEditor_DebugLayout_v0.0.8")
        ->AddArea
        (
            FTabManager::NewPrimaryArea()
            ->SetOrientation(Orient_Vertical)
            // Toolbar
            ->Split
            (
                FTabManager::NewStack()
                ->SetSizeCoefficient(0.1f)
                ->SetHideTabWell(true)
                ->AddTab(InFlowEditor->GetToolbarTabId(), ETabState::OpenedTab)
            )
            // Body of the editor
            ->Split
            (
                FTabManager::NewSplitter()
                ->SetOrientation(Orient_Vertical)

                // Upper half of the editor
                ->Split
                (
                    FTabManager::NewStack()
                    ->AddTab(FSnapMapEditorTabs::VisualizeResultGraphID, ETabState::OpenedTab)
                    ->SetHideTabWell(true)
                    ->SetSizeCoefficient(1)
                )

                // Lower half of the editor
                ->Split
                (
                    FTabManager::NewSplitter()
                    ->SetOrientation(Orient_Horizontal)
                    ->SetSizeCoefficient(1)
                    ->Split
                    (
                        FTabManager::NewStack()
                        ->SetSizeCoefficient(0.15f)
                        ->AddTab(FSnapMapEditorTabs::VisualizeDetailsID, ETabState::OpenedTab)
                        ->AddTab(FSnapMapEditorTabs::ViewportSceneSettingsID, ETabState::OpenedTab)
                        ->SetForegroundTab(FSnapMapEditorTabs::VisualizeDetailsID)
                    )
                    ->Split
                    (
                        FTabManager::NewStack()
                        ->SetSizeCoefficient(0.85f)
                        ->AddTab(FSnapMapEditorTabs::ViewportID, ETabState::OpenedTab)
                        ->SetHideTabWell(true)
                    )
                )
            )
        );
}

/////////////////////// ASnapMapFlowEditorVisualization ///////////////////////

ASnapMapFlowEditorVisualization::ASnapMapFlowEditorVisualization() {
    USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>("SceneRoot");
    RootComponent = SceneRoot;
    bIsEditorOnlyActor = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    PrimaryActorTick.bCanEverTick = true;
    InstanceIdCounter = 0;
}

void ASnapMapFlowEditorVisualization::LoadLevel(const FGuid& InNodeId, TSoftObjectPtr<UWorld> ModuleLevel,
                             const FBox& ModuleBounds, const FTransform& InWorldTransform) {
    UnloadLevel(InNodeId);

    UWorld* World = GetWorld();
    bool bSuccess = false;
    UPackage* LevelPackage = nullptr;
    ULevelStreamingDynamic* LevelStreaming = FLevelStreamLoader::LoadLevelInstanceBySoftObjectPtr(
        World, ModuleLevel.GetLongPackageName(), ++InstanceIdCounter,
        InWorldTransform.GetLocation(), FRotator(InWorldTransform.GetRotation()), bSuccess, LevelPackage);

    if (LevelPackage != nullptr) {
        LoadedPackages.Add(InNodeId, LevelPackage);
    }

    if (bSuccess && LevelStreaming) {
        LevelStreaming->bShouldBlockOnLoad = true;
        LevelStreaming->bShouldBlockOnUnload = true;
        LevelStreaming->SetShouldBeLoaded(true);
        LevelStreaming->SetShouldBeVisible(true);
        LoadedLevels.Add(InNodeId, LevelStreaming);
        World->FlushLevelStreaming(EFlushLevelStreamingType::Full);

        ULevel* LoadedLevel = LevelStreaming->GetLoadedLevel();
        if (LoadedLevel) {
            TArray<AActor*> LevelActorList = LoadedLevel->Actors;
            for (AActor* LevelActor : LevelActorList) {
                if (ASnapConnectionActor* Connection = Cast<ASnapConnectionActor>(LevelActor)) {
                    Connection->ConnectionComponent->ConnectionState = ESnapConnectionState::Wall;
                    Connection->BuildConnectionInstance(LoadedLevel);
                }
            }
        }
    }
}

void ASnapMapFlowEditorVisualization::UpdateConnectionState(const FGuid& InNodeId, const FGuid& InConnectionId,
                                                            bool bIsDoor) {
    ULevelStreamingDynamic** SearchResult = LoadedLevels.Find(InNodeId);
    if (SearchResult) {
        ULevelStreamingDynamic* LevelStreaming = *SearchResult;
        if (LevelStreaming) {
            ULevel* LoadedLevel = LevelStreaming->GetLoadedLevel();
            if (LoadedLevel) {
                TArray<AActor*> LevelActorList = LoadedLevel->Actors;
                for (AActor* LevelActor : LevelActorList) {
                    if (ASnapConnectionActor* Connection = Cast<ASnapConnectionActor>(LevelActor)) {
                        if (Connection->GetConnectionId() == InConnectionId) {
                            Connection->ConnectionComponent->ConnectionState = bIsDoor ? ESnapConnectionState::Door : ESnapConnectionState::Wall;
                            Connection->BuildConnectionInstance(LoadedLevel);
                        }
                    }
                }
            }
        }
    }

    UPackage** PackageSearchResult = LoadedPackages.Find(InNodeId);
    if (PackageSearchResult) {
        UPackage* Package = *PackageSearchResult;
        if (Package) {
            Package->SetDirtyFlag(false);
        }
    }
}

void ASnapMapFlowEditorVisualization::UnloadLevel(const FGuid& InNodeId) {
    UWorld* World = GetWorld();
    ULevelStreamingDynamic** SearchResult = LoadedLevels.Find(InNodeId);
    if (SearchResult) {
        ULevelStreamingDynamic* OldLevelStreaming = *SearchResult;
        if (OldLevelStreaming) {
            OldLevelStreaming->SetIsRequestingUnloadAndRemoval(true);
            OldLevelStreaming->SetShouldBeLoaded(false);
            OldLevelStreaming->SetShouldBeVisible(false);

            World->UpdateLevelStreaming();
            World->FlushLevelStreaming(EFlushLevelStreamingType::Full);
            World->RemoveStreamingLevel(OldLevelStreaming);
        }
        LoadedLevels.Remove(InNodeId);
    }
    UPackage** PackageSearchResult = LoadedPackages.Find(InNodeId);
    if (PackageSearchResult) {
        TSharedPtr<IDungeonEditorService> EditorService = IDungeonEditorService::Get();
        if (EditorService.IsValid()) {
            UPackage* Package = *PackageSearchResult;
            Package->SetDirtyFlag(false);
            if (!EditorService->UnloadPackages({Package})) {
                UE_LOG(LogFlowDebugAppMode, Error, TEXT("Failed to unload streaming level packages"));
            }
        }
        LoadedPackages.Remove(InNodeId);
    }
}

void ASnapMapFlowEditorVisualization::UnloadAllLevels() {
    TArray<ULevelStreaming*> LevelsToRemove;
    for (auto& Entry : LoadedLevels) {
        ULevelStreamingDynamic* LoadedLevel = Entry.Value;
        if (LoadedLevel) {
            LoadedLevel->SetIsRequestingUnloadAndRemoval(true);
        }

        LevelsToRemove.Add(LoadedLevel);
    }

    UWorld* World = GetWorld();
    World->UpdateLevelStreaming();
    World->FlushLevelStreaming(EFlushLevelStreamingType::Full);
    World->RemoveStreamingLevels(LevelsToRemove);

    LoadedLevels.Reset();

    TArray<UPackage*> PackagesToUnload;
    LoadedPackages.GenerateValueArray(PackagesToUnload);
    for (UPackage* Package : PackagesToUnload) {
        Package->SetDirtyFlag(false);
    }
    TSharedPtr<IDungeonEditorService> EditorService = IDungeonEditorService::Get();
    if (EditorService.IsValid()) {
        if (!EditorService->UnloadPackages(PackagesToUnload)) {
            UE_LOG(LogFlowDebugAppMode, Error, TEXT("Failed to unload streaming level packages"));
        }
    }
    LoadedPackages.Reset();

    if (GEngine) {
        GEngine->ForceGarbageCollection(true);
    }
}

void ASnapMapFlowEditorVisualization::SetDebugBox(const FBox& InDebugDrawBounds, const FColor& InColor) {
    DebugDrawBounds = InDebugDrawBounds;
    DebugDrawColor = InColor;
}

void ASnapMapFlowEditorVisualization::Tick(float DeltaSeconds) {
    DebugDraw();
}

void ASnapMapFlowEditorVisualization::DebugDraw() {
    if (DebugDrawBounds.IsValid) {
        DrawDebugBox(GetWorld(), DebugDrawBounds.GetCenter(), DebugDrawBounds.GetExtent(), DebugDrawColor, false, -1, 0,
                     50);
    }
}

#undef  LOCTEXT_NAMESPACE

