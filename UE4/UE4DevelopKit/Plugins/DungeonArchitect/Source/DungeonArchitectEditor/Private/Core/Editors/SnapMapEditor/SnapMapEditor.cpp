//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/SnapMapEditor/SnapMapEditor.h"

#include "Builders/SnapMap/SnapMapAsset.h"
#include "Core/Editors/SnapMapEditor/AppModes/DebugAppMode.h"
#include "Core/Editors/SnapMapEditor/AppModes/GraphDesignAppMode.h"
#include "Core/Editors/SnapMapEditor/AppModes/VisualizeAppMode.h"
#include "Core/Editors/SnapMapEditor/SnapMapEditorCommands.h"
#include "Core/Editors/SnapMapEditor/SnapMapEditorToolbar.h"
#include "Core/Editors/SnapMapEditor/SnapMapEditorUtils.h"
#include "Frameworks/GraphGrammar/RuleGraph/Debugging/EdGraph_DebugGrammar.h"
#include "Frameworks/GraphGrammar/RuleGraph/EdGraph_Grammar.h"

#include "Subsystems/AssetEditorSubsystem.h"
#include "Toolkits/AssetEditorManager.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"

#define LOCTEXT_NAMESPACE "SnapMapEditor"

DEFINE_LOG_CATEGORY_STATIC(SnapMapEditor, Log, All);

const FName FSnapMapEditor::GraphEditorModeID(TEXT("GraphEditorMode"));
const FName FSnapMapEditor::VisualizationModeID(TEXT("VisualizationMode"));
const FName FSnapMapEditor::DebugModeID(TEXT("DebugMode"));
const FName DungeonFlowEditorAppName = FName(TEXT("DungeonFlowEditorApp"));

FSnapMapEditor::~FSnapMapEditor() {
    //TODO: Cleanup graph handlers
}

void FSnapMapEditor::InitFlowEditor(const EToolkitMode::Type Mode,
                                        const TSharedPtr<class IToolkitHost>& InitToolkitHost,
                                        USnapMapAsset* InDungeonFlow) {
    if (!DocumentManager.IsValid()) {
        DocumentManager = MakeShareable(new FDocumentTracker);
        DocumentManager->Initialize(SharedThis(this));
    }

    FSnapMapEditorCommands::Register();

    // Initialize the asset editor
    GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseOtherEditors(InDungeonFlow, this);

    AssetBeingEdited = InDungeonFlow;

    // Create the visualization graph
    VisualizationGraph = NewObject<UEdGraph_Grammar>();
    DebugGraph = NewObject<UEdGraph_DebugGrammar>();
    DebugModeSettings = NewObject<UFlowEditorDebugAppModeSettings>();

    const TSharedRef<FTabManager::FLayout> DummyLayout = FTabManager::NewLayout("NullLayout")->AddArea(
        FTabManager::NewPrimaryArea());

    if (!ToolbarBuilder.IsValid()) {
        ToolbarBuilder = MakeShareable(new FSnapMapEditorToolbar(SharedThis(this)));
    }

    // Initialize the asset editor and spawn nothing (dummy layout)
    const bool bCreateDefaultStandaloneMenu = true;
    const bool bCreateDefaultToolbar = true;
    InitAssetEditor(Mode, InitToolkitHost, DungeonFlowEditorAppName, DummyLayout, bCreateDefaultStandaloneMenu,
                    bCreateDefaultToolbar, AssetBeingEdited);

    AddApplicationMode(GraphEditorModeID, MakeShareable(new FSnapMapEditor_GraphDesignAppMode(SharedThis(this))));
    AddApplicationMode(VisualizationModeID, MakeShareable(new FSnapMapEditor_VisualizeAppMode(SharedThis(this))));
    AddApplicationMode(DebugModeID, MakeShareable(new FSnapMapEditor_DebugAppMode(SharedThis(this))));

    SetCurrentMode(GraphEditorModeID);
}


FName FSnapMapEditor::GetToolkitFName() const {
    return FName("DungeonFlow");
}

FText FSnapMapEditor::GetBaseToolkitName() const {
    return LOCTEXT("SnapMapEditorAppLabel", "SnapMap Editor");
}

FText FSnapMapEditor::GetToolkitName() const {
    const bool bDirtyState = AssetBeingEdited->GetOutermost()->IsDirty();

    FFormatNamedArguments Args;
    Args.Add(TEXT("DungeonName"), FText::FromString(AssetBeingEdited->GetName()));
    Args.Add(TEXT("DirtyState"), bDirtyState ? FText::FromString(TEXT("*")) : FText::GetEmpty());
    return FText::Format(LOCTEXT("SnapMapEditorAppTitle", "{DungeonName}"), Args);
}

