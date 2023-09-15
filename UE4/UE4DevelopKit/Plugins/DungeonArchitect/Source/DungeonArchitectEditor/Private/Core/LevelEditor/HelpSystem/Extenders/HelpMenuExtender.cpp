//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/LevelEditor/HelpSystem/Extenders/HelpMenuExtender.h"

#include "Core/LevelEditor/HelpSystem/DungeonArchitectHelpSystemCommands.h"

#include "EditorStyleSet.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "LevelEditor.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FDAHelpMenuExtender"

class DocumentationHandler {
public:
    static void OpenDocUserGuide() {
        const FString Link = "http://coderespawn.github.io/dungeon-architect-user-guide-ue4";
        OpenLink(Link);
    }

    static void OpenDocQuickStartGuide() {
        const FString Link = "http://coderespawn.github.io/dungeon-architect-quick-start-ue4";
        OpenLink(Link);
    }

    static void OpenDocVideoTutorials() {
        const FString Link =
            "http://htmlpreview.github.io/?https://github.com/coderespawn/dungeon-architect-quick-start-ue4/blob/4.9/Docs/VideoTutorials.html";
        OpenLink(Link);
    }

private:
    static void OpenLink(const FString& Link) {
        FPlatformProcess::LaunchURL(*Link, nullptr, nullptr);
    }
};

void FDAHelpMenuExtender::Extend() {
    // bind the level editor commands
    GlobalLevelEditorActions = TSharedPtr<FUICommandList>(new FUICommandList);
    FUICommandList& ActionList = *GlobalLevelEditorActions;
    ActionList.MapAction(FDungeonArchitectHelpSystemCommands::Get().OpenDocumentationUserGuide,
                         FExecuteAction::CreateStatic(&DocumentationHandler::OpenDocUserGuide));
    ActionList.MapAction(FDungeonArchitectHelpSystemCommands::Get().OpenDocumentationQuickStartGuide,
                         FExecuteAction::CreateStatic(&DocumentationHandler::OpenDocQuickStartGuide));
    ActionList.MapAction(FDungeonArchitectHelpSystemCommands::Get().OpenDocumentationVideoTutorials,
                         FExecuteAction::CreateStatic(&DocumentationHandler::OpenDocVideoTutorials));
    {
        MenuExtender = MakeShareable(new FExtender());
        MenuExtender->AddMenuExtension(
            "HelpBrowse",
            EExtensionHook::After,
            GlobalLevelEditorActions,
            FMenuExtensionDelegate::CreateStatic(&FDAHelpMenuExtender::AddDocMenuCommands));

        FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
        LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
    }
}

void FDAHelpMenuExtender::Release() {
    FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
    LevelEditorModule.GetMenuExtensibilityManager()->RemoveExtender(MenuExtender);
    MenuExtender.Reset();
}

void FDAHelpMenuExtender::AddDocMenuCommands(FMenuBuilder& MenuBuilder) {
    FSlateIcon DocMenuIcon = FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.BrowseDocumentation");
    MenuBuilder.AddSubMenu(LOCTEXT("DungeonArchitect", "Dungeon Architect"),
                           LOCTEXT("DungeonArchitectToolTip", "Dungeon Architect Documentation"),
                           FNewMenuDelegate::CreateStatic(&FDAHelpMenuExtender::CreateDocSubMenu),
                           false, DocMenuIcon
    );
}

void FDAHelpMenuExtender::CreateDocSubMenu(FMenuBuilder& MenuBuilder) {
    FSlateIcon DocItemIcon = FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.BrowseDocumentation");
    MenuBuilder.AddMenuEntry(FDungeonArchitectHelpSystemCommands::Get().OpenDocumentationUserGuide, NAME_None,
                             TAttribute<FText>(), TAttribute<FText>(), DocItemIcon);
    MenuBuilder.AddMenuEntry(FDungeonArchitectHelpSystemCommands::Get().OpenDocumentationQuickStartGuide, NAME_None,
                             TAttribute<FText>(), TAttribute<FText>(), DocItemIcon);
    MenuBuilder.AddMenuEntry(FDungeonArchitectHelpSystemCommands::Get().OpenDocumentationVideoTutorials, NAME_None,
                             TAttribute<FText>(), TAttribute<FText>(), DocItemIcon);
}


#undef LOCTEXT_NAMESPACE

