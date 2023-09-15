//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/SnapMapEditor/AppModes/GraphDesignAppMode.h"

#include "Builders/SnapMap/SnapMapAsset.h"
#include "Core/Dungeon.h"
#include "Core/Editors/SnapMapEditor/SnapMapEditor.h"
#include "Core/Editors/SnapMapEditor/SnapMapEditorCommands.h"
#include "Core/Editors/SnapMapEditor/SnapMapEditorTabFactories.h"
#include "Core/Editors/SnapMapEditor/SnapMapEditorTabs.h"
#include "Core/Editors/SnapMapEditor/SnapMapEditorToolbar.h"
#include "Core/Editors/SnapMapEditor/SnapMapTestRunner.h"
#include "Frameworks/GraphGrammar/Editor/GraphGrammarValidation.h"
#include "Frameworks/GraphGrammar/Editor/SGrammarEditor.h"
#include "Frameworks/GraphGrammar/Editor/SGrammarErrorList.h"
#include "Frameworks/GraphGrammar/RuleGraph/EdGraph_Grammar.h"

#include "IDetailsView.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "Widgets/Docking/SDockTab.h"

////////////////////// FSnapMapEditor_GraphDesignAppMode //////////////////////

#define LOCTEXT_NAMESPACE "SnapMapEditor_GraphDesignAppMode"

DEFINE_LOG_CATEGORY_STATIC(SnapMapEditorDesign, Log, All);

FSnapMapEditor_GraphDesignAppMode::FSnapMapEditor_GraphDesignAppMode(
    TSharedPtr<class FSnapMapEditor> InFlowEditor)
    : FSnapMapEdAppModeBase(FSnapMapEditor::GraphEditorModeID)
      , FlowEditor(InFlowEditor) {
    Settings = NewObject<USnapMapEditor_GraphDesignAppModeSettings>();

    // Create the details property editor widget
    {
        FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(
            "PropertyEditor");
        const FDetailsViewArgs DetailsViewArgs(false, false, true, FDetailsViewArgs::HideNameArea, true, this);
        TSharedRef<IDetailsView> PropertyEditorRef = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
        PropertyEditor = PropertyEditorRef;
    }

    USnapMapAsset* AssetBeingEdited = InFlowEditor->GetAssetBeingEdited();
    ResultGraph = NewObject<UEdGraph_Grammar>();
    ExecutionGraphEditor.Initialize(ResultGraph, AssetBeingEdited->MissionGrammar, PropertyEditor, Settings);
    ResultGraphEditor.Initialize(ResultGraph);
  
    ExecutionGraphPanel = SNew(SVerticalBox)
        + SVerticalBox::Slot()
          .AutoHeight()
          .HAlign(HAlign_Fill)
          .Padding(0, 0, 0, 0)
        [
            ExecutionGraphEditor.CreateToolWidget()
        ]
        + SVerticalBox::Slot()
        .FillHeight(1.0f)
        [
            ExecutionGraphEditor.GetContent()
        ];


    this->GrammarEditor = SNew(SGrammarEditor, AssetBeingEdited->MissionGrammar)
        .PropertyEditor(PropertyEditor)
        .OnGrammarStateChanged_Raw(this, &FSnapMapEditor_GraphDesignAppMode::OnGrammarStateChanged);

    // Create the error list view
    ErrorListView = SNew(SGrammarErrorList)
        .GrammarEditor(GrammarEditor);

    BindCommands(InFlowEditor->GetToolkitCommands());

    bRequestValidation = true;
    FlowEditorTabFactories.RegisterFactory(MakeShareable(new FSnapMapEditorTabFactory_GrammarEditor(InFlowEditor, GrammarEditor)));
    FlowEditorTabFactories.RegisterFactory(MakeShareable(new FSnapMapEditorTabFactory_ExecutionGraph(InFlowEditor, ExecutionGraphPanel)));
    FlowEditorTabFactories.RegisterFactory(MakeShareable(new FSnapMapEditorTabFactory_ResultGraph(InFlowEditor, ResultGraphEditor.GetGraphWidget())));
    FlowEditorTabFactories.RegisterFactory(MakeShareable(new FSnapMapEditorTabFactory_Details(InFlowEditor, PropertyEditor)));
    FlowEditorTabFactories.RegisterFactory(MakeShareable(new FSnapMapEditorTabFactory_ErrorList(InFlowEditor, ErrorListView)));
    FlowEditorTabFactories.RegisterFactory(MakeShareable(new FSnapMapEditorTabFactory_Performance(InFlowEditor, nullptr)));

    TabLayout = BuildEditorFrameLayout(InFlowEditor);

    InFlowEditor->GetToolbarBuilder()->AddModesToolbar(ToolbarExtender);
    InFlowEditor->GetToolbarBuilder()->AddFlowDesignerToolbar(ToolbarExtender);
}