FLinearColor FSnapMapEditor::GetWorldCentricTabColorScale() const {
    return FLinearColor::White;
}

FString FSnapMapEditor::GetWorldCentricTabPrefix() const {
    return TEXT("SnapMapEditor");
}

FString FSnapMapEditor::GetDocumentationLink() const {
    // TODO: Fix me
    return TEXT("Dungeon/SnapMapEditor");
}

void FSnapMapEditor::SaveAsset_Execute() {
    if (AssetBeingEdited && AssetBeingEdited->MissionGrammar) {
        FSnapMapEditorUtils::CompileGrammarToScript(AssetBeingEdited->MissionGrammar);
    }

    TSharedPtr<FSnapMapEdAppModeBase> AppMode = StaticCastSharedPtr<FSnapMapEdAppModeBase>(GetCurrentModePtr());
    if (AppMode.IsValid()) {
        AppMode->OnAssetSave();
    }

    ISnapMapEditor::SaveAsset_Execute();
}

void FSnapMapEditor::AddReferencedObjects(FReferenceCollector& Collector) {
    Collector.AddReferencedObject(VisualizationGraph);
    Collector.AddReferencedObject(DebugGraph);
    Collector.AddReferencedObject(DebugModeSettings);
}

void FSnapMapEditor::NotifyPreChange(FProperty* PropertyAboutToChange) {

}

void FSnapMapEditor::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent,
                                          FProperty* PropertyThatChanged) {
}

bool FSnapMapEditor::IsTickableInEditor() const {
    return true;
}

void FSnapMapEditor::Tick(float DeltaTime) {
    TSharedPtr<FSnapMapEdAppModeBase> AppMode = StaticCastSharedPtr<FSnapMapEdAppModeBase>(GetCurrentModePtr());
    if (AppMode.IsValid()) {
        AppMode->Tick(DeltaTime);
    }
}

bool FSnapMapEditor::IsTickable() const {
    return true;
}

TStatId FSnapMapEditor::GetStatId() const {
    return TStatId();
}

FText FSnapMapEditor::GetLocalizedMode(FName InMode) {
    static TMap<FName, FText> LocModes;

    if (LocModes.Num() == 0) {
        LocModes.Add(GraphEditorModeID, LOCTEXT("GraphEditorMode", "Design"));
        LocModes.Add(VisualizationModeID, LOCTEXT("VisualizationMode", "Visualize"));
        LocModes.Add(DebugModeID, LOCTEXT("DebugMode", "Debug"));
    }

    check(InMode != NAME_None);
    const FText* OutDesc = LocModes.Find(InMode);
    check(OutDesc);
    return *OutDesc;
}

void FSnapMapEditor::RegisterToolbarTab(const TSharedRef<class FTabManager>& InTabManager) {
    FAssetEditorToolkit::RegisterTabSpawners(InTabManager);
}

USnapMapAsset* FSnapMapEditor::GetAssetBeingEdited() const {
    return AssetBeingEdited;
}

UEdGraph_Grammar* FSnapMapEditor::GetVisualizationGraph() const {
    return VisualizationGraph;
}

UEdGraph_DebugGrammar* FSnapMapEditor::GetDebugGraph() const {
    return DebugGraph;
}

UFlowEditorDebugAppModeSettings* FSnapMapEditor::GetDebugAppModeSettings() const {
    return DebugModeSettings;
}

bool FSnapMapEditor::CanAccessGraphEditMode() const {
    return true;
}

bool FSnapMapEditor::CanAccessVisualizationMode() const {
    return true;
}


bool FSnapMapEditor::CanAccessDebugMode() const {
    return VisualizationGraph && VisualizationGraph->Nodes.Num() > 0;
}

void FSnapMapEditor::RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager) {
    FWorkflowCentricApplication::RegisterTabSpawners(InTabManager);
}

void FSnapMapEditor::UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager) {
    FWorkflowCentricApplication::UnregisterTabSpawners(InTabManager);
}


////////////////////////////// UDungeonFlowEditorCallbackHandler ////////////////////////////// 

void USnapMapEditorCallbackHandler::OnPostDungeonBuild_Implementation(ADungeon* Dungeon) {
    OnDungeonBuilt.ExecuteIfBound(Dungeon);
}

void USnapMapEditorCallbackHandler::OnDungeonDestroyed_Implementation(ADungeon* Dungeon) {
    OnDungeonDestroyed.ExecuteIfBound(Dungeon);
}

#undef LOCTEXT_NAMESPACE

