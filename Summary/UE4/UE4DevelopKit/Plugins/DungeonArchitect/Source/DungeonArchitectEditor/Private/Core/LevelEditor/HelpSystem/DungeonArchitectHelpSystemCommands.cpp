//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/LevelEditor/HelpSystem/DungeonArchitectHelpSystemCommands.h"

#include "Core/LevelEditor/HelpSystem/DungeonArchitectHelpSystemStyle.h"

#include "Framework/Commands/Commands.h"
#include "Framework/Commands/InputChord.h"
#include "Framework/Commands/UICommandInfo.h"

#define LOCTEXT_NAMESPACE "ContentBrowser"

FDungeonArchitectHelpSystemCommands::FDungeonArchitectHelpSystemCommands() : TCommands<
    FDungeonArchitectHelpSystemCommands>(
    TEXT("DungeonArchitectHelpSystem"),
    NSLOCTEXT("DungeonArchitectHelpSystem", "DungeonArchitectHelpSystem", "Dungeon Architect Help System"),
    NAME_None,
    FDungeonArchitectHelpSystemStyle::GetStyleSetName()) {
}

void FDungeonArchitectHelpSystemCommands::RegisterCommands() {
    UI_COMMAND(OpenDocumentationUserGuide, "User Guide", "Opens the Dungeon Architect User Guide",
               EUserInterfaceActionType::Button, FInputChord());
    UI_COMMAND(OpenDocumentationQuickStartGuide, "Quick Start Guide", "Opens the Dungeon Architect Quick Start Guide",
               EUserInterfaceActionType::Button, FInputChord());
    UI_COMMAND(OpenDocumentationVideoTutorials, "Video Tutorials", "Opens the Dungeon Architect Video Tutorials",
               EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE

