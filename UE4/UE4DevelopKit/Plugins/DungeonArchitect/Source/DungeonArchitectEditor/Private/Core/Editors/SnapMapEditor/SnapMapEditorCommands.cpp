//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/SnapMapEditor/SnapMapEditorCommands.h"

#include "Core/LevelEditor/Customizations/DungeonArchitectStyle.h"

#define LOCTEXT_NAMESPACE "SnapMapEditorCommands"

FSnapMapEditorCommands::FSnapMapEditorCommands() : TCommands<FSnapMapEditorCommands>(
    TEXT("SnapEd"),
    NSLOCTEXT("SnapEd", "SnapEd", "Dungeon Snap Editor"),
    NAME_None,
    FDungeonArchitectStyle::GetStyleSetName()) {
}

void FSnapMapEditorCommands::RegisterCommands() {
    UI_COMMAND(BuildGraph, "Build Graph", "Build the flow graph in the result panel", EUserInterfaceActionType::Button, FInputChord());
    UI_COMMAND(ValidateGrammarGraph, "Validate", "Validates the graph grammar for any errors (check the Error List tab)", EUserInterfaceActionType::Button, FInputChord());
    UI_COMMAND(Performance, "Performance", "Build this graph thousands of times to test the performance", EUserInterfaceActionType::Button, FInputChord());
    UI_COMMAND(Settings, "Settings", "Show editor Settings", EUserInterfaceActionType::Button, FInputChord());
    UI_COMMAND(DebugStepForward, "Step Forward", "Move one step forward in the build process", EUserInterfaceActionType::Button, FInputChord());
    UI_COMMAND(DebugRestart, "Restart", "Restart the build", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE

