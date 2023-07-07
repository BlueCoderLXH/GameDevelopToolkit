//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/SnapConnectionEditor/SnapConnectionEditorCommands.h"

#include "Core/LevelEditor/Customizations/DungeonArchitectStyle.h"

#define LOCTEXT_NAMESPACE "FSnapConnectionEditorCommands"

FSnapConnectionEditorCommands::FSnapConnectionEditorCommands()
    : TCommands<FSnapConnectionEditorCommands>("SnapConnectionEditor",
                                                  NSLOCTEXT("Contexts", "SnapConnectionEditor",
                                                            "Snap Door Editor"), NAME_None,
                                                  FDungeonArchitectStyle::GetStyleSetName()) {\

}

void FSnapConnectionEditorCommands::RegisterCommands() {
    UI_COMMAND(Rebuild, "Rebuild", "Rebuilds the door", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE

