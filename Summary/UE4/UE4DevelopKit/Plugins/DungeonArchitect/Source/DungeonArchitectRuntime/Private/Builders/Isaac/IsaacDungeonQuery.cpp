//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/Isaac/IsaacDungeonQuery.h"

#include "Builders/Isaac/Utils/IsaacBuilderUtils.h"
#include "Core/Utils/MathUtils.h"

#include "Containers/Queue.h"

DEFINE_LOG_CATEGORY(LogIsaacDungeonQuery);

void UIsaacDungeonQuery::InitializeImpl(UDungeonConfig* InConfig, UDungeonModel* InModel) {
    Config = Cast<UIsaacDungeonConfig>(InConfig);
    Model = Cast<UIsaacDungeonModel>(InModel);

    if (!Config) {
        UE_LOG(LogIsaacDungeonQuery, Error, TEXT("Invalid config passed to isaac query object"));
    }
    if (!Model) {
        UE_LOG(LogIsaacDungeonQuery, Error, TEXT("Invalid model passed to isaac query object"));
    }
}

FIsaacRoom UIsaacDungeonQuery::GetRoom(int32 RoomId) {
    if (!Model) {
        UE_LOG(LogIsaacDungeonQuery, Error, TEXT("GetRoom: Invalid model state"));
        return FIsaacRoom();
    }

    for (const FIsaacRoom& Room : Model->Rooms) {
        if (Room.Id == RoomId) {
            return Room;
        }
    }

    UE_LOG(LogIsaacDungeonQuery, Error, TEXT("GetRoom: Cannot find room with id: %d"), RoomId);
    return FIsaacRoom();
}

void CreateRoomMap(const TArray<FIsaacRoom>& Rooms, TMap<int32, FIsaacRoom>& OutRoomMap) {
    for (const FIsaacRoom& Room : Rooms) {
        OutRoomMap.Add(Room.Id, Room);
    }
}

void UIsaacDungeonQuery::GetFurthestRooms(int32& OutRoomA, int32& OutRoomB) {
    int32 BestDistance = 0;
    TMap<int32, FIsaacRoom> RoomMap;
    CreateRoomMap(Model->Rooms, RoomMap);

    struct FIsaacRoomSearchNode {
        FIsaacRoomSearchNode() : RoomId(-1), Distance(0) {
        }

        FIsaacRoomSearchNode(int32 InRoomId, float InDistance) : RoomId(InRoomId), Distance(InDistance) {
        }

        int32 RoomId;
        float Distance;
    };

    for (const FIsaacRoom& StartRoom : Model->Rooms) {
        // Start a BFS from here
        TQueue<FIsaacRoomSearchNode> Queue; // Queue node is the room id
        Queue.Enqueue(FIsaacRoomSearchNode(StartRoom.Id, 0));
        TSet<int32> Visited;
        Visited.Add(StartRoom.Id);
        while (!Queue.IsEmpty()) {
            FIsaacRoomSearchNode Front;
            Queue.Dequeue(Front);

            if (Front.Distance > BestDistance) {
                BestDistance = Front.Distance;
                OutRoomA = StartRoom.Id;
                OutRoomB = Front.RoomId;
            }

            // Move to the child nodes
            const FIsaacRoom& FrontRoom = RoomMap[Front.RoomId];
            for (int32 ChildRoomId : FrontRoom.AdjacentRooms) {
                if (Visited.Contains(ChildRoomId)) {
                    continue;
                }
                Queue.Enqueue(FIsaacRoomSearchNode(ChildRoomId, Front.Distance + 1));
                Visited.Add(ChildRoomId);
            }
        }
    }
}

bool UIsaacDungeonQuery::ContainsDoorBetween(int32 RoomAId, int32 RoomBId) {
    FIsaacRoom RoomA = GetRoom(RoomAId);
    return RoomA.AdjacentRooms.Contains(RoomBId);
}

FVector UIsaacDungeonQuery::GetValidPlatformOnRoom(int32 RoomId) {
    FIsaacRoom Room = GetRoom(RoomId);

    FVector TileSize(Config->TileSize.X, Config->TileSize.Y, 0);
    FVector RoomSizeWorld = FVector(Config->RoomWidth, Config->RoomLength, 0) * TileSize;
    FVector RoomPadding(Config->RoomPadding.X, Config->RoomPadding.Y, 0);

    FVector RoomBasePosition = FMathUtils::ToVector(Room.Location) * (RoomSizeWorld + RoomPadding);

    TArray<FVector> FloorTiles;
    for (int x = 0; x < Room.Layout.Width; x++) {
        for (int y = 0; y < Room.Layout.Height; y++) {
            FIsaacRoomTile Tile = IsaacBuilderUtils::GetTileAt(x, y, Room);
            if (Tile.tileType == EIsaacRoomTileType::Floor) {
                FVector TilePosition(x + 0.5f, y + 0.5f, 0);
                FVector TileOffset = TilePosition * TileSize;
                FVector GroundPosition = RoomBasePosition + TileOffset;
                FloorTiles.Add(GroundPosition);
            }
        }
    }

    int32 NumTiles = FloorTiles.Num();
    if (NumTiles == 0) {
        return FVector::ZeroVector;
    }

    int32 RandomIndex = FMath::RandRange(0, NumTiles - 1);
    return FloorTiles[RandomIndex];
}

FVector UIsaacDungeonQuery::GetRandomValidPlatform() {
    if (!Model) {
        UE_LOG(LogIsaacDungeonQuery, Error, TEXT("GetRandomValidPlatform: Invalid model state"));
        return FVector::ZeroVector;
    }

    if (Model->Rooms.Num() == 0) {
        UE_LOG(LogIsaacDungeonQuery, Error, TEXT("GetRandomValidPlatform: No rooms exist. Please build first"));
        return FVector::ZeroVector;
    }

    int32 Index = FMath::RandRange(0, Model->Rooms.Num() - 1);
    const FIsaacRoom& Room = Model->Rooms[Index];
    return GetValidPlatformOnRoom(Room.Id);
}

