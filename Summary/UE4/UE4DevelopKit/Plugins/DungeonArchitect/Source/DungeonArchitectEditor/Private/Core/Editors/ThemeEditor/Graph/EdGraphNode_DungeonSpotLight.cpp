//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonSpotLight.h"

#include "Core/Utils/AssetUtils.h"

#define LOCTEXT_NAMESPACE "EdGraphNode_DungeonLight"

UEdGraphNode_DungeonSpotLight::UEdGraphNode_DungeonSpotLight(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
    SpotLight = ObjectInitializer.CreateDefaultSubobject<USpotLightComponent>(this, "SpotLight");

    SpotLight->CastShadows = false;
    SpotLight->CastDynamicShadows = false;
    SpotLight->CastStaticShadows = false;
}

UObject* UEdGraphNode_DungeonSpotLight::GetNodeAssetObject(UObject* Outer) {
    UObject* AssetObject = NewObject<USpotLightComponent>(Outer, NAME_None, RF_NoFlags, SpotLight);
    return AssetObject;
}

UObject* UEdGraphNode_DungeonSpotLight::GetThumbnailAssetObject() {
    return FAssetUtils::GetSpotLightSprite();
}

#undef LOCTEXT_NAMESPACE

