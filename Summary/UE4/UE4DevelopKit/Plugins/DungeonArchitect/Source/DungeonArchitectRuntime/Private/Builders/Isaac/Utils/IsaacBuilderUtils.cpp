//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/Isaac/Utils/IsaacBuilderUtils.h"


FIsaacRoomTile IsaacBuilderUtils::GetTileAt(int x, int y, const FIsaacRoom& room) {
    if (x < 0 || x >= room.Layout.Width || y < 0 || y >= room.Layout.Height) {
        FIsaacRoomTile invalidTile;
        invalidTile.tileType = EIsaacRoomTileType::Empty;
        // Check if we have a door here
        if (ContainsDoorAt(x, y, room)) {
            invalidTile.tileType = EIsaacRoomTileType::Door;
        }
        return invalidTile;
    }
    int32 index = y * room.Layout.Width + x;
    return room.Layout.Tiles[index];
}

bool IsaacBuilderUtils::ContainsDoorAt(int x, int y, const FIsaacRoom& room) {
    return room.DoorPositions.Contains(FIntVector(x, y, 0));
}

FIsaacRoom IsaacBuilderUtils::GetRoom(UIsaacDungeonModel* model, int roomId) {
    for (const FIsaacRoom& room : model->Rooms) {
        if (room.Id == roomId) {
            return room;
        }
    }
    return FIsaacRoom();
}

