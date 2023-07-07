//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/DungeonConfig.h"
#include "FloorPlanConfig.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(FloorPlanConfigLog, Log, All);


UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API UFloorPlanConfig : public UDungeonConfig {
    GENERATED_UCLASS_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    FVector BuildingSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    FVector GridSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    int32 MinRoomSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    int32 MaxRoomSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    int32 HallWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    int32 MinRoomChunkArea;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    float RoomSplitProbabilityOffset;

    
#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif  // WITH_EDITOR
    
};

