//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonActorBase.h"

#include "Components/SpotLightComponent.h"
#include "EdGraphNode_DungeonSpotLight.generated.h"

UCLASS()
class DUNGEONARCHITECTEDITOR_API UEdGraphNode_DungeonSpotLight : public UEdGraphNode_DungeonActorBase {
    GENERATED_UCLASS_BODY()

public:
    virtual UObject* GetNodeAssetObject(UObject* Outer) override;
    virtual UObject* GetThumbnailAssetObject() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    USpotLightComponent* SpotLight;
};

