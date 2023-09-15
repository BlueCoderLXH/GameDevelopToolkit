//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

class UDungeonSpawnLogic;

struct DUNGEONARCHITECTRUNTIME_API FDungeonSceneProviderContext {
    FDungeonSceneProviderContext() : transform(FTransform::Identity) {
    }

    FTransform transform;
    FName NodeId;
    TArray<UDungeonSpawnLogic*> SpawnLogics;
    TSharedPtr<class IDungeonMarkerUserData> MarkerUserData;
};

