//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

class UTexture2D;
class ULevel;

class DUNGEONARCHITECTRUNTIME_API FAssetUtils {
public:
    static UTexture2D* GetSprite(const FString& Path);
    static UTexture2D* GetPointLightSprite();
    static UTexture2D* GetSpotLightSprite();
    static FString GetAssetPath(UObject* Object);

    /*
    template<typename T>
    static T* CloneAsset(UObject* AssetObject, const FString& TemplatePath, const FString& DesiredName) {
        FString PackageName, AssetName;
        GenerateExternalAssetName(AssetObject, DesiredName, PackageName, AssetName);
        T* Template = LoadObject<T>(NULL, *TemplatePath, NULL, LOAD_None, NULL);

        IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
        UObject* AssetObject = AssetTools.DuplicateAsset(AssetName, PackageName, Template);
        T* Asset = Cast<T>(AssetObject);
        if (!Asset) {
            UE_LOG(LogThemeMaterialFactory, Warning, TEXT("Failed to clone asset at location %s/%s"), *PackageName, *AssetName);
        }

        return Asset;
    }
    */

    static ULevel* GetPersistentLevel(UWorld* InWorld);


};

