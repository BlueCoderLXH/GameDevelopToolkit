//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/SnapMapEditor/SnapMapEditorToolbar.h"

#include "Core/Common/DungeonArchitectCommands.h"
#include "Core/Editors/SnapMapEditor/SnapMapEditor.h"
#include "Core/Editors/SnapMapEditor/SnapMapEditorCommands.h"
#include "Core/LevelEditor/Customizations/DungeonArchitectStyle.h"

#include "EditorStyleSet.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSpacer.h"
#include "WorkflowOrientedApp/SModeWidget.h"

#define LOCTEXT_NAMESPACE "BehaviorTreeApplicationMode"

class SSnapMapEditorModeSeparator : public SBorder {
public:
    SLATE_BEGIN_ARGS(SSnapMapEditorModeSeparator) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArg) {
        SBorder::Construct(
            SBorder::FArguments()
            .BorderImage(FEditorStyle::GetBrush("BlueprintEditor.PipelineSeparator"))
            .Padding(0.0f)
        );
    }

    // SWidget interface
    virtual FVector2D ComputeDesiredSize(float) const override {
        const float Height = 20.0f;
        const float Thickness = 16.0f;
        return FVector2D(Thickness, Height);
    }

    // End of SWidget interface
};

void FSnapMapEditorToolbar::AddModesToolbar(TSharedPtr<FExtender> Extender) {
    check(FlowEditor.IsValid());
    TSharedPtr<FSnapMapEditor> FlowEditorPtr = FlowEditor.Pin();

    Extender->AddToolBarExtension(
        "Asset",
        EExtensionHook::After,
        FlowEditorPtr->GetToolkitCommands(),
        FToolBarExtensionDelegate::CreateSP(this, &FSnapMapEditorToolbar::FillModesToolbar));
}

void FSnapMapEditorToolbar::AddFlowDesignerToolbar(TSharedPtr<FExtender> Extender) {
    check(FlowEditor.IsValid());
    TSharedPtr<FSnapMapEditor> FlowEditorPtr = FlowEditor.Pin();

    TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
    ToolbarExtender->AddToolBarExtension("Asset", EExtensionHook::After, FlowEditorPtr->GetToolkitCommands(),
                                         FToolBarExtensionDelegate::CreateSP(
                                             this, &FSnapMapEditorToolbar::FillFlowDesignerToolbar));
    FlowEditorPtr->AddToolbarExtender(ToolbarExtender);
}

void FSnapMapEditorToolbar::AddFlowDebugToolbar(TSharedPtr<FExtender> Extender) {
    check(FlowEditor.IsValid());
    TSharedPtr<FSnapMapEditor> FlowEditorPtr = FlowEditor.Pin();

    TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
    ToolbarExtender->AddToolBarExtension("Asset", EExtensionHook::After, FlowEditorPtr->GetToolkitCommands(),
                                         FToolBarExtensionDelegate::CreateSP(
                                             this, &FSnapMapEditorToolbar::FillFlowDebugToolbar));
    FlowEditorPtr->AddToolbarExtender(ToolbarExtender);
}

