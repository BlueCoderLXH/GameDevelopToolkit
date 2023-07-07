//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/ThemeEditor/Widgets/SDungeonEditorViewportToolbar.h"

#include "Core/Editors/ThemeEditor/Widgets/SDungeonEditorViewport.h"

#include "EditorViewportCommands.h"
#include "LevelEditor.h"
#include "SEditorViewportToolBarButton.h"
#include "SEditorViewportToolBarMenu.h"
#include "SEditorViewportViewMenu.h"
#include "Types/ISlateMetaData.h"

#define LOCTEXT_NAMESPACE "DungeonEditorViewportToolBar"


void SDungeonEditorViewportToolBar::Construct(const FArguments& InArgs) {
    Viewport = InArgs._Viewport;

    TSharedRef<SDungeonEditorViewport> ViewportRef = Viewport.Pin().ToSharedRef();

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
					.OnGetMenuContent(this, &SDungeonEditorViewportToolBar::GenerateActionsMenu)
                ]

                + SHorizontalBox::Slot()
                  .AutoWidth()
                  .Padding(ToolbarSlotPadding)
                [
                    SNew(SEditorViewportToolbarMenu)
					.Label(LOCTEXT("ShowMenuTitle", "Properties"))
					.Cursor(EMouseCursor::Default)
					.ParentToolBar(SharedThis(this))
					.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("EditorViewportToolBar.ShowMenu")))
					.OnGetMenuContent(this, &SDungeonEditorViewportToolBar::GeneratePropertiesMenu)
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
					.OnGetMenuContent(this, &SDungeonEditorViewportToolBar::GenerateCameraMenu)
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


TSharedRef<SWidget> SDungeonEditorViewportToolBar::GeneratePropertiesMenu() const {
    const bool bInShouldCloseWindowAfterMenuSelection = true;
    FMenuBuilder PropertiesMenuBuilder(bInShouldCloseWindowAfterMenuSelection, Viewport.Pin()->GetCommandList());

    const FDungeonEditorViewportCommands& PreviewViewportActions = FDungeonEditorViewportCommands::Get();
    PropertiesMenuBuilder.BeginSection("LevelViewportViewportOptions2");

    PropertiesMenuBuilder.AddMenuEntry(PreviewViewportActions.ShowPropertyDungeon);
    PropertiesMenuBuilder.AddMenuEntry(PreviewViewportActions.ShowPropertySkylight);
    PropertiesMenuBuilder.AddMenuEntry(PreviewViewportActions.ShowPropertyDirectionalLight);
    PropertiesMenuBuilder.AddMenuEntry(PreviewViewportActions.ShowPropertyAtmosphericFog);
    PropertiesMenuBuilder.AddMenuEntry(PreviewViewportActions.ToggleDebugData);
    PropertiesMenuBuilder.EndSection();

    return PropertiesMenuBuilder.MakeWidget();
}


TSharedRef<SWidget> SDungeonEditorViewportToolBar::GenerateActionsMenu() const {

    const bool bInShouldCloseWindowAfterMenuSelection = true;
    FMenuBuilder PropertiesMenuBuilder(bInShouldCloseWindowAfterMenuSelection, Viewport.Pin()->GetCommandList());

    const FDungeonEditorViewportCommands& PreviewViewportActions = FDungeonEditorViewportCommands::Get();
    PropertiesMenuBuilder.BeginSection("DungeonViewportOptions");

    PropertiesMenuBuilder.AddMenuEntry(PreviewViewportActions.ForceRebuildPreviewLayout);
    PropertiesMenuBuilder.AddMenuEntry(PreviewViewportActions.DrawDebugData);
    PropertiesMenuBuilder.EndSection();

    return PropertiesMenuBuilder.MakeWidget();
}

TSharedRef<SWidget> SDungeonEditorViewportToolBar::GenerateCameraMenu() const {
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

TSharedPtr<FExtender> SDungeonEditorViewportToolBar::GetViewMenuExtender() {
    FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
    TSharedPtr<FExtender> LevelEditorExtenders = LevelEditorModule.GetMenuExtensibilityManager()->GetAllExtenders();
    return LevelEditorExtenders;
}

void FDungeonEditorViewportCommands::RegisterCommands() {
    UI_COMMAND(ShowPropertyDungeon, "Dungeon", "Shows Dungeon Properties", EUserInterfaceActionType::Button,
               FInputChord());
    UI_COMMAND(ShowPropertySkylight, "Skylight", "Shows Skylight Properties", EUserInterfaceActionType::Button,
               FInputChord());
    UI_COMMAND(ShowPropertyDirectionalLight, "Directional Light", "Shows Directional Light Properties",
               EUserInterfaceActionType::Button, FInputChord());
    UI_COMMAND(ShowPropertyAtmosphericFog, "Atmospheric Fog", "Shows Atmospheric Fog Properties",
               EUserInterfaceActionType::Button, FInputChord());
    UI_COMMAND(ToggleDebugData, "Toggle Debug Data", "Shows the debug data of the dungeon",
               EUserInterfaceActionType::Button, FInputChord());

    UI_COMMAND(ForceRebuildPreviewLayout, "Force Rebuild Preview Layout",
               "Does a clean rebuild of the dungeon whenever it changes. This might make the preview rebuild slower",
               EUserInterfaceActionType::ToggleButton, FInputChord());
    UI_COMMAND(DrawDebugData, "Draw Debug Data", "Shows the debug data of the dungeon",
               EUserInterfaceActionType::ToggleButton, FInputChord());

}

#undef LOCTEXT_NAMESPACE

