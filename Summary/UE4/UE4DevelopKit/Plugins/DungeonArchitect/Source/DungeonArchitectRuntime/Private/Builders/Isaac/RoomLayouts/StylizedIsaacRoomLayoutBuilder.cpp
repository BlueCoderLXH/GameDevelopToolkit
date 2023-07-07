//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/Isaac/RoomLayouts/StylizedIsaacRoomLayoutBuilder.h"


UStylizedIsaacRoomLayoutBuilder::UStylizedIsaacRoomLayoutBuilder(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer), minBrushSize(1), maxBrushSize(3) {

}

FIsaacRoomLayout UStylizedIsaacRoomLayoutBuilder::GenerateLayout(FIsaacRoomPtr room, FRandomStream& random,
                                                                 int roomWidth, int roomHeight) {
    TArray<FIntVector> doors = room->DoorPositions;
    FIsaacRoomLayout layout;
    layout.Initialize(roomWidth, roomHeight, EIsaacRoomTileType::Empty);

    int32 numDoors = doors.Num();
    if (numDoors > 1) {
        for (int i = 0; i < numDoors; i++) {
            for (int j = i + 1; j < numDoors; j++) {
                int32 brushSize = random.RandRange(minBrushSize, maxBrushSize + 1);
                ConnectDoors(layout, doors[i], doors[j], brushSize);
            }
        }
    }
    else {
        int32 brushSize = random.RandRange(minBrushSize, maxBrushSize + 1);
        ConnectDoors(layout, doors[0], doors[0], brushSize);
    }

    return layout;
}

void UStylizedIsaacRoomLayoutBuilder::ConnectDoors(FIsaacRoomLayout& layout, const FIntVector& doorA,
                                                   const FIntVector& doorB, int brushSize) {
    int32 minX = FMath::Min(doorA.X, doorB.X);
    int32 minY = FMath::Min(doorA.Y, doorB.Y);
    int32 maxX = FMath::Max(doorA.X, doorB.X);
    int32 maxY = FMath::Max(doorA.Y, doorB.Y);
    int32 width = layout.Width;
    int32 height = layout.Height;

    minX = FMath::Clamp(minX, 0, width - 1);
    maxX = FMath::Clamp(maxX, 0, width - 1);
    minY = FMath::Clamp(minY, 0, height - 1);
    maxY = FMath::Clamp(maxY, 0, height - 1);


    for (int x = minX; x <= maxX; x++) {
        int32 doorY = FMath::Clamp(doorA.Y, 0, height - 1);
        PaintTile(layout, x, doorY, brushSize, EIsaacRoomTileType::Floor);
    }

    for (int y = minY; y <= maxY; y++) {
        int32 doorX = FMath::Clamp(doorB.X, 0, width - 1);
        PaintTile(layout, doorX, y, brushSize, EIsaacRoomTileType::Floor);
    }
}

void UStylizedIsaacRoomLayoutBuilder::PaintTile(FIsaacRoomLayout& layout, int32 x, int32 y, int brushSize,
                                                EIsaacRoomTileType tileType) {
    int32 w = layout.Width;
    int32 h = layout.Height;
    if (x < 0 || x >= w || y < 0 || y >= h) return;

    int32 sx = x - FMath::FloorToInt(brushSize / 2.0f);
    int32 sy = y - FMath::FloorToInt(brushSize / 2.0f);

    for (int dx = 0; dx < brushSize; dx++) {
        for (int dy = 0; dy < brushSize; dy++) {
            int32 xx = sx + dx;
            int32 yy = sy + dy;

            SetTile(layout, xx, yy, w, h, EIsaacRoomTileType::Floor);
        }
    }
}

void UStylizedIsaacRoomLayoutBuilder::SetTile(FIsaacRoomLayout& layout, int x, int y, int width, int height,
                                              EIsaacRoomTileType tileType) {
    if (x < 0 || x >= width || y < 0 || y >= height) return;
    int32 index = width * y + x;
    layout.Tiles[index].tileType = tileType;
}

