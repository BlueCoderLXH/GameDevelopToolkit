//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/Isaac/IsaacDungeonBuilder.h"

#include "Builders/Isaac/IsaacDungeonConfig.h"
#include "Builders/Isaac/IsaacDungeonModel.h"
#include "Builders/Isaac/IsaacDungeonQuery.h"
#include "Builders/Isaac/IsaacDungeonSelectorLogic.h"
#include "Builders/Isaac/IsaacDungeonToolData.h"
#include "Builders/Isaac/IsaacDungeonTransformLogic.h"
#include "Builders/Isaac/Utils/IsaacBuilderUtils.h"
#include "Core/Dungeon.h"
#include "Core/DungeonArchitectConstants.h"
#include "Core/Utils/DungeonModelHelper.h"
#include "Core/Utils/MathUtils.h"
#include "Core/Volumes/DungeonMirrorVolume.h"

#include <sstream>
#include <stack>

DEFINE_LOG_CATEGORY(IsaacDungeonBuilderLog);


void UIsaacDungeonBuilder::BuildDungeonImpl(UWorld* World) {
    isaacModel = Cast<UIsaacDungeonModel>(model);
    isaacConfig = Cast<UIsaacDungeonConfig>(config);

    if (!isaacModel) {
        UE_LOG(IsaacDungeonBuilderLog, Error, TEXT("Invalid dungeon model provided to the grid builder"));
        return;
    }

    if (!isaacConfig) {
        UE_LOG(IsaacDungeonBuilderLog, Error, TEXT("Invalid dungeon gridConfig provided to the grid builder"));
        return;
    }

    GenerateLevelLayout();

    PropSockets.Reset();
}

struct FLevelGrowthNode {
    FIsaacRoomPtr room;
    int32 moveDirection;
};

class FIsaacRoomFactory {
public:
    FIsaacRoomFactory(int InRoomWidth, int InRoomHeight) : roomWidth(InRoomWidth), roomHeight(InRoomHeight) {
    }

    FIsaacRoomPtr CreateRoom(FIntVector position) {
        FIsaacRoomPtr room = MakeShareable(new FIsaacRoom());
        room->Id = idCounter++;
        room->Location = position;
        return room;
    }

private:
    int idCounter = 0;
    int roomWidth;
    int roomHeight;

};

typedef TSharedPtr<FIsaacRoomFactory> FIsaacRoomFactoryPtr;

FIntVector directions[] = {
    FIntVector(1, 0, 0),
    FIntVector(0, 1, 0),
    FIntVector(-1, 0, 0),
    FIntVector(0, -1, 0),
};

void UIsaacDungeonBuilder::GenerateLevelLayout() {
    auto startPosition = FIntVector::ZeroValue;
    TQueue<FLevelGrowthNode> queue;
    TSet<FIntVector> visited;

    FIsaacRoomFactoryPtr roomFactory = MakeShareable(
        new FIsaacRoomFactory(isaacConfig->RoomWidth, isaacConfig->RoomLength));
    rooms.Reset();
    doors.Reset();

    FIsaacRoomPtr startRoom = roomFactory->CreateRoom(FIntVector::ZeroValue);
    rooms.Add(startRoom);

    FLevelGrowthNode start;
    start.room = startRoom;
    start.moveDirection = 0;

    queue.Enqueue(start);
    visited.Add(start.room->Location);

    int32 numRooms = random.RandRange(isaacConfig->MinRooms, isaacConfig->MaxRooms);
    bool isSpawnRoom = true;
    while (!queue.IsEmpty()) {
        FLevelGrowthNode top;
        queue.Dequeue(top);
        if (isSpawnRoom) {
            // in the spawn room.  Spawn on all 4 sides
            for (int d = 0; d < 4; d++) {
                AddNextRoomNode(roomFactory, queue, visited, numRooms, top.room, d,
                                isaacConfig->SpawnRoomBranchProbability);
            }
            isSpawnRoom = false;
        }
        else {
            // Grow forward
            AddNextRoomNode(roomFactory, queue, visited, numRooms, top.room, top.moveDirection,
                            isaacConfig->GrowForwardProbability);

            // Grow sideways
            AddNextRoomNode(roomFactory, queue, visited, numRooms, top.room, (top.moveDirection + 1) % 4,
                            isaacConfig->GrowSidewaysProbability);
            AddNextRoomNode(roomFactory, queue, visited, numRooms, top.room, (top.moveDirection + 3) % 4,
                            isaacConfig->GrowSidewaysProbability);
        }

        if (rooms.Num() >= numRooms) {
            break;
        }
    }

    for (auto room : rooms) {
        room->Layout = isaacConfig->RoomLayoutBuilder->GenerateLayout(room, random, isaacConfig->RoomWidth,
                                                                      isaacConfig->RoomLength);
    }

    // Save the room and door data to the model as a copy by value
    isaacModel->Rooms.Reset();
    for (FIsaacRoomPtr roomPtr : rooms) {
        isaacModel->Rooms.Add(*roomPtr);
    }

    isaacModel->Doors.Reset();
    for (FIsaacDoorPtr doorPtr : doors) {
        isaacModel->Doors.Add(*doorPtr);
    }

    rooms.Reset();
    doors.Reset();
}

