//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Volumes/DungeonVolume.h"
#include "DungeonMirrorVolume.generated.h"

/**
*
*/
UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API ADungeonMirrorVolume : public ADungeonVolume {
    GENERATED_BODY()

public:
    ADungeonMirrorVolume(const FObjectInitializer& ObjectInitializer);

};

