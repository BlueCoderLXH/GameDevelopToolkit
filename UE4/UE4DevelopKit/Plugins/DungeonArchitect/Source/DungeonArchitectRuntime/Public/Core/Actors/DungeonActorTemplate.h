//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DungeonActorTemplate.generated.h"

/**
*
*/
UCLASS()
class DUNGEONARCHITECTRUNTIME_API UDungeonActorTemplate : public UObject {
    GENERATED_UCLASS_BODY()

public:

    UPROPERTY()
    TSubclassOf<AActor> ClassTemplate;

    /** Property to point to the template child actor for details panel purposes */
    UPROPERTY(Export)
    AActor* ActorTemplate;

};

