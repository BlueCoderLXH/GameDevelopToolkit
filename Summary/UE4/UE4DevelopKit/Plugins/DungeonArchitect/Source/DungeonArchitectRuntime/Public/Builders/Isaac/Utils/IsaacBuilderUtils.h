//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Builders/Isaac/IsaacDungeonModel.h"

class DUNGEONARCHITECTRUNTIME_API IsaacBuilderUtils {
public:

    static FIsaacRoomTile GetTileAt(int x, int z, const FIsaacRoom& room);
    static bool ContainsDoorAt(int x, int y, const FIsaacRoom& room);
    static FIsaacRoom GetRoom(UIsaacDungeonModel* model, int roomId);
};