TSharedRef<SDockTab> FSnapMapEditorTabFactory_Performance::SpawnTab(const FWorkflowTabSpawnInfo& Info) const {
    // Spawn the tab
    TSharedRef<SDockTab> NewTab = SNew(SDockTab)
        .TabRole(this->TabRole)
        .Icon(GetTabIcon(Info))
        .Label(ConstructTabName(Info))
        .ShouldAutosize(bShouldAutosize)
    [
        SNullWidget::NullWidget
    ];

    const TSharedPtr<FSnapMapEditor> FlowEditor = FlowEditorPtr.Pin();
    if (FlowEditor.IsValid()) {
        USnapMapAsset* FlowAsset = FlowEditor->GetAssetBeingEdited();
        TSharedPtr<SWindow> Window = NewTab->GetParentWindow();
        TSharedPtr<SWidget> Widget = SNew(SSnapMapTestRunner, NewTab, Window, FlowAsset);
        NewTab->SetContent(Widget.ToSharedRef());
    }

    NewTab->SetTabToolTipWidget(CreateTabToolTipWidget(Info));
    return NewTab;
}

void FSnapMapEditor_GraphDesignAppMode::RegisterTabFactories(TSharedPtr<class FTabManager> InTabManager) {
    check(FlowEditor.IsValid());
    TSharedPtr<FSnapMapEditor> FlowEditorPtr = FlowEditor.Pin();

    FlowEditorPtr->RegisterToolbarTab(InTabManager.ToSharedRef());

    FlowEditorPtr->PushTabFactories(FlowEditorTabFactories);

    FApplicationMode::RegisterTabFactories(InTabManager);
}

void FSnapMapEditor_GraphDesignAppMode::PreDeactivateMode() {
    FApplicationMode::PreDeactivateMode();

    check(FlowEditor.IsValid());
    TSharedPtr<FSnapMapEditor> FlowEditorPtr = FlowEditor.Pin();

}

void FSnapMapEditor_GraphDesignAppMode::PostActivateMode() {
    check(FlowEditor.IsValid());
    TSharedPtr<FSnapMapEditor> FlowEditorPtr = FlowEditor.Pin();

    FApplicationMode::PostActivateMode();
}

TSharedRef<FTabManager::FLayout> FSnapMapEditor_GraphDesignAppMode::BuildEditorFrameLayout(
    TSharedPtr<class FSnapMapEditor> InFlowEditor) {
    return FTabManager::NewLayout("Standalone_DungeonFlowEditor_DesignLayout_v0.1.0")
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
                ->SetOrientation(Orient_Horizontal)
                // Left side of the editor
                ->Split
                (
                    FTabManager::NewStack()
                    ->SetSizeCoefficient(0.4f)
                    ->AddTab(FSnapMapEditorTabs::GrammarEditorID, ETabState::OpenedTab)
                )
                // Right side of the editor
                ->Split
                (
                    FTabManager::NewSplitter()
                    ->SetOrientation(Orient_Vertical)
                    ->SetSizeCoefficient(0.6f)

                    // Top side 
                    ->Split
                    (
                        FTabManager::NewSplitter()
                        ->SetOrientation(Orient_Vertical)

                        // The Execution Graph
                        ->Split
                        (
                            FTabManager::NewStack()
                            ->SetSizeCoefficient(0.5f)
                            ->AddTab(FSnapMapEditorTabs::ExecutionGraphID, ETabState::OpenedTab)
                        )

                        // The result graph
                        ->Split
                        (
                            FTabManager::NewStack()
                            ->SetSizeCoefficient(0.5f)
                            ->AddTab(FSnapMapEditorTabs::ResultGraphID, ETabState::OpenedTab)
                        )
                    )

                    // Bottom side
                    ->Split
                    (
                        FTabManager::NewSplitter()
                        ->SetOrientation(Orient_Horizontal)
                        ->SetSizeCoefficient(0.25f)
                        ->Split
                        (
                            FTabManager::NewStack()
                            ->SetSizeCoefficient(0.25f)
                            ->AddTab(FSnapMapEditorTabs::DetailsID, ETabState::OpenedTab)
                        )
                        ->Split
                        (
                            FTabManager::NewStack()
                            ->SetSizeCoefficient(0.75f)
                            ->AddTab(FSnapMapEditorTabs::ErrorListID, ETabState::OpenedTab)
                        )
                    )
                )
            )
        );
}

