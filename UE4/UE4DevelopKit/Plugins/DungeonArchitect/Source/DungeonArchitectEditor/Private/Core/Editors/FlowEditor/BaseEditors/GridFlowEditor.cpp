//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/FlowEditor/BaseEditors/GridFlowEditor.h"

#include "Builders/GridFlow/GridFlowBuilder.h"
#include "Builders/GridFlow/GridFlowConfig.h"
#include "Core/Dungeon.h"
#include "Core/Editors/FlowEditor/DomainEditors/FlowDomainEdAbstractGraph.h"
#include "Core/Editors/FlowEditor/DomainEditors/FlowDomainEdTilemap.h"
#include "Core/Editors/FlowEditor/DomainMediators/AbstractGraphTilemapDomainMediator.h"
#include "Core/Editors/FlowEditor/FlowEditorSettings.h"
#include "Core/Editors/FlowEditor/FlowTestRunner.h"

#define LOCTEXT_NAMESPACE "GridFlowEditor"

//////////////////// Grid Flow Editor ////////////////////

void FGridFlowEditor::CreateDomainEditors() {
    DomainEditors.Reset();
    DomainMediators.Reset();

    const TSharedPtr<FFlowDomainEdAbstractGraph> DomainAbstractGraph = MakeShareable(new FFlowDomainEdAbstractGraph);
    const TSharedPtr<FFlowDomainEdTilemap> DomainTilemap = MakeShareable(new FFlowDomainEdTilemap);
    
    TSharedPtr<FAbstractGraphTilemapDomainMediator> AbstractGraphTilemapMediator = MakeShareable(new FAbstractGraphTilemapDomainMediator);
    AbstractGraphTilemapMediator->Initialize(DomainAbstractGraph, DomainTilemap, PreviewViewport);
    
    // Register the domain editors
    DomainEditors.Add(DomainAbstractGraph);
    DomainEditors.Add(DomainTilemap);

    // Register the domain mediators
    DomainMediators.Add(AbstractGraphTilemapMediator);

    // Initialize the domain editors
    for (IFlowDomainEditorPtr DomainEditor : DomainEditors) {
        DomainEditor->Initialize(PropertyEditor);
    }
}

FName FGridFlowEditor::GetFlowEdAppName() const {
    static const FName AppName = FName(TEXT("GridFlowEditor"));
    return AppName;
}

ADungeon* FGridFlowEditor::CreatePreviewDungeon(UWorld* World) {
    ADungeon* PreviewActor = World->SpawnActor<ADungeon>(FVector::ZeroVector, FQuat::Identity.Rotator());
    if (PreviewActor) {
        PreviewActor->SetBuilderClass(UGridFlowBuilder::StaticClass());
        UGridFlowConfig* GridFlowConfig = Cast<UGridFlowConfig>(PreviewActor->GetConfig());
        if (GridFlowConfig) {
            // Setup the grid flow graph asset to the currently edited asset
            GridFlowConfig->GridFlow = Cast<UGridFlowAsset>(AssetBeingEdited);
            GridFlowConfig->MaxBuildTimePerFrameMs = 30;
        }

        // Setup the default theme file
        UDungeonThemeAsset* Theme = Cast<UDungeonThemeAsset>(StaticLoadObject(
            UDungeonThemeAsset::StaticClass(), nullptr,
            TEXT("/DungeonArchitect/Core/Builders/GridFlowContent/Theme/T_DefaultGridFlow")));
        if (Theme) {
            PreviewActor->Themes.Reset();
            PreviewActor->Themes.Add(Theme);
        }
    }
    return PreviewActor;
}

FText FGridFlowEditor::GetEditorBrandingText() const {
    return LOCTEXT("EditorBranding", "GRID FLOW");
}

TSharedRef<SWidget> FGridFlowEditor::CreatePerfWidget(const TSharedRef<SDockTab> DockTab, TSharedPtr<SWindow> OwnerWindow) {
    return CreatePerfWidgetImpl<SGridFlowTestRunner>(DockTab, OwnerWindow);
}

void FGridFlowEditor::InitDungeonConfig(UDungeonConfig* Config) {
    UGridFlowConfig* GridFlowConfig = Cast<UGridFlowConfig>(Config);
    if (GridFlowConfig) {
        GridFlowConfig->ParameterOverrides = EditorSettings->ParameterOverrides;
    }
}

FName FGridFlowEditor::GetToolkitFName() const {
    return FName("GridFlowEditor");
}

FText FGridFlowEditor::GetBaseToolkitName() const {
    return LOCTEXT("GridFlowEditorAppLabel", "Grid Flow Editor");
}

FString FGridFlowEditor::GetWorldCentricTabPrefix() const {
    return TEXT("GridFlowEditor");
}

//////////////////// Grid Flow Perf Editor ////////////////////

void SGridFlowTestRunner::Construct(const FArguments& InArgs, const TSharedRef<SDockTab>& ConstructUnderMajorTab,
            const TSharedPtr<SWindow>& ConstructUnderWindow, TWeakObjectPtr<UFlowAssetBase> InFlowAsset)
{
    Super::FArguments ParentArgs;
    ParentArgs._OnServiceStarted = InArgs._OnServiceStarted;
    ParentArgs._OnServiceStopped = InArgs._OnServiceStopped;

    Super::Construct(ParentArgs, ConstructUnderMajorTab, ConstructUnderWindow, InFlowAsset);
}

TSharedPtr<IFlowProcessDomainExtender> FGridFlowTestRunnerTask::CreateDomainExtender(const FGridFlowTestRunnerSettings& InSettings) {
    return MakeShareable(new FGridFlowProcessDomainExtender);
}

#undef LOCTEXT_NAMESPACE

