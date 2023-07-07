//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/FlowEditor/Panels/Viewport/SFlowPreview3DViewportToolbar.h"

#include "Core/Editors/FlowEditor/Panels/Viewport/SFlowPreview3DViewport.h"

#include "EditorViewportCommands.h"
#include "LevelEditor.h"
#include "Modules/ModuleManager.h"
#include "SEditorViewportToolBarButton.h"
#include "SEditorViewportToolBarMenu.h"
#include "SEditorViewportViewMenu.h"
#include "Types/ISlateMetaData.h"

#define LOCTEXT_NAMESPACE "LAThemeEditorViewportToolBar"


void SFlowPreview3DViewportToolbar::Construct(const FArguments& InArgs) {
    Viewport = InArgs._Viewport;

    TSharedRef<SFlowPreview3DViewport> ViewportRef = Viewport.Pin().ToSharedRef();

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
					.Label(LOCTEXT("ShowMenuTitle", "Properties"))
					.Cursor(EMouseCursor::Default)
					.ParentToolBar(SharedThis(this))
					.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("EditorViewportToolBar.ShowMenu")))
					.OnGetMenuContent(this, &SFlowPreview3DViewportToolbar::GeneratePropertiesMenu)
                ]
                + SHorizontalBox::Slot()
                  .AutoWidth()
                  .Padding(ToolbarSlotPadding)
                [
                    SNew(SEditorViewportToolbarMenu)
					.Label(LOCTEXT("ShowCamMenuTitle", "Camera"))
					.Cursor(EMouseCursor::Default)
					.ParentToolBar(SharedThis(this))
					.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("EditorViewportToolBar.CameraMenu")))
					.OnGetMenuContent(this, &SFlowPreview3DViewportToolbar::GenerateCameraMenu)
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


TSharedRef<SWidget> SFlowPreview3DViewportToolbar::GeneratePropertiesMenu() const {
    const bool bInShouldCloseWindowAfterMenuSelection = true;
    FMenuBuilder PropertiesMenuBuilder(bInShouldCloseWindowAfterMenuSelection, Viewport.Pin()->GetCommandList());

    const FGridFlowEditorViewportCommands& PreviewViewportActions = FGridFlowEditorViewportCommands::Get();
    PropertiesMenuBuilder.BeginSection("LevelViewportViewportOptions2");

    PropertiesMenuBuilder.AddMenuEntry(PreviewViewportActions.ToggleDebugData);
    PropertiesMenuBuilder.AddMenuEntry(PreviewViewportActions.DisplayDungeonProperties);
    PropertiesMenuBuilder.EndSection();

    return PropertiesMenuBuilder.MakeWidget();
}


TSharedRef<SWidget> SFlowPreview3DViewportToolbar::GenerateCameraMenu() const {
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

TSharedPtr<FExtender> SFlowPreview3DViewportToolbar::GetViewMenuExtender() {
    FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
    TSharedPtr<FExtender> LevelEditorExtenders = LevelEditorModule.GetMenuExtensibilityManager()->GetAllExtenders();
    return LevelEditorExtenders;
}

void FGridFlowEditorViewportCommands::RegisterCommands() {
    UI_COMMAND(ToggleDebugData, "Toggle Debug Data", "Shows the debug data of the dungeon",
               EUserInterfaceActionType::Button, FInputChord());
    UI_COMMAND(DisplayDungeonProperties, "Dungeon Properties", "Shows the dungeon properties",
               EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE

