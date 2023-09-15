//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Utils/AssetUtils.h"

#include "Engine/Level.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"

static FString PATH_SPRITE_POINTLIGHT = "/Engine/EditorResources/LightIcons/S_LightPoint.S_LightPoint";
static FString PATH_SPRITE_POINTLIGHT_MOVABLE = "/Engine/EditorResources/LightIcons/S_LightPointMove.S_LightPointMove";
static FString PATH_SPRITE_SPOTLIGHT = "/Engine/EditorResources/LightIcons/S_LightSpot.S_LightSpot";
static FString PATH_SPRITE_SPOTLIGHT_MOVABLE = "/Engine/EditorResources/LightIcons/S_LightSpotMove.S_LightSpotMove";
static FString PATH_SPRITE_LIGHTERROR = "/Engine/EditorResources/LightIcons/S_LightError.S_LightError";

UTexture2D* FAssetUtils::GetSprite(const FString& Path) {
    return Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, *Path));
}

UTexture2D* FAssetUtils::GetPointLightSprite() {
    return GetSprite(PATH_SPRITE_POINTLIGHT);
}

UTexture2D* FAssetUtils::GetSpotLightSprite() {
    return GetSprite(PATH_SPRITE_SPOTLIGHT);
}

FString FAssetUtils::GetAssetPath(UObject* Object) {
    if (!Object) {
        return FString();
    }
    return Object->GetName();
    /*
    FString Path = Object->GetPathName();
    int32 ColonIndex = Path.Find(SUBOBJECT_DELIMITER);
    if (ColonIndex >= 0) {
        Path = Path.Mid(0, ColonIndex);
    }
    return Path;
    */
}

ULevel* FAssetUtils::GetPersistentLevel(UWorld* InWorld) {
    ULevel* PersistentLevel = nullptr;
    for (int i = 0; i < InWorld->GetNumLevels(); i++) {
        ULevel* Level = InWorld->GetLevel(i);
        if (Level->IsPersistentLevel()) {
            return Level;
        }
    }
    return nullptr;
}