void UIsaacDungeonBuilder::AddNextRoomNode(TSharedPtr<FIsaacRoomFactory> roomFactory, TQueue<FLevelGrowthNode>& queue,
                                           TSet<FIntVector>& visited,
                                           int maxRooms, FIsaacRoomPtr parentRoom, int direction, float probability) {
    if (random.FRand() > probability) return;
    if (rooms.Num() >= maxRooms) return;

    FIntVector nextPosition = parentRoom->Location + directions[direction];
    if (!visited.Contains(nextPosition)) {
        FIsaacRoomPtr nextRoom = roomFactory->CreateRoom(nextPosition);
        rooms.Add(nextRoom);
        FLevelGrowthNode nextNode;
        nextNode.room = nextRoom;
        nextNode.moveDirection = direction;
        queue.Enqueue(nextNode);
        visited.Add(nextPosition);

        // Create a door between the two rooms
        ConnectRoomsWithDoors(parentRoom, nextRoom);
    }
    else {
        // See if we can connect to the other room
        // first make sure we don't already have a connection between the two
        FIsaacRoomPtr nextRoom = GetRoomAt(nextPosition);
        if (!ContainsDoorBetween(parentRoom->Id, nextRoom->Id)) {
            float loopTest = random.FRand();
            if (loopTest < isaacConfig->CycleProbability) {
                // Connect the two rooms together
                if (nextRoom.IsValid()) {
                    // Create a door between the two rooms
                    ConnectRoomsWithDoors(parentRoom, nextRoom);
                }
            }
        }
    }
}


void UIsaacDungeonBuilder::ConnectRoomsWithDoors(FIsaacRoomPtr roomA, FIsaacRoomPtr roomB) {
    // Create a door between the two rooms
    roomA->AdjacentRooms.Add(roomB->Id);
    roomB->AdjacentRooms.Add(roomA->Id);
    float doorPositionRatio = random.FRand();
    CreateDoor(roomA, roomB, doorPositionRatio);
}

FIsaacRoomPtr UIsaacDungeonBuilder::GetRoomAt(const FIntVector& position) {
    for (FIsaacRoomPtr room : rooms) {
        if (room->Location == position) {
            return room;
        }
    }
    return nullptr;
}

bool UIsaacDungeonBuilder::ContainsDoorBetween(int roomA, int roomB) {
    for (FIsaacDoorPtr door : doors) {
        if (door->roomA == roomA && door->roomB == roomB) return true;
        if (door->roomA == roomB && door->roomB == roomA) return true;
    }
    return false;
}

void UIsaacDungeonBuilder::CreateDoor(FIsaacRoomPtr roomA, FIsaacRoomPtr roomB, float ratio) {
    FIsaacDoorPtr door = MakeShareable(new FIsaacDoor());
    door->roomA = roomA->Id;
    door->roomB = roomB->Id;
    door->ratio = ratio;
    doors.Add(door);

    // Create the door tile
    int32 roomWidth = isaacConfig->RoomWidth;
    int32 roomHeight = isaacConfig->RoomLength;
    bool horizontal = (roomA->Location.Y - roomB->Location.Y) == 0; // Are the two room horizontal
    int32 size = horizontal ? isaacConfig->RoomLength : isaacConfig->RoomWidth;
    int32 location1D = FMath::FloorToInt(size * door->ratio);

    FIsaacRoomPtr leftRoom = roomA;
    FIsaacRoomPtr rightRoom = roomB;
    if (horizontal && leftRoom->Location.X > rightRoom->Location.X) {
        // Swap
        leftRoom = roomB;
        rightRoom = roomA;
    }
    else if (!horizontal && leftRoom->Location.Y > rightRoom->Location.Y) {
        // Swap
        leftRoom = roomB;
        rightRoom = roomA;
    }


    FIntVector leftRoomDoor;
    FIntVector rightRoomDoor;

    if (horizontal) {
        leftRoomDoor = FIntVector(roomWidth, location1D, 0);
        rightRoomDoor = FIntVector(-1, location1D, 0);
    }
    else {
        leftRoomDoor = FIntVector(location1D, roomHeight, 0);
        rightRoomDoor = FIntVector(location1D, -1, 0);
    }

    leftRoom->DoorPositions.Add(leftRoomDoor);
    rightRoom->DoorPositions.Add(rightRoomDoor);
}

