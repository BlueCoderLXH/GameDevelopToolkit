//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/DungeonModel.h"
#include "IsaacDungeonModel.generated.h"

UENUM(BlueprintType)
enum class EIsaacRoomType : uint8 {
    Normal = 0 UMETA(DisplayName = "Normal"),
    Spawn = 1 UMETA(DisplayName = "Spawn"),
    Treasure = 2 UMETA(DisplayName = "Treasure"),
    Boss = 4 UMETA(DisplayName = "Boss"),
    Exit = 8 UMETA(DisplayName = "Exit")
};


UENUM(BlueprintType)
enum class EIsaacRoomTileType : uint8 {
    Floor UMETA(DisplayName = "Floor"),
    Door UMETA(DisplayName = "Door"),
    Empty UMETA(DisplayName = "Empty")
};

USTRUCT(Blueprintable)
struct DUNGEONARCHITECTRUNTIME_API FIsaacRoomTile {
    GENERATED_USTRUCT_BODY()

    UPROPERTY()
    EIsaacRoomTileType tileType;
};


USTRUCT(Blueprintable)
struct DUNGEONARCHITECTRUNTIME_API FIsaacRoomLayout {
    GENERATED_USTRUCT_BODY()

    UPROPERTY()
    TArray<FIsaacRoomTile> Tiles;

    UPROPERTY()
    int32 Width;

    UPROPERTY()
    int32 Height;

    void Initialize(int32 InWidth, int32 InHeight, EIsaacRoomTileType TileType) {
        this->Width = InWidth;
        this->Height = InHeight;
        int size = Width * Height;
        Tiles.Reset();
        Tiles.AddUninitialized(size);

        for (int i = 0; i < size; i++) {
            Tiles[i] = FIsaacRoomTile();
            Tiles[i].tileType = TileType;
        }
    }
};

USTRUCT(Blueprintable)
struct DUNGEONARCHITECTRUNTIME_API FIsaacRoom {
    GENERATED_USTRUCT_BODY()

    FIsaacRoom()
        : Id(0)
          , Location(FIntVector::ZeroValue)
          , RoomType(EIsaacRoomType::Normal) {
    }

    FIsaacRoom(int32 x, int32 y)
        : Id(0)
          , RoomType(EIsaacRoomType::Normal) {
        Location.X = x;
        Location.Y = y;
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    int32 Id;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    FIntVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    EIsaacRoomType RoomType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    FIsaacRoomLayout Layout;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    TArray<int32> AdjacentRooms;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    TArray<FIntVector> DoorPositions;

    FORCEINLINE bool operator==(const FIsaacRoom& other) const {
        return other.Id == Id;
    }

};

typedef TSharedPtr<FIsaacRoom> FIsaacRoomPtr;

USTRUCT(Blueprintable)
struct DUNGEONARCHITECTRUNTIME_API FIsaacDoor {
    GENERATED_USTRUCT_BODY()
    FIsaacDoor() : roomA(-1), roomB(-1), ratio(0.5f) {
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    int32 roomA;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    int32 roomB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    float ratio;
};

typedef TSharedPtr<FIsaacDoor> FIsaacDoorPtr;

/**
	* 
	*/
UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API UIsaacDungeonModel : public UDungeonModel {
    GENERATED_BODY()
public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    TArray<FIsaacRoom> Rooms;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    TArray<FIsaacDoor> Doors;

    UFUNCTION(BlueprintCallable, Category = Dungeon)
    void RemoveStylingFromRoom(int32 RoomId);

    virtual void Reset() override;
};