void FSnapMapEditorToolbar::FillModesToolbar(FToolBarBuilder& ToolbarBuilder) {
    check(FlowEditor.IsValid());
    TSharedPtr<FSnapMapEditor> FlowEditorPtr = FlowEditor.Pin();


    TAttribute<FName> GetActiveMode(FlowEditorPtr.ToSharedRef(), &FSnapMapEditor::GetCurrentMode);
    FOnModeChangeRequested SetActiveMode = FOnModeChangeRequested::CreateSP(
        FlowEditorPtr.ToSharedRef(), &FSnapMapEditor::SetCurrentMode);

    // Left side padding
    FlowEditorPtr->AddToolbarWidget(SNew(SSpacer).Size(FVector2D(4.0f, 1.0f)));

    // Design Mode
    FlowEditorPtr->AddToolbarWidget(
        SNew(SModeWidget, FSnapMapEditor::GetLocalizedMode(FSnapMapEditor::GraphEditorModeID),
             FSnapMapEditor::GraphEditorModeID)
		.OnGetActiveMode(GetActiveMode)
		.OnSetActiveMode(SetActiveMode)
		.CanBeSelected(FlowEditorPtr.Get(), &FSnapMapEditor::CanAccessGraphEditMode)
		.ToolTipText(LOCTEXT("GraphEditorModeButtonTooltip", "Switch to Graph Grammar Design Mode"))
		.IconImage(FDungeonArchitectStyle::Get().GetBrush("DA.SnapEd.SwitchToDesignMode"))
		.SmallIconImage(FDungeonArchitectStyle::Get().GetBrush("DA.SnapEd.SwitchToDesignMode.Small"))
    );

    FlowEditorPtr->AddToolbarWidget(SNew(SSnapMapEditorModeSeparator));

    // Visualization Mode
    FlowEditorPtr->AddToolbarWidget(
        SNew(SModeWidget, FSnapMapEditor::GetLocalizedMode(FSnapMapEditor::VisualizationModeID),
             FSnapMapEditor::VisualizationModeID)
		.OnGetActiveMode(GetActiveMode)
		.OnSetActiveMode(SetActiveMode)
		.CanBeSelected(FlowEditorPtr.Get(), &FSnapMapEditor::CanAccessVisualizationMode)
		.ToolTipText(LOCTEXT("VisualizationModeButtonTooltip", "Switch to Visualization mode"))
		.IconImage(FDungeonArchitectStyle::Get().GetBrush("DA.SnapEd.SwitchToVisualizeMode"))
		.SmallIconImage(FDungeonArchitectStyle::Get().GetBrush("DA.SnapEd.SwitchToVisualizeMode.Small"))
    );

    FlowEditorPtr->AddToolbarWidget(SNew(SSnapMapEditorModeSeparator));

    // Debug Mode
    FlowEditorPtr->AddToolbarWidget(
        SNew(SModeWidget, FSnapMapEditor::GetLocalizedMode(FSnapMapEditor::DebugModeID),
             FSnapMapEditor::DebugModeID)
		.OnGetActiveMode(GetActiveMode)
		.OnSetActiveMode(SetActiveMode)
		.CanBeSelected(FlowEditorPtr.Get(), &FSnapMapEditor::CanAccessDebugMode)
		.ToolTipText(LOCTEXT("VisualizationModeButtonTooltip", "Switch to Debug mode"))
		.IconImage(FDungeonArchitectStyle::Get().GetBrush("DA.SnapEd.SwitchToDebugMode"))
		.SmallIconImage(FDungeonArchitectStyle::Get().GetBrush("DA.SnapEd.SwitchToDebugMode.Small"))
    );

    // Right side padding
    FlowEditorPtr->AddToolbarWidget(SNew(SSpacer).Size(FVector2D(4.0f, 1.0f)));

}

void FSnapMapEditorToolbar::FillFlowDesignerToolbar(FToolBarBuilder& ToolbarBuilder) {
    check(FlowEditor.IsValid());
    TSharedPtr<FSnapMapEditor> FlowEditorPtr = FlowEditor.Pin();

    if (FlowEditorPtr->GetCurrentMode() == FSnapMapEditor::GraphEditorModeID) {
        ToolbarBuilder.BeginSection("Build");
        {
            ToolbarBuilder.AddToolBarButton(FSnapMapEditorCommands::Get().BuildGraph);
        }
        ToolbarBuilder.EndSection();
        
        ToolbarBuilder.BeginSection("Flow Control");
        {
            ToolbarBuilder.AddToolBarButton(FSnapMapEditorCommands::Get().ValidateGrammarGraph);
            ToolbarBuilder.AddToolBarButton(FSnapMapEditorCommands::Get().Performance);
            ToolbarBuilder.AddToolBarButton(FSnapMapEditorCommands::Get().Settings);
        }
        ToolbarBuilder.EndSection();
    }
}


void FSnapMapEditorToolbar::FillFlowDebugToolbar(FToolBarBuilder& ToolbarBuilder) {
    check(FlowEditor.IsValid());
    TSharedPtr<FSnapMapEditor> FlowEditorPtr = FlowEditor.Pin();

    if (FlowEditorPtr->GetCurrentMode() == FSnapMapEditor::DebugModeID) {
        ToolbarBuilder.BeginSection("Flow Control");
        {
            ToolbarBuilder.AddToolBarButton(FSnapMapEditorCommands::Get().DebugRestart);
            ToolbarBuilder.AddToolBarButton(FSnapMapEditorCommands::Get().DebugStepForward);
        }
        ToolbarBuilder.EndSection();
    }
}

#undef LOCTEXT_NAMESPACE

