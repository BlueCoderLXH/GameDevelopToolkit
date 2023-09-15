//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/LaunchPad/Data/LaunchPadResource.h"

#include "Brushes/SlateImageBrush.h"
#include "HAL/PlatformFilemanager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "Styling/CoreStyle.h"

namespace {
    const FVector2D IMG_SIZE_THUMB = FVector2D(180, 100);
    const FVector2D IMG_SIZE_DEFAULT = FVector2D(720, 400);
}

//////////////////////////// ILaunchPadDataSource //////////////////////////// 

const FSlateBrush* ILaunchPadDataSource::GetImageThumb(const FString& InPath) {
    return GetImageImpl(InPath, IMG_SIZE_THUMB);
}

const FSlateBrush* ILaunchPadDataSource::GetImage(const FString& InPath) {
    return GetImageImpl(InPath, IMG_SIZE_DEFAULT);
}

const FSlateBrush* ILaunchPadDataSource::GetImage(const FString& InPath, const FVector2D& InImageSize) {
    return GetImageImpl(InPath, InImageSize);
}

//////////////////////////// FLaunchPadResourceFolderDataSource //////////////////////////// 

FLaunchPadResourceFolderDataSource::FLaunchPadResourceFolderDataSource() {
    BaseTextDir = IPluginManager::Get().FindPlugin(TEXT("DungeonArchitect"))->GetBaseDir() / TEXT(
        "Resources/LaunchPad/data/");
    BaseImageDir = IPluginManager::Get().FindPlugin(TEXT("DungeonArchitect"))->GetBaseDir() / TEXT(
        "Resources/LaunchPad/images/");
}

FLaunchPadTextResourcePtr FLaunchPadResourceFolderDataSource::GetText(const FString& InPath) {
    if (InPath.Contains("..")) {
        // reject this
        return nullptr;
    }
    FString FullPath = BaseTextDir + InPath;

    FString Data;
    if (FFileHelper::LoadFileToString(Data, *FullPath)) {
        FLaunchPadTextResourcePtr TextResource = MakeShareable(new FLaunchPadTextResource);
        TextResource->Path = InPath;
        TextResource->Value = Data;
        return TextResource;
    }
    return nullptr;
}

const FSlateBrush*
FLaunchPadResourceFolderDataSource::GetImageImpl(const FString& InPath, const FVector2D& InImageSize) {
    bool bInputValid = !InPath.Contains("..");

    FSlateBrush* Brush = nullptr;

    if (bInputValid) {
        uint32 HashCode = HashCombine(GetTypeHash(InPath), GetTypeHash(InImageSize));
        FSlateBrush* SearchResult = Brushes.Find(HashCode);
        if (SearchResult) {
            Brush = SearchResult;
        }
        else {
            FString FullPath = BaseImageDir + InPath;
            IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
            if (PlatformFile.FileExists(*FullPath)) {
                Brushes.Add(HashCode, FSlateImageBrush(FullPath, InImageSize));
                Brush = &Brushes[HashCode];
            }
        }
    }

    if (!Brush) {
        Brush = FCoreStyle::Get().GetDefaultBrush();
    }

    return Brush;
}

