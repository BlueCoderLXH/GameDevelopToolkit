//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/LevelEditor/EditorMode/DungeonEdModeToolkit.h"

#include "Core/LevelEditor/EditorMode/DungeonEdMode.h"

#include "Editor.h"
#include "EditorModeManager.h"

#define LOCTEXT_NAMESPACE "DungeonEditMode"

void FDungeonEdModeToolkit::RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) {

}

void FDungeonEdModeToolkit::UnregisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) {

}

void FDungeonEdModeToolkit::Init(const TSharedPtr<class IToolkitHost>& InitToolkitHost) {
    FModeToolkit::Init(InitToolkitHost);
}

FName FDungeonEdModeToolkit::GetToolkitFName() const {
    return FName("DungeonEditMode");
}

FText FDungeonEdModeToolkit::GetBaseToolkitName() const {
    return LOCTEXT("ToolkitName", "Dungeon Edit Mode");
}

class FEdMode* FDungeonEdModeToolkit::GetEditorMode() const {
    return GLevelEditorModeTools().GetActiveMode(FEdModeDungeon::EM_Dungeon);
}

TSharedPtr<SWidget> FDungeonEdModeToolkit::GetInlineContent() const {
    return DungeonEdWidget;
}


void FDungeonEdModeToolkit::SetInlineContent(TSharedPtr<SWidget> Widget) {
    DungeonEdWidget = Widget;
}

#undef LOCTEXT_NAMESPACE

