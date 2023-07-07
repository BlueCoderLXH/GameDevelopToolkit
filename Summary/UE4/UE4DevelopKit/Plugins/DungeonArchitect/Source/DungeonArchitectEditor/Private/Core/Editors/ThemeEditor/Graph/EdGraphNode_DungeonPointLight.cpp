//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonPointLight.h"

#include "Core/Utils/AssetUtils.h"

#define LOCTEXT_NAMESPACE "EdGraphNode_DungeonPointLight"

UEdGraphNode_DungeonPointLight::UEdGraphNode_DungeonPointLight(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
    PointLight = ObjectInitializer.CreateDefaultSubobject<UPointLightComponent>(this, "PointLight");

    PointLight->CastShadows = false;
    PointLight->CastDynamicShadows = false;
    PointLight->CastStaticShadows = false;
}

UObject* UEdGraphNode_DungeonPointLight::GetNodeAssetObject(UObject* Outer) {
    UObject* AssetObject = NewObject<UPointLightComponent>(Outer, NAME_None, RF_NoFlags, PointLight);
    return AssetObject;
}

UObject* UEdGraphNode_DungeonPointLight::GetThumbnailAssetObject() {
    return FAssetUtils::GetPointLightSprite();
}

#undef LOCTEXT_NAMESPACE

