//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Volumes/DungeonVolume.h"
#include "FloorPlanRoomVolume.generated.h"

/**
*
*/
UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API AFloorPlanRoomVolume : public ADungeonVolume {
    GENERATED_UCLASS_BODY()

public:

    /** Should the walls be generated around this room */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    bool bCreateWalls;

    /** Should it be connected with doors */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    bool bConnectDoors;

    /** Give a higher priority to the volume chunk so it overrides everything in its path */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    FString WallMarker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    FString GroundMarker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    FString CeilingMarker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    FString DoorMarker;

    /** Will emit a marker at the center of the volume on each floor it overlaps with */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    FString CenterMarker;

    /** The marker on the center of the room, emitted at each floor it spans */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    FString PerFloorCenterMarker;
};

