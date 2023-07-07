//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Navigation/NavLinkProxy.h"
#include "DungeonNavLinkProxy.generated.h"

UCLASS(Blueprintable, autoCollapseCategories=(SmartLink, Actor), hideCategories=(Input))
class ADungeonNavLinkProxy : public ANavLinkProxy {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="Dungeon")
    void SetupSmartLinkData(const FVector& Start, const FVector& End, ENavLinkDirection::Type Direction);
    
};

