//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/LevelEditor/HelpSystem/DungeonArchitectHelpSystemStyle.h"

#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

#define IMAGE_PLUGIN_BRUSH( RelativePath, ... ) FSlateImageBrush( FDungeonArchitectHelpSystemStyle::InResource( RelativePath, ".png" ), __VA_ARGS__ )

TSharedPtr<FSlateStyleSet> FDungeonArchitectHelpSystemStyle::StyleInstance = nullptr;

FString FDungeonArchitectHelpSystemStyle::InResource(const FString& RelativePath, const ANSICHAR* Extension) {
    static FString ResourcesDir = IPluginManager::Get().FindPlugin(TEXT("DungeonArchitect"))->GetBaseDir() /
        TEXT("Resources");
    return (ResourcesDir / RelativePath) + Extension;
}

void FDungeonArchitectHelpSystemStyle::Initialize() {
    if (!StyleInstance.IsValid()) {
        StyleInstance = Create();
        FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
    }
}

void FDungeonArchitectHelpSystemStyle::Shutdown() {
    FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
    ensure(StyleInstance.IsUnique());
    StyleInstance.Reset();
}

FName FDungeonArchitectHelpSystemStyle::GetStyleSetName() {
    static FName StyleSetName(TEXT("DungeonArchitectHelpSystemStyle"));
    return StyleSetName;
}

TSharedRef<class FSlateStyleSet> FDungeonArchitectHelpSystemStyle::Create() {
    const FVector2D Icon16x16(16.0f, 16.0f);
    const FVector2D Icon20x20(20.0f, 20.0f);
    const FVector2D Icon40x40(40.0f, 40.0f);
    const FVector2D Icon48x48(48.0f, 48.0f);

    TSharedRef<FSlateStyleSet> StyleRef = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
    StyleRef->SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
    StyleRef->SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));

    FSlateStyleSet& Style = StyleRef.Get();
    // Help System icons
    {
        Style.Set("DungeonArchitect.HelpSystem.IconUserGuide",
                  new IMAGE_PLUGIN_BRUSH("Icons/HelpSystem/DA_UserGuide_40x", Icon40x40));
        Style.Set("DungeonArchitect.HelpSystem.IconQuickStartGuide",
                  new IMAGE_PLUGIN_BRUSH("Icons/HelpSystem/DA_QuickStart_40x", Icon40x40));
        Style.Set("DungeonArchitect.HelpSystem.IconVideoTutorials",
                  new IMAGE_PLUGIN_BRUSH("Icons/HelpSystem/DA_VideoTutorials_40x", Icon40x40));
        Style.Set("DungeonArchitect.HelpSystem.IconReleaseNotes",
                  new IMAGE_PLUGIN_BRUSH("Icons/HelpSystem/DA_ReleaseNotes_40x", Icon40x40));
        Style.Set("DungeonArchitect.HelpSystem.IconDiscordChat",
                  new IMAGE_PLUGIN_BRUSH("Icons/HelpSystem/DA_Discord_40x", Icon40x40));
        Style.Set("DungeonArchitect.HelpSystem.IconForumThread",
                  new IMAGE_PLUGIN_BRUSH("Icons/HelpSystem/DA_ForumThread_40x", Icon40x40));
        Style.Set("DungeonArchitect.HelpSystem.IconRatePlugin",
                  new IMAGE_PLUGIN_BRUSH("Icons/HelpSystem/DA_RatePlugin_40x", Icon40x40));
    }

    return StyleRef;
}

#undef IMAGE_PLUGIN_BRUSH

const ISlateStyle& FDungeonArchitectHelpSystemStyle::Get() {
    return *StyleInstance;
}

#undef IMAGE_PLUGIN_BRUSH

