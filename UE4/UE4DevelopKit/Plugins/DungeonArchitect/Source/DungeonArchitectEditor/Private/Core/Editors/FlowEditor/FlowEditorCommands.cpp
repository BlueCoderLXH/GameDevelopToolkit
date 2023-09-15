//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/FlowEditor/FlowEditorCommands.h"

#include "Core/LevelEditor/Customizations/DungeonArchitectStyle.h"

#define LOCTEXT_NAMESPACE "GridFlowEditorCommands"


FFlowEditorCommands::FFlowEditorCommands() : TCommands<FFlowEditorCommands>(
    TEXT("FlowEditor"),
    NSLOCTEXT("FlowEditor", "FlowEditor", "Flow Editor"),
    NAME_None,
    FDungeonArchitectStyle::GetStyleSetName()) {
}

void FFlowEditorCommands::RegisterCommands() {
    UI_COMMAND(Build, "Build", "Build and Run the graph", EUserInterfaceActionType::Button, FInputChord());
    UI_COMMAND(Performance, "Performance", "Build this flow graph thousands of times to test the performance",
               EUserInterfaceActionType::Button, FInputChord());
    UI_COMMAND(ShowEditorSettings, "Editor Settings", "Show Editor Settings", EUserInterfaceActionType::Button,
               FInputChord());
    UI_COMMAND(ShowPreviewDungeonSettings, "Preview Dungeon Settings", "Show Preview Dungeon Settings",
               EUserInterfaceActionType::Button, FInputChord());
}


#undef LOCTEXT_NAMESPACE

