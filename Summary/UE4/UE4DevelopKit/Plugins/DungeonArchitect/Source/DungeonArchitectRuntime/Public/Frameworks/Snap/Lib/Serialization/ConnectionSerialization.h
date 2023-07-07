//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "ConnectionSerialization.generated.h"

USTRUCT()
struct DUNGEONARCHITECTRUNTIME_API FSnapConnectionInstance {
    GENERATED_USTRUCT_BODY()

    UPROPERTY()
    FGuid ModuleA;

    UPROPERTY()
    FGuid DoorA;

    UPROPERTY()
    FGuid ModuleB;

    UPROPERTY()
    FGuid DoorB;

    UPROPERTY()
    FTransform WorldTransform;

    UPROPERTY()
    bool bHasSpawnedDoorActor = false;

    // 如果当前Connection为门, bReverse为false：为前门, bReverse为true：为后门
    UPROPERTY()
    bool bReverse = false;

    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> SpawnedDoorActors;
};

