//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/FlowEditor/BaseEditors/SnapGridFlowEditor.h"

#include "Builders/SnapGridFlow/SnapGridFlowAsset.h"
#include "Builders/SnapGridFlow/SnapGridFlowDungeon.h"
#include "Core/Dungeon.h"
#include "Core/Editors/FlowEditor/DomainEditors/FlowDomainEdAbstractGraph3D.h"
#include "Core/Editors/FlowEditor/DomainEditors/FlowDomainEdSnap.h"
#include "Core/Editors/FlowEditor/FlowEditorSettings.h"
#include "Core/Editors/FlowEditor/FlowTestRunner.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Implementations/GridFlowAbstractGraph3D.h"
#include "Frameworks/Flow/Domains/FlowDomain.h"
#include "Frameworks/Flow/Domains/Snap/SnapFlowAbstractGraphSupport.h"
#include "Frameworks/Snap/SnapGridFlow/SnapGridFlowLibrary.h"

#define LOCTEXT_NAMESPACE "SnapGridFlowEditor"

DEFINE_LOG_CATEGORY_STATIC(LogSnapGridFlowEditor, Log, All);

class UGridFlowConfig;

/////////////////// Snap Grid Flow Editor ///////////////////

void FSnapGridFlowEditor::InitDungeonConfig(UDungeonConfig* Config) {
    USnapGridFlowConfig* GridFlowConfig = Cast<USnapGridFlowConfig>(Config);
    if (GridFlowConfig) {
        GridFlowConfig->ParameterOverrides = EditorSettings->ParameterOverrides;
    }
}

void FSnapGridFlowEditor::CreateDomainEditors() {
    DomainEditors.Reset();
    DomainMediators.Reset();

    // Register the domain editors
    DomainEditors.Add(MakeShareable(new FFlowDomainEdAbstractGraph3D));
    DomainEditors.Add(MakeShareable(new FFlowDomainEdSnap));

    // Initialize the domain editors
    for (IFlowDomainEditorPtr DomainEditor : DomainEditors) {
        DomainEditor->Initialize(PropertyEditor);
    }
}

FName FSnapGridFlowEditor::GetFlowEdAppName() const {
    static const FName AppName = FName(TEXT("SnapGridFlowEditor"));
    return AppName;
}

ADungeon* FSnapGridFlowEditor::CreatePreviewDungeon(UWorld* World) {
    ADungeon* PreviewActor = World->SpawnActor<ADungeon>(FVector::ZeroVector, FQuat::Identity.Rotator());
    if (PreviewActor) {
        // TODO: Initialize the dungeon actor
        PreviewActor->SetBuilderClass(USnapGridFlowBuilder::StaticClass());
        USnapGridFlowConfig* GridFlowConfig = Cast<USnapGridFlowConfig>(PreviewActor->GetConfig());
        if (GridFlowConfig) {
            // Setup the grid flow graph asset to the currently edited asset
            GridFlowConfig->FlowGraph = Cast<USnapGridFlowAsset>(AssetBeingEdited);
        }
    }
    return PreviewActor;
}

FText FSnapGridFlowEditor::GetEditorBrandingText() const {
    return LOCTEXT("EditorBranding", "SNAP GRID FLOW");
}

UFlowEditorSettings* FSnapGridFlowEditor::CreateEditorSettingsObject() const {
    USnapGridFlowEditorSettings* NewSettings = NewObject<USnapGridFlowEditorSettings>();
    NewSettings->MaxBuildRetries = 100;
    return NewSettings;
}

TSharedRef<SWidget> FSnapGridFlowEditor::CreatePerfWidget(const TSharedRef<SDockTab> DockTab, TSharedPtr<SWindow> OwnerWindow) {
    PerfRunner = SNew(SSnapGridFlowTestRunner, DockTab, OwnerWindow, AssetBeingEdited)
            .OnServiceStarted(this, &FSnapGridFlowEditor::OnTestRunnerServiceStarted);

    return PerfRunner.ToSharedRef();
}

void FSnapGridFlowEditor::OnTestRunnerServiceStarted() {
    Super::OnTestRunnerServiceStarted();

    // Assign the module database to the perf runner
    if (PerfRunner.IsValid()) {
        USnapGridFlowEditorSettings* SGEditorSettings = Cast<USnapGridFlowEditorSettings>(EditorSettings);
        if (SGEditorSettings) {
            USnapGridFlowModuleDatabase* ModuleDatabase = SGEditorSettings ? SGEditorSettings->ModuleDatabase.LoadSynchronous() : nullptr;
            PerfRunner->SetModuleDatabase(ModuleDatabase);
        }
    }
}

bool FSnapGridFlowEditor::ShouldBuildPreviewDungeon() const {
    // Disable dungeon rebuilds till the snap editor stream-out issue is resolved
    // TODO: Fix me and enable this
    return false;
}

FName FSnapGridFlowEditor::GetToolkitFName() const {
    static const FName ToolkitName = FName("SnapGridFlowEditor");
    return ToolkitName;
}

FText FSnapGridFlowEditor::GetBaseToolkitName() const {
    return LOCTEXT("SnapGridFlowEditorAppLabel", "Grid Flow Editor");
}