class FIsaacRoomLayoutBuilder {
public:
    virtual FIsaacRoomLayout GenerateLayout(FIsaacRoomPtr room, FRandomStream& random, int roomWidth, int roomHeight) =
    0;
};

class StylizedIsaacRoomLayoutBuilder : public FIsaacRoomLayoutBuilder {
public:
    int32 minBrushSize = 1;
    int32 maxBrushSize = 3;

    virtual FIsaacRoomLayout GenerateLayout(FIsaacRoomPtr room, FRandomStream& random, int roomWidth, int roomHeight) override {
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

    void ConnectDoors(FIsaacRoomLayout& layout, const FIntVector& doorA, const FIntVector& doorB, int brushSize) {
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

    void PaintTile(FIsaacRoomLayout& layout, int32 x, int32 y, int brushSize, EIsaacRoomTileType tileType) {
        int32 w = layout.Width;
        int32 h = layout.Height;
        if (x < 0 || x >= w || y < 0 || y >= h) return;

        int32 sx = y - FMath::FloorToInt(brushSize / 2.0f);
        int32 sy = y - FMath::FloorToInt(brushSize / 2.0f);

        for (int dx = 0; dx < brushSize; dx++) {
            for (int dy = 0; dy < brushSize; dy++) {
                int32 xx = sx + dx;
                int32 yy = sy + dy;

                SetTile(layout, xx, yy, w, h, EIsaacRoomTileType::Floor);
            }
        }
    }

    void SetTile(FIsaacRoomLayout& layout, int x, int y, int width, int height, EIsaacRoomTileType tileType) {
        if (x < 0 || x >= width || y < 0 || y >= height) return;
        int32 index = width * y + x;
        layout.Tiles[index].tileType = tileType;
    }
};

FIsaacRoomLayout UIsaacDungeonBuilder::GenerateEmptyRoomLayout() {
    FIsaacRoomLayout layout;
    layout.Initialize(isaacConfig->RoomWidth, isaacConfig->RoomLength, EIsaacRoomTileType::Floor);
    return layout;
}


void UIsaacDungeonBuilder::EmitDungeonMarkers_Implementation() {
    Super::EmitDungeonMarkers_Implementation();

    isaacModel = Cast<UIsaacDungeonModel>(model);
    isaacConfig = Cast<UIsaacDungeonConfig>(config);

    ClearSockets();

    FVector tileSize(isaacConfig->TileSize.X, isaacConfig->TileSize.Y, 0);
    FVector roomSizeWorld = FVector(isaacConfig->RoomWidth, isaacConfig->RoomLength, 0) * tileSize;
    FVector roomPadding(isaacConfig->RoomPadding.X, isaacConfig->RoomPadding.Y, 0);
    for (const FIsaacRoom& room : isaacModel->Rooms) {
        FVector roomBasePosition = FMathUtils::ToVector(room.Location) * (roomSizeWorld + roomPadding);
        int32 roomWidth = room.Layout.Width;
        int32 roomHeight = room.Layout.Height;

        for (int x = -1; x < roomWidth + 1; x++) {
            for (int y = -1; y < roomHeight + 1; y++) {
                FIsaacRoomTile tile = IsaacBuilderUtils::GetTileAt(x, y, room);
                if (tile.tileType == EIsaacRoomTileType::Floor) {
                    CreateMarkerAt(x + 0.5f, y + 0.5f, 0, DAMarkerConstants::ST_GROUND, roomBasePosition, tileSize);
                }

            }
        }

        EmitBoundaryMarkers(DAMarkerConstants::ST_WALL, EIsaacRoomTileType::Empty, EIsaacRoomTileType::Floor, room,
                            roomWidth, roomHeight, roomBasePosition, tileSize);
        EmitBoundaryMarkers(DAMarkerConstants::ST_DOOR, EIsaacRoomTileType::Door, EIsaacRoomTileType::Floor, room,
                            roomWidth, roomHeight, roomBasePosition, tileSize);
    }
}

void UIsaacDungeonBuilder::EmitBoundaryMarkers(const FString& markerName, EIsaacRoomTileType adjacentTile1,
                                               EIsaacRoomTileType adjacentTile2,
                                               const FIsaacRoom& room, int32 roomWidth, int32 roomHeight,
                                               const FVector& roomBasePosition, const FVector& tileSize) {

    for (int x = -1; x < roomWidth; x++) {
        for (int y = -1; y < roomHeight; y++) {
            FIsaacRoomTile left = IsaacBuilderUtils::GetTileAt(x, y, room);
            FIsaacRoomTile right = IsaacBuilderUtils::GetTileAt(x + 1, y, room);

            FIsaacRoomTile top = left;
            FIsaacRoomTile bottom = IsaacBuilderUtils::GetTileAt(x, y + 1, room);

            if (left.tileType == adjacentTile1 && right.tileType == adjacentTile2) {
                CreateMarkerAt(x + 1, y + 0.5f, 90, markerName, roomBasePosition, tileSize);
            }
            else if (left.tileType == adjacentTile2 && right.tileType == adjacentTile1) {
                CreateMarkerAt(x + 1, y + 0.5f, -90, markerName, roomBasePosition, tileSize);
            }

            if (top.tileType == adjacentTile1 && bottom.tileType == adjacentTile2) {
                CreateMarkerAt(x + 0.5f, y + 1, 180, markerName, roomBasePosition, tileSize);
            }
            else if (top.tileType == adjacentTile2 && bottom.tileType == adjacentTile1) {
                CreateMarkerAt(x + 0.5f, y + 1, 0, markerName, roomBasePosition, tileSize);
            }

        }
    }
}


void UIsaacDungeonBuilder::CreateMarkerAt(float gridX, float gridY, float angle, const FString& markerName,
                                          const FVector& roomBasePosition, const FVector& tileSize) {
    FVector tilePosition(gridX, gridY, 0);
    FVector tileOffset = tilePosition * tileSize;
    FVector markerPosition = roomBasePosition + tileOffset;
    FQuat rotation = FQuat::MakeFromEuler(FVector(0, 0, angle));
    FTransform transform = FTransform::Identity;
    transform.SetLocation(markerPosition);
    transform.SetRotation(rotation);

    EmitMarker(markerName, transform);
}

void UIsaacDungeonBuilder::DrawDebugData(UWorld* InWorld, bool bPersistant /*= false*/, float lifeTime /*= 0*/) {

}

void UIsaacDungeonBuilder::MirrorDungeon() {

}

TSubclassOf<UDungeonModel> UIsaacDungeonBuilder::GetModelClass() {
    return UIsaacDungeonModel::StaticClass();
}

TSubclassOf<UDungeonConfig> UIsaacDungeonBuilder::GetConfigClass() {
    return UIsaacDungeonConfig::StaticClass();
}

TSubclassOf<UDungeonToolData> UIsaacDungeonBuilder::GetToolDataClass() {
    return UIsaacDungeonToolData::StaticClass();
}

TSubclassOf<UDungeonQuery> UIsaacDungeonBuilder::GetQueryClass() {
    return UIsaacDungeonQuery::StaticClass();
}

bool UIsaacDungeonBuilder::ProcessSpatialConstraint(UDungeonSpatialConstraint* SpatialConstraint,
                                                    const FTransform& Transform, FQuat& OutRotationOffset) {
    return false;
}

void UIsaacDungeonBuilder::GetDefaultMarkerNames(TArray<FString>& OutMarkerNames) {
    OutMarkerNames.Reset();
    OutMarkerNames.Add(DAMarkerConstants::ST_GROUND);
    OutMarkerNames.Add(DAMarkerConstants::ST_WALL);
    OutMarkerNames.Add(DAMarkerConstants::ST_DOOR);
}

void UIsaacDungeonBuilder::MirrorDungeonWithVolume(ADungeonMirrorVolume* MirrorVolume) {
}

bool UIsaacDungeonBuilder::PerformSelectionLogic(const TArray<UDungeonSelectorLogic*>& SelectionLogics,
                                                 const FPropSocket& socket) {
    return false;
}

FTransform UIsaacDungeonBuilder::PerformTransformLogic(const TArray<UDungeonTransformLogic*>& TransformLogics,
                                                       const FPropSocket& socket) {
    return FTransform::Identity;
}

