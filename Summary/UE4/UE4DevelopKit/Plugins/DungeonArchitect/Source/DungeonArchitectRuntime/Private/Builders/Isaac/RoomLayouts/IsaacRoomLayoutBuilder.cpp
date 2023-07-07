//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/Isaac/RoomLayouts/IsaacRoomLayoutBuilder.h"


FIsaacRoomLayout UIsaacRoomLayoutBuilder::GenerateLayout(FIsaacRoomPtr room, FRandomStream& random, int roomWidth,
                                                         int roomHeight) {
    FIsaacRoomLayout layout;
    layout.Initialize(roomWidth, roomHeight, EIsaacRoomTileType::Floor);
    return layout;
}

