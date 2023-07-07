//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/SnapMapEditor/Viewport/SSnapMapEditorViewportToolbar.h"

#include "Core/Editors/SnapMapEditor/Viewport/SSnapMapEditorViewport.h"

#include "EditorViewportCommands.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "LevelEditor.h"
#include "Modules/ModuleManager.h"
#include "SEditorViewport.h"
#include "SEditorViewportToolBarButton.h"
#include "SEditorViewportToolBarMenu.h"
#include "SEditorViewportViewMenu.h"
#include "Types/ISlateMetaData.h"
#include "Widgets/Layout/SBorder.h"

#define LOCTEXT_NAMESPACE "DungeonFlowEditorViewportToolBar"


void SSnapMapEditorViewportToolBar::Construct(const FArguments& InArgs) {
    Viewport = InArgs._Viewport;

    TSharedRef<SSnapMapEditorViewport> ViewportRef = Viewport.Pin().ToSharedRef();

    FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");

    const FMargin ToolbarSlotPadding(2.0f, 2.0f);
    const FMargin ToolbarButtonPadding(2.0f, 0.0f);

    static const FName DefaultForegroundName("DefaultForeground");

    ChildSlot
    [
        SNew(SBorder)
		.BorderImage(FEditorStyle::GetBrush("NoBorder"))
		// Color and opacity is changed based on whether or not the mouse cursor is hovering over the toolbar area
		.ColorAndOpacity(this, &SViewportToolBar::OnGetColorAndOpacity)
		.ForegroundColor(FEditorStyle::GetSlateColor(DefaultForegroundName))
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SHorizontalBox)

                + SHorizontalBox::Slot()
                  .AutoWidth()
                  .Padding(ToolbarSlotPadding)
                [
                    SNew(SEditorViewportToolbarMenu)
					.Image("EditorViewportToolBar.MenuDropdown")
					.Cursor(EMouseCursor::Default)
					.ParentToolBar(SharedThis(this))
					.OnGetMenuContent(this, &SSnapMapEditorViewportToolBar::GenerateActionsMenu)
                ]

                /*
                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(ToolbarSlotPadding)
                [
                    SNew(SEditorViewportToolbarMenu)
                    .Label(LOCTEXT("ShowMenuTitle", "Properties"))
                    .Cursor(EMouseCursor::Default)
                    .ParentToolBar(SharedThis(this))
                    .AddMetaData<FTagMetaData>(FTagMetaData(TEXT("EditorViewportToolBar.ShowMenu")))
                    .OnGetMenuContent(this, &SDungeonFlowEditorViewportToolBar::GeneratePropertiesMenu)
                ]
                */

                + SHorizontalBox::Slot()
                  .AutoWidth()
                  .Padding(ToolbarSlotPadding)
                [
                    SNew(SEditorViewportToolbarMenu)
					.Label(LOCTEXT("ShowCamMenuTitle", "Camera"))
					.Cursor(EMouseCursor::Default)
					.ParentToolBar(SharedThis(this))
					.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("EditorViewportToolBar.CameraMenu")))
					.OnGetMenuContent(this, &SSnapMapEditorViewportToolBar::GenerateCameraMenu)
                ]
                + SHorizontalBox::Slot()
                  .AutoWidth()
                  .Padding(ToolbarSlotPadding)
                [
                    SNew(SEditorViewportViewMenu, ViewportRef, SharedThis(this))
					.Cursor(EMouseCursor::Default)
					.MenuExtenders(GetViewMenuExtender())
					.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("ViewMenuButton")))
                ]
            ]
        ]
    ];

    SViewportToolBar::Construct(SViewportToolBar::FArguments());
}


TSharedRef<SWidget> SSnapMapEditorViewportToolBar::GeneratePropertiesMenu() const {
    const bool bInShouldCloseWindowAfterMenuSelection = true;
    FMenuBuilder PropertiesMenuBuilder(bInShouldCloseWindowAfterMenuSelection, Viewport.Pin()->GetCommandList());

    const FSnapMapEditorViewportCommands& PreviewViewportActions = FSnapMapEditorViewportCommands::Get();
    PropertiesMenuBuilder.BeginSection("SnapMapViewportOptions");

    //PropertiesMenuBuilder.AddMenuEntry(PreviewViewportActions.ShowPropertyDungeon);
    PropertiesMenuBuilder.EndSection();

    return PropertiesMenuBuilder.MakeWidget();
}


TSharedRef<SWidget> SSnapMapEditorViewportToolBar::GenerateActionsMenu() const {

    const bool bInShouldCloseWindowAfterMenuSelection = true;
    FMenuBuilder PropertiesMenuBuilder(bInShouldCloseWindowAfterMenuSelection, Viewport.Pin()->GetCommandList());
    {
        PropertiesMenuBuilder.BeginSection("SnapMapViewportStatsOptions");
        {
            PropertiesMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().ToggleRealTime);
            PropertiesMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().ToggleStats);
            PropertiesMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().ToggleFPS);
        }
        PropertiesMenuBuilder.EndSection();
    }

    return PropertiesMenuBuilder.MakeWidget();
}

TSharedRef<SWidget> SSnapMapEditorViewportToolBar::GenerateCameraMenu() const {
    const bool bInShouldCloseWindowAfterMenuSelection = true;
    FMenuBuilder CameraMenuBuilder(bInShouldCloseWindowAfterMenuSelection, Viewport.Pin()->GetCommandList());

    // Camera types
    CameraMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Perspective);

    CameraMenuBuilder.BeginSection("LevelViewportCameraType_Ortho", LOCTEXT("CameraTypeHeader_Ortho", "Orthographic"));
    CameraMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Top);
    CameraMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Bottom);
    CameraMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Left);
    CameraMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Right);
    CameraMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Front);
    CameraMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Back);
    CameraMenuBuilder.EndSection();

    return CameraMenuBuilder.MakeWidget();
}

TSharedPtr<FExtender> SSnapMapEditorViewportToolBar::GetViewMenuExtender() {
    FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
    TSharedPtr<FExtender> LevelEditorExtenders = LevelEditorModule.GetMenuExtensibilityManager()->GetAllExtenders();
    return LevelEditorExtenders;
}

void FSnapMapEditorViewportCommands::RegisterCommands() {
    //UI_COMMAND(ShowPropertyDungeon, "Dungeon", "Shows Dungeon Properties", EUserInterfaceActionType::Button, FInputChord());

}

#undef LOCTEXT_NAMESPACE

