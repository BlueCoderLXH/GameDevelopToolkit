//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Builders/Isaac/RoomLayouts/IsaacRoomLayoutBuilder.h"
#include "Core/DungeonConfig.h"
#include "IsaacDungeonConfig.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(IsaacDungeonConfigLog, Log, All);


UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API UIsaacDungeonConfig : public UDungeonConfig {
    GENERATED_UCLASS_BODY()

public:
    /**
    * Min no. of rooms to create in the map
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    int32 MinRooms;

    /** Max no. of rooms to create in the map */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    int32 MaxRooms;

    /** The size of the room */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    int32 RoomWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    int32 RoomLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    FVector TileSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    FVector RoomPadding;

    /** Probability of moving forward in the room */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    float GrowForwardProbability;

    /** Probability of moving forward in the room */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    float GrowSidewaysProbability;

    /** Probability of moving forward in the room */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    float SpawnRoomBranchProbability;

    /** Probability of moving forward in the room */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    float CycleProbability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, SimpleDisplay, Category = Dungeon)
    UIsaacRoomLayoutBuilder* RoomLayoutBuilder;

};