FString FSnapGridFlowEditor::GetWorldCentricTabPrefix() const {
    return TEXT("SnapGridFlowEditor");
}

TSharedPtr<FTabManager::FLayout> FSnapGridFlowEditor::CreateFrameLayout() const {
    TSharedPtr<FTabManager::FLayout> Layout = FTabManager::NewLayout(ConstructLayoutName("0.3.2"))
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
                ->SetOrientation(Orient_Vertical)
                ->Split
                (

                    FTabManager::NewSplitter()
                    ->SetSizeCoefficient(0.35f)
                    ->SetOrientation(Orient_Horizontal)
                    ->Split // Exec Graph
                    (
                        FTabManager::NewStack()
                        ->SetSizeCoefficient(0.85f)
                        ->AddTab(FFlowEditorTabs::ExecGraphID, ETabState::OpenedTab)
                        ->SetHideTabWell(true)
                    )
                    ->Split // Details Tab
                    (
                        FTabManager::NewStack()
                        ->SetSizeCoefficient(0.15f)
                        ->AddTab(FFlowEditorTabs::DetailsID, ETabState::OpenedTab)
                    )
                )
                ->Split
                (
                    FTabManager::NewSplitter()
                    ->SetSizeCoefficient(0.65f)
                    ->SetOrientation(Orient_Horizontal)
                    ->Split // Domain Editors
                    (
                    CreateDomainEditorLayout()
                        ->SetSizeCoefficient(0.6f)
                    )
                    ->Split // Preview Viewport 3D / Performance
                    (
                        FTabManager::NewStack()
                        // ->AddTab(FFlowEditorTabs::ViewportID, ETabState::OpenedTab)
                        ->AddTab(FFlowEditorTabs::PerformanceID, ETabState::OpenedTab)
                        ->SetForegroundTab(FFlowEditorTabs::PerformanceID)
                        ->SetSizeCoefficient(0.4f)
                    )
                )
            )
        );

    return Layout;
}

void FSnapGridFlowEditor::ConfigureDomainObject(IFlowDomainPtr Domain) {
    if (!Domain.IsValid()) return;
    if (Domain->GetDomainID() == FGridFlowAbstractGraph3DDomain::DomainID) {
        // Configure the abstract 3d domain object from the editor settings
        USnapGridFlowEditorSettings* SGEditorSettings = Cast<USnapGridFlowEditorSettings>(EditorSettings);
        if (SGEditorSettings) {
            TSharedPtr<FGridFlowAbstractGraph3DDomain> GridAbstract3DDomain = StaticCastSharedPtr<FGridFlowAbstractGraph3DDomain>(Domain);

            // Setup the graph constraints
            USnapGridFlowModuleDatabase* ModuleDatabase = SGEditorSettings->ModuleDatabase.LoadSynchronous();
            if (ModuleDatabase) {
                GridAbstract3DDomain->SetGraphConstraints(MakeShareable(new FSnapGridFlowAbstractGraphConstraints(ModuleDatabase)));
                GridAbstract3DDomain->SetGroupGenerator(MakeShareable(new FSnapFlowAGNodeGroupGenerator(ModuleDatabase)));
            }
            else {
                GridAbstract3DDomain->SetGraphConstraints(MakeShareable(new FNullFlowAbstractGraphConstraints));
                GridAbstract3DDomain->SetGroupGenerator(MakeShareable(new FNullFlowAGNodeGroupGenerator));
            }
        }
        UE_LOG(LogSnapGridFlowEditor, Log, TEXT("Configuring abstract layout graph 3d domain object"));
    }
}


/////////////////// Snap Grid Flow Perf Editor ///////////////////

void SSnapGridFlowTestRunner::Construct(const FArguments& InArgs, const TSharedRef<SDockTab>& ConstructUnderMajorTab,
            const TSharedPtr<SWindow>& ConstructUnderWindow, TWeakObjectPtr<UFlowAssetBase> InFlowAsset) {
   
    Super::FArguments ParentArgs;
    ParentArgs._OnServiceStarted = InArgs._OnServiceStarted;
    ParentArgs._OnServiceStopped = InArgs._OnServiceStopped;

    Super::Construct(ParentArgs, ConstructUnderMajorTab, ConstructUnderWindow, InFlowAsset);
}

void SSnapGridFlowTestRunner::SetupSettingsObject(FSnapGridFlowTestRunnerSettings& OutSettings) {
    Super::SetupSettingsObject(OutSettings);
    
    OutSettings.ModuleDatabase = ModuleDatabase;
}


TSharedPtr<IFlowProcessDomainExtender> FSnapGridFlowTestRunnerTask::CreateDomainExtender(const FSnapGridFlowTestRunnerSettings& InSettings) {
    return MakeShareable(new FSnapGridFlowProcessDomainExtender(InSettings.ModuleDatabase));
}

void SSnapGridFlowTestRunner::SetModuleDatabase(TWeakObjectPtr<USnapGridFlowModuleDatabase> InModuleDatabase) {
    ModuleDatabase = InModuleDatabase;
}

#undef LOCTEXT_NAMESPACE

