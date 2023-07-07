//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/LaunchPad/LaunchPad.h"

#include "Core/Editors/LaunchPad/Widgets/SLaunchPad.h"
#include "Core/LevelEditor/Customizations/DungeonArchitectStyle.h"

#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

#define LOCTEXT_NAMESPACE "LaunchPadSystem"

namespace {
    const FName DALaunchPadWindowID = FName(TEXT("DALaunchPadApp"));

    TSharedRef<class SDockTab> SpawnLaunchPadTab(const FSpawnTabArgs& Args) {
        return SNew(SDockTab)
            .TabRole(NomadTab)
            [
                SNew(SLaunchPad)
            ];
    }
}

void FLaunchPadSystem::Register() {
    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
                                DALaunchPadWindowID,
                                FOnSpawnTab::CreateStatic(&SpawnLaunchPadTab))
                            .SetDisplayName(LOCTEXT("TabTitle", "Launch Pad - Dungeon Architect"))
                            .SetTooltipText(LOCTEXT("TooltipText", "Browse Samples and Templates"))
                            .SetGroup(WorkspaceMenu::GetMenuStructure().GetLevelEditorCategory())
                            .SetIcon(FSlateIcon(FDungeonArchitectStyle::GetStyleSetName(),
                                                "DungeonArchitect.ToolbarItem.IconLaunchPad"));
}

void FLaunchPadSystem::Unregister() {
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(DALaunchPadWindowID);
}

void FLaunchPadSystem::Launch() {
    FGlobalTabmanager::Get()->TryInvokeTab(DALaunchPadWindowID);
}

#undef LOCTEXT_NAMESPACE

