//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Common/DungeonArchitectCommands.h"

#include "Core/LevelEditor/Customizations/DungeonArchitectStyle.h"

#define LOCTEXT_NAMESPACE "DungeonArchitectCommands"

FDungeonArchitectCommands::FDungeonArchitectCommands() : TCommands<FDungeonArchitectCommands>(
    TEXT("DungeonArchitect"),
    NSLOCTEXT("DungeonArchitect", "DungeonArchitect", "Dungeon Architect"),
    NAME_None,
    FDungeonArchitectStyle::GetStyleSetName()) {
}

void FDungeonArchitectCommands::RegisterCommands() {
    UI_COMMAND(OpenDungeonEditor, "Open Dungeon Editor",
               "Opens the dungeon editor that lets you define props meshes for the dungeons",
               EUserInterfaceActionType::Button, FInputChord(EKeys::Enter));

    UI_COMMAND(OpenHelpSystem, "Help / Support", "Opens the Dungeon Architect Help System",
               EUserInterfaceActionType::Button, FInputChord(EKeys::F1));

    UI_COMMAND(ModePaint, "Paint", "Paint", EUserInterfaceActionType::ToggleButton, FInputChord());
    UI_COMMAND(ModeRectangle, "Rectangle", "Rectangle", EUserInterfaceActionType::ToggleButton, FInputChord());
    UI_COMMAND(ModeBorder, "Border", "Border", EUserInterfaceActionType::ToggleButton, FInputChord());
    UI_COMMAND(ModeSelect, "Select", "Select", EUserInterfaceActionType::ToggleButton, FInputChord());

    UI_COMMAND(UpgradeThemeFile, "Upgrade Theme File", "Upgrade Theme File", EUserInterfaceActionType::Button,
               FInputChord());
}

#undef LOCTEXT_NAMESPACE

