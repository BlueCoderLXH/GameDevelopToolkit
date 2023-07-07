//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/LevelEditor/Extenders/EditorUIExtender.h"

#include "Core/Editors/LaunchPad/LaunchPad.h"
#include "Core/LevelEditor/Customizations/DungeonArchitectStyle.h"
#include "Core/LevelEditor/HelpSystem/DungeonArchitectHelpSystem.h"

#include "AssetToolsModule.h"
#include "EditorModeManager.h"
#include "EngineUtils.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Framework/Notifications/NotificationManager.h"
#include "LevelEditor.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "EditorUIExtender"

void FEditorUIExtender::Extend() {
    struct Local {
        static void LaunchURL(FString URL) {
            FPlatformProcess::LaunchURL(*URL, nullptr, nullptr);
        }

        static TSharedRef<SWidget> HandleShowDropDownMenu() {
            FMenuBuilder MenuBuilder(true, FDALevelToolbarCommands::Get().LevelMenuActionList);

            MenuBuilder.BeginSection("DA-Tools", LOCTEXT("DAHeader", "Dungeon Architect"));

            FSlateIcon LaunchpadToolItemIcon = FSlateIcon(FDungeonArchitectStyle::GetStyleSetName(),
                                                          "DungeonArchitect.ToolbarItem.IconLaunchPad");
            MenuBuilder.AddMenuEntry(FDALevelToolbarCommands::Get().OpenLaunchPad, NAME_None, TAttribute<FText>(),
                                     TAttribute<FText>(), LaunchpadToolItemIcon);

            FSlateIcon DocItemIcon = FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.BrowseDocumentation");
            MenuBuilder.AddMenuEntry(FDALevelToolbarCommands::Get().OpenHelpWindow, NAME_None, TAttribute<FText>(),
                                     TAttribute<FText>(), DocItemIcon);

            MenuBuilder.EndSection();

            return MenuBuilder.MakeWidget();
        }

        static void ExtendLevelToolbar(FToolBarBuilder& ToolbarBuilder) {
            ToolbarBuilder.AddToolBarButton(FDALevelToolbarCommands::Get().OpenLaunchPad,
                                            TEXT("DALaunchPad"),
                                            LOCTEXT("DAToolbarButtonText", "Dungeon Arch"),
                                            LOCTEXT("DAToolbarButtonTooltip", "Dungeon Architect Launch Pad"),
                                            FSlateIcon(FDungeonArchitectStyle::GetStyleSetName(),
                                                       TEXT("DungeonArchitect.Toolbar.IconMain")));
        }
    };

    FLevelEditorModule& LevelEditorModule = FModuleManager::Get().LoadModuleChecked<FLevelEditorModule>("LevelEditor");

    LevelToolbarExtender = MakeShareable(new FExtender);
    LevelToolbarExtender->AddToolBarExtension(
        "Settings",
        EExtensionHook::After,
        FDALevelToolbarCommands::Get().LevelMenuActionList,
        FToolBarExtensionDelegate::CreateStatic(&Local::ExtendLevelToolbar)
    );

    LevelEditorModule.GetToolBarExtensibilityManager().Get()->AddExtender(LevelToolbarExtender);
}

void FEditorUIExtender::Release() {
    FLevelEditorModule* LevelEditorModule = FModuleManager::Get().GetModulePtr<FLevelEditorModule>("LevelEditor");
    if (LevelEditorModule) {
        if (LevelEditorModule->GetToolBarExtensibilityManager().IsValid()) {
            LevelEditorModule->GetToolBarExtensibilityManager().Get()->RemoveExtender(LevelToolbarExtender);
        }
    }
}

FDALevelToolbarCommands::FDALevelToolbarCommands()
    : TCommands<FDALevelToolbarCommands>(
        TEXT("DAToolbar"),
        NSLOCTEXT("DAToolbar", "DAToolbar", "Dungeon Architect"),
        NAME_None,
        FDungeonArchitectStyle::GetStyleSetName()) {
}

void FDALevelToolbarCommands::RegisterCommands() {
    UI_COMMAND(OpenLaunchPad, "Launch Pad", "Browse samples and templates", EUserInterfaceActionType::Button,
               FInputChord());
    UI_COMMAND(OpenHelpWindow, "Help / Support", "Documentation and help for the Dungeon Architect plugin",
               EUserInterfaceActionType::Button, FInputChord());

    LevelMenuActionList = MakeShareable(new FUICommandList);

    BindCommands();
}

void FDALevelToolbarCommands::BindCommands() {
    LevelMenuActionList->MapAction(
        OpenHelpWindow,
        FExecuteAction::CreateStatic(&FDungeonArchitectHelpSystem::LaunchHelpSystemTab)
    );

    LevelMenuActionList->MapAction(
        OpenLaunchPad,
        FExecuteAction::CreateStatic(&FLaunchPadSystem::Launch)
    );
}

#undef LOCTEXT_NAMESPACE

