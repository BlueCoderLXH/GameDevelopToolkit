//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/Isaac/IsaacDungeonConfig.h"

#include "Builders/Isaac/RoomLayouts/StylizedIsaacRoomLayoutBuilder.h"

DEFINE_LOG_CATEGORY(IsaacDungeonConfigLog);

UIsaacDungeonConfig::UIsaacDungeonConfig(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer),
    MinRooms(30),
    MaxRooms(40),
    RoomWidth(10),
    RoomLength(6),
    TileSize(FVector(400, 400, 200)),
    RoomPadding(FVector(400, 400, 0)),
    GrowForwardProbability(0.80f),
    GrowSidewaysProbability(0.25f),
    SpawnRoomBranchProbability(0.76f),
    CycleProbability(0.5f) {
    RoomLayoutBuilder = ObjectInitializer.CreateDefaultSubobject<UStylizedIsaacRoomLayoutBuilder>(
        this, "RoomLayoutBuilder");
}

