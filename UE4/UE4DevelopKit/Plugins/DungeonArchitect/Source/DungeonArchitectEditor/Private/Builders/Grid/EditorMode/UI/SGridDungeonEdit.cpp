//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/Grid/EditorMode/UI/SGridDungeonEdit.h"

#include "Builders/Grid/EditorMode/GridDungeonEdModeHandler.h"
#include "Builders/Grid/EditorMode/Tools/GridDungeonEdToolBorder.h"
#include "Builders/Grid/EditorMode/Tools/GridDungeonEdToolFactory.h"
#include "Builders/Grid/EditorMode/Tools/GridDungeonEdToolPaint.h"
#include "Builders/Grid/EditorMode/Tools/GridDungeonEdToolRectangle.h"
#include "Core/Common/DungeonArchitectCommands.h"
#include "Core/LevelEditor/EditorMode/DungeonEdMode.h"
#include "Core/LevelEditor/EditorMode/DungeonEdModeHandler.h"

#include "Editor.h"
#include "EditorModeManager.h"
#include "EditorStyleSet.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/MultiBox/MultiBoxDefs.h"
#include "IDetailsView.h"
#include "PropertyEditorModule.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"

void SGridDungeonEdit::Construct(const FArguments& InArgs) {
    // Everything (or almost) uses this padding, change it to expand the padding.
    FMargin StandardPadding(0.0f, 4.0f, 0.0f, 4.0f);

    UICommandList = MakeShareable(new FUICommandList);

    BindCommands();

    FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(
        "PropertyEditor");
    FDetailsViewArgs DetailsViewArgs(false, false, false, FDetailsViewArgs::HideNameArea);

    DetailsPanel = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
    //DetailsPanel->SetIsPropertyVisibleDelegate(FIsPropertyVisible::CreateSP(this, &SGridDungeonEdit::GetIsPropertyVisible));

    TSharedRef<SWidget> DetailsPanelRef = DetailsPanel.ToSharedRef();

    this->ChildSlot
    [
        SNew(SOverlay)

        + SOverlay::Slot()
        [
            SNew(SScrollBox)
            + SScrollBox::Slot()
            .Padding(2.0f, 0)
            [
                SNew(SVerticalBox)

                + SVerticalBox::Slot()
                  .AutoHeight()
                  .Padding(StandardPadding)
                [
                    BuildToolBar()
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    DetailsPanelRef
                    //SNullWidget::NullWidget
                ]
            ]
        ]
    ];

    SwitchTool(FGridDungeonEdToolPaint::ToolID);
}

TSharedRef<SWidget> SGridDungeonEdit::BuildToolBar() {

    FToolBarBuilder Toolbar(UICommandList, FMultiBoxCustomization::None);
    {
        Toolbar.AddToolBarButton(FDungeonArchitectCommands::Get().ModePaint);
        Toolbar.AddToolBarButton(FDungeonArchitectCommands::Get().ModeRectangle);
        Toolbar.AddToolBarButton(FDungeonArchitectCommands::Get().ModeBorder);
        //Toolbar.AddToolBarButton(FDungeonArchitectCommands::Get().ModeSelect);
    }

    return
        SNew(SHorizontalBox)

        + SHorizontalBox::Slot()
        .Padding(4, 0)
        [
            SNew(SOverlay)
            + SOverlay::Slot()
            [
                SNew(SBorder)
				.HAlign(HAlign_Center)
				.Padding(0)
				.BorderImage(FEditorStyle::GetBrush("NoBorder"))
				.IsEnabled(FSlateApplication::Get().GetNormalExecutionAttribute())
                [
                    Toolbar.MakeWidget()
                ]
            ]
        ];
}

void SGridDungeonEdit::ClearAllToolSelection() {

}

void SGridDungeonEdit::BindCommands() {
    const FDungeonArchitectCommands& Commands = FDungeonArchitectCommands::Get();

    UICommandList->MapAction(
        Commands.ModePaint,
        FExecuteAction::CreateSP(this, &SGridDungeonEdit::OnToolSelectedPaint),
        FCanExecuteAction(),
        FIsActionChecked::CreateSP(this, &SGridDungeonEdit::IsToolActive_Paint));

    UICommandList->MapAction(
        Commands.ModeRectangle,
        FExecuteAction::CreateSP(this, &SGridDungeonEdit::OnToolSelectedRectangle),
        FCanExecuteAction(),
        FIsActionChecked::CreateSP(this, &SGridDungeonEdit::IsToolActive_Rectangle));

    UICommandList->MapAction(
        Commands.ModeBorder,
        FExecuteAction::CreateSP(this, &SGridDungeonEdit::OnToolSelectedBorder),
        FCanExecuteAction(),
        FIsActionChecked::CreateSP(this, &SGridDungeonEdit::IsToolActive_Border));

}

void SGridDungeonEdit::OnToolSelectedPaint() {
    SwitchTool(FGridDungeonEdToolPaint::ToolID);
}

void SGridDungeonEdit::OnToolSelectedRectangle() {
    SwitchTool(FGridDungeonEdToolRectangle::ToolID);
}

void SGridDungeonEdit::OnToolSelectedBorder() {
    SwitchTool(FGridDungeonEdToolBorder::ToolID);
}


bool SGridDungeonEdit::IsToolActive_Paint() {
    return IsToolActive(FGridDungeonEdToolPaint::ToolID);
}

bool SGridDungeonEdit::IsToolActive_Rectangle() {
    return IsToolActive(FGridDungeonEdToolRectangle::ToolID);
}

bool SGridDungeonEdit::IsToolActive_Border() {
    return IsToolActive(FGridDungeonEdToolBorder::ToolID);
}

UGridDungeonEdModeHandler* SGridDungeonEdit::GetModeHandler() {
    FEdModeDungeon* DungeonEditMode = StaticCast<FEdModeDungeon*>(
        GLevelEditorModeTools().GetActiveMode(FEdModeDungeon::EM_Dungeon));
    return Cast<UGridDungeonEdModeHandler>(DungeonEditMode->GetHandler());
}

bool SGridDungeonEdit::IsToolActive(FDungeonEdToolID ToolType) {
    UGridDungeonEdModeHandler* ModeHandler = GetModeHandler();
    if (!ModeHandler) return false;
    TSharedPtr<IDungeonEdTool> ActiveTool = ModeHandler->GetActiveTool();
    if (!ActiveTool.IsValid()) return false;
    return ActiveTool->GetToolType() == ToolType;
}

void SGridDungeonEdit::SwitchTool(FDungeonEdToolID ToolType) {
    UGridDungeonEdModeHandler* ModeHandler = GetModeHandler();
    if (!ModeHandler) return;
    TSharedPtr<IDungeonEdTool> ActiveTool = ModeHandler->GetActiveTool();
    if (ActiveTool.IsValid() && ActiveTool->GetToolType() == ToolType) {
        return;
    }

    ActiveTool = FGridDungeonEdToolFactory::Create(ToolType, ModeHandler);
    if (ActiveTool.IsValid()) {
        ModeHandler->SetActiveTool(ActiveTool);
        DetailsPanel->SetObject(ActiveTool->GetToolModel());
    }
}

void SGridDungeonEdit::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) {
}

bool SGridDungeonEdit::GetIsPropertyVisible(const struct FPropertyAndParent& PropertyAndParent) const {
    return true;
}

SGridDungeonEdit::~SGridDungeonEdit() {
}

