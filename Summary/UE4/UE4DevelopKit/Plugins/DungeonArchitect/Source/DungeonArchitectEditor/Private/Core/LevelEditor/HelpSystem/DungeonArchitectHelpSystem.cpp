//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/LevelEditor/HelpSystem/DungeonArchitectHelpSystem.h"

#include "Core/LevelEditor/HelpSystem/DungeonArchitectHelpSystemCommands.h"
#include "Core/LevelEditor/HelpSystem/DungeonArchitectHelpSystemStyle.h"
#include "Core/LevelEditor/HelpSystem/Extenders/HelpMenuExtender.h"
#include "Core/LevelEditor/HelpSystem/Widgets/HelpSystemList.h"

#include "EditorStyleSet.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

#define LOCTEXT_NAMESPACE "DungeonArchitectHelpSystem"

namespace {
    const FName DAHelpSystemWindowID = FName(TEXT("DAHelpSystemApp"));
}

void FDungeonArchitectHelpSystem::Initialize() {
    FDungeonArchitectHelpSystemStyle::Initialize();
    FDungeonArchitectHelpSystemCommands::Register();
    MenuExtender = MakeShareable(new FDAHelpMenuExtender);
    MenuExtender->Extend();

    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
                                DAHelpSystemWindowID,
                                FOnSpawnTab::CreateRaw(this, &FDungeonArchitectHelpSystem::SpawnHelpSystemTab))
                            .SetDisplayName(
                                NSLOCTEXT("DAHelpSystemModule", "TabTitle", "Support - Dungeon Architect"))
                            .SetTooltipText(NSLOCTEXT("DAHelpSystemModule", "TooltipText",
                                                      "Dungeon Architect Help System"))
                            .SetGroup(WorkspaceMenu::GetMenuStructure().GetLevelEditorCategory())
                            .SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "CollisionAnalyzer.TabIcon"));
}

void FDungeonArchitectHelpSystem::Release() {
    MenuExtender->Release();
    MenuExtender = nullptr;

    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(DAHelpSystemWindowID);

    FDungeonArchitectHelpSystemStyle::Shutdown();
    FDungeonArchitectHelpSystemCommands::Unregister();
}

void FDungeonArchitectHelpSystem::LaunchHelpSystemTab() {
    FGlobalTabmanager::Get()->TryInvokeTab(DAHelpSystemWindowID);
}


TSharedRef<class SDockTab> FDungeonArchitectHelpSystem::SpawnHelpSystemTab(const FSpawnTabArgs& Args) {
    return SNew(SDockTab)
        .TabRole(NomadTab)
        [
            SNew(SHelpSystemList)
        ];
}

#undef LOCTEXT_NAMESPACE