void FSnapMapEditor_GraphDesignAppMode::BindCommands(TSharedRef<FUICommandList> ToolkitCommands) {
    const FSnapMapEditorCommands& Commands = FSnapMapEditorCommands::Get();

    ToolkitCommands->MapAction(
        Commands.ValidateGrammarGraph,
        FExecuteAction::CreateRaw(this, &FSnapMapEditor_GraphDesignAppMode::OnClick_ValidateGrammar));

    ToolkitCommands->MapAction(
        Commands.BuildGraph,
        FExecuteAction::CreateRaw(this, &FSnapMapEditor_GraphDesignAppMode::OnClick_ExecuteDesignGraph));

    ToolkitCommands->MapAction(
        Commands.Settings,
        FExecuteAction::CreateRaw(this, &FSnapMapEditor_GraphDesignAppMode::OnClick_Settings));
    
    ToolkitCommands->MapAction(
        Commands.Performance,
        FExecuteAction::CreateRaw(this, &FSnapMapEditor_GraphDesignAppMode::OnClick_Performance));
}

void FSnapMapEditor_GraphDesignAppMode::OnClick_ExecuteDesignGraph() {
    UE_LOG(SnapMapEditorDesign, Log, TEXT("Executing graph"));
    ExecutionGraphEditor.ExecuteResultGraph();
    if (Settings && Settings->bAutoZoomResultGraph) {
        ResultGraphEditor.ZoomToFit();
    }
    FlashEditorTab(FSnapMapEditorTabs::ResultGraphID);
}

void FSnapMapEditor_GraphDesignAppMode::OnClick_Settings() {
    if (PropertyEditor.IsValid()) {
        PropertyEditor->SetObject(Settings);
        FlashEditorTab(FSnapMapEditorTabs::DetailsID);
    }
}

void FSnapMapEditor_GraphDesignAppMode::OnClick_Performance() {
    
    UE_LOG(SnapMapEditorDesign, Log, TEXT("Opening perf stat dialog"));
    
    TSharedPtr<FSnapMapEditor> FlowEditorPtr = FlowEditor.Pin();
    if (FlowEditorPtr.IsValid()) {
        TSharedPtr<FTabManager> TabManager = FlowEditorPtr->GetTabManager();
        if (TabManager.IsValid()) {
            TabManager->TryInvokeTab(FSnapMapEditorTabs::PerformanceID);
        }
    }
}

void FSnapMapEditor_GraphDesignAppMode::OnClick_ValidateGrammar() {
    ValidateGrammar();
    FlashEditorTab(FSnapMapEditorTabs::ErrorListID);
}

void FSnapMapEditor_GraphDesignAppMode::FlashEditorTab(const FName& InTabId) const {
    TSharedPtr<FSnapMapEditor> FlowEditorPtr = FlowEditor.Pin();
    if (FlowEditorPtr.IsValid()) {
        const TSharedPtr<FTabManager> TabManager = FlowEditorPtr->GetTabManager();
        if (TabManager.IsValid()) {
            TSharedPtr<SDockTab> Tab = TabManager->FindExistingLiveTab(InTabId);
            if (Tab.IsValid()) {
                Tab->FlashTab();
            }
        }
    }
}

void FSnapMapEditor_GraphDesignAppMode::ValidateGrammar() {
    check(FlowEditor.IsValid());
    TSharedPtr<FSnapMapEditor> FlowEditorPtr = FlowEditor.Pin();
    USnapMapAsset* AssetBeingEdited = FlowEditorPtr->GetAssetBeingEdited();

    if (AssetBeingEdited) {
        FGrammarValidationResult ValidationResult = FGrammarValidator::Validate(AssetBeingEdited->MissionGrammar);
        if (!ValidationResult.ContainsErrors() && !ValidationResult.ContainsWarnings()) {
            // Add a success entry
            ValidationResult.Entries.Add(
                MakeShareable(
                    new FGrammarValidationEntry(EGrammarLogType::Success, FText::GetEmpty(),
                                                nullptr)));
        }
        ErrorListView->UpdateList(ValidationResult.Entries);
    }
}


void FSnapMapEditor_GraphDesignAppMode::AddReferencedObjects(FReferenceCollector& Collector) {
    if (ResultGraph) {
        Collector.AddReferencedObject(ResultGraph);
    }
    if (Settings != nullptr) {
        Collector.AddReferencedObject(Settings);
    }
}

void FSnapMapEditor_GraphDesignAppMode::Tick(float DeltaTime) {
    if (bRequestValidation) {
        bRequestValidation = false;
        ValidateGrammar();
    }
}

void FSnapMapEditor_GraphDesignAppMode::OnGrammarStateChanged() {
    bRequestValidation = true;
}

void FSnapMapEditor_GraphDesignAppMode::NotifyPreChange(FProperty* PropertyAboutToChange) {

}

void FSnapMapEditor_GraphDesignAppMode::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent,
                                                             FProperty* PropertyThatChanged) {
    OnGrammarStateChanged();
}

#undef  LOCTEXT_NAMESPACE

