//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/LaunchPad/Styles/LaunchPadStyle.h"

#include "Brushes/SlateImageBrush.h"
#include "Interfaces/IPluginManager.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

#define LP_IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush(BaseImageDir / (RelativePath), __VA_ARGS__ )

TSharedPtr<FSlateStyleSet> FDALaunchPadStyle::StyleInstance = nullptr;

void FDALaunchPadStyle::Initialize() {
    if (!StyleInstance.IsValid()) {
        StyleInstance = Create();
        FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
    }
}

void FDALaunchPadStyle::Shutdown() {
    FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
    ensure(StyleInstance.IsUnique());
    StyleInstance.Reset();
}

FName FDALaunchPadStyle::GetStyleSetName() {
    static FName StyleSetName(TEXT("DALaunchPadStyle"));
    return StyleSetName;
}

TSharedRef<class FSlateStyleSet> FDALaunchPadStyle::Create() {
    TSharedRef<FStyle> StyleRef = MakeShareable(new FStyle);
    StyleRef->Initialize();

    return StyleRef;
}

const ISlateStyle& FDALaunchPadStyle::Get() {
    return *StyleInstance;
}

FDALaunchPadStyle::FStyle::FStyle()
    : FSlateStyleSet(FDALaunchPadStyle::GetStyleSetName()) {
}


void FDALaunchPadStyle::FStyle::Initialize() {
    FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("DungeonArchitect"))->GetBaseDir();
    FString BaseImageDir = PluginDir / TEXT("Resources/LaunchPad/images/");
    FString ImageRegistryFilePath = BaseImageDir / TEXT("image_registry.json");

    SetContentRoot(BaseImageDir);
    SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));

    FString ImageRegistryJsonText;
    if (!FFileHelper::LoadFileToString(ImageRegistryJsonText, *ImageRegistryFilePath)) {
        return;
    }

    FDALaunchPadStyleRegistry Registry;
    if (!FJsonObjectConverter::JsonObjectStringToUStruct(ImageRegistryJsonText, &Registry, 0, 0)) {
        return;
    }

    for (const FDALaunchPadStyleRegistryPath& PathInfo : Registry.Paths) {
        Set(*PathInfo.Id, new LP_IMAGE_BRUSH(PathInfo.Path, FVector2D(PathInfo.Width, PathInfo.Height)));
    }

}

#undef LP_IMAGE_BRUSH

