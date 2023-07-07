//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Builders/Grid/GridDungeonModel.h"
#include "Core/Volumes/DungeonVolume.h"
#include "GridDungeonPlatformVolume.generated.h"

/**
*
*/
UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API AGridDungeonPlatformVolume : public ADungeonVolume {
    GENERATED_BODY()

public:
    AGridDungeonPlatformVolume(const FObjectInitializer& ObjectInitializer);

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    FCellType CellType; // TODO: abstract this
};

