//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/DungeonModel.h"
#include "Core/Utils/Rectangle.h"
#include "GridDungeonModel.generated.h"

UENUM(BlueprintType)
enum class DungeonModelBuildState : uint8 {
    Initial UMETA(DisplayName = "Initial"),
    Separation UMETA(DisplayName = "Separation"),
    Triangulation UMETA(DisplayName = "Triangulation"),
    SpanningTree UMETA(DisplayName = "SpanningTree"),
    Corridors UMETA(DisplayName = "Corridors"),
    Complete UMETA(DisplayName = "Complete")
};

UENUM(BlueprintType)
enum class FCellType : uint8 {
    Room UMETA(DisplayName = "Room"),
    Corridor UMETA(DisplayName = "Corridor"),
    CorridorPadding UMETA(DisplayName = "CorridorPadding"),
    Unknown UMETA(DisplayName = "Unknown")
};


USTRUCT(Blueprintable)
struct DUNGEONARCHITECTRUNTIME_API FCell {
    GENERATED_USTRUCT_BODY()

    FCell() : Id(0), CellType(FCellType::Unknown), UserDefined(false), ClusterId(-1) {
    }

    FCell(int32 x, int32 y, int32 width, int32 height) : CellType(FCellType::Unknown) {
        Bounds.Location.X = x;
        Bounds.Location.Y = y;
        Bounds.Size.X = width;
        Bounds.Size.Y = height;
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    int32 Id;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    FRectangle Bounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    FCellType CellType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    bool UserDefined;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    int32 ClusterId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    TArray<int32> ConnectedRooms;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    TArray<int32> FixedRoomConnections;

    UPROPERTY()
    TArray<int32> AdjacentCells;

    FIntVector Center() const {
        return Bounds.Center();
    }

    FORCEINLINE bool operator==(const FCell& other) const {
        return other.Id == Id;
    }

};

USTRUCT(Blueprintable)
struct DUNGEONARCHITECTRUNTIME_API FCellDoor {
    GENERATED_USTRUCT_BODY()

    FCellDoor() : bEnabled(true) {
    }

    FIntVector AdjacentTiles[2];
    int AdjacentCells[2];
    bool bEnabled;

};

bool operator==(const FCellDoor& A, const FCellDoor& B);

/* Holds metadata about each occupied grid in the cell for quick reference from a lookup */
USTRUCT(Blueprintable)
struct DUNGEONARCHITECTRUNTIME_API FGridCellInfo {
    GENERATED_USTRUCT_BODY()
    FGridCellInfo() : CellId(0), CellType(FCellType::Unknown), ContainsDoor(false) {
    }

    FGridCellInfo(int32 pCellId, FCellType pCellType) : CellId(pCellId), CellType(pCellType), ContainsDoor(false) {
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    int32 CellId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    FCellType CellType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    bool ContainsDoor;
};

class DUNGEONARCHITECTRUNTIME_API FDoorManager {
public:
    FCellDoor CreateDoor(const FIntVector& p1, const FIntVector& p2, int cellId1, int cellId2) {
        if (doorLookupCache.Contains(p1) && doorLookupCache[p1].Contains(p2)) return doorLookupCache[p1][p2];
        if (doorLookupCache.Contains(p2) && doorLookupCache[p2].Contains(p1)) return doorLookupCache[p2][p1];

        // Create a new door
        FCellDoor door;
        door.AdjacentTiles[0] = p1;
        door.AdjacentTiles[1] = p2;
        door.AdjacentCells[0] = cellId1;
        door.AdjacentCells[1] = cellId2;

        if (!doorLookupCache.Contains(p1)) doorLookupCache.Add(p1, TMap<FIntVector, FCellDoor>());
        if (!doorLookupCache.Contains(p2)) doorLookupCache.Add(p2, TMap<FIntVector, FCellDoor>());

        if (!doorLookupCache[p1].Contains(p2)) doorLookupCache[p1].Add(p2, door);
        if (!doorLookupCache[p2].Contains(p1)) doorLookupCache[p2].Add(p1, door);

        doors.Add(door);
        return door;
    }

    FORCEINLINE const TArray<FCellDoor>& GetDoors() const { return doors; }
    FORCEINLINE TArray<FCellDoor>& GetDoors() { return doors; }

    void RemoveDoor(const FCellDoor& Door) {
        doors.Remove(Door);

        // Remove it from the cache

        TArray<FIntVector> Keys1, Keys2;
        doorLookupCache.GenerateKeyArray(Keys1);
        for (const FIntVector& Key1 : Keys1) {
            doorLookupCache[Key1].GenerateKeyArray(Keys2);
            for (const FIntVector& Key2 : Keys2) {
                FCellDoor TestDoor = doorLookupCache[Key1][Key2];
                if (TestDoor == Door) {
                    doorLookupCache[Key1].Remove(Key2);
                }
            }
        }
    }

    bool ContainsDoorBetweenCells(int cell0, int cell1) {
        for (const FCellDoor& door : doors) {
            if (!door.bEnabled) { continue; }
            if ((door.AdjacentCells[0] == cell0 && door.AdjacentCells[1] == cell1) ||
                (door.AdjacentCells[0] == cell1 && door.AdjacentCells[1] == cell0)) {
                return true;
            }
        }
        return false;
    }

    bool ContainsDoor(int x1, int y1, int x2, int y2) {
        for (const auto& door : doors) {
            if (!door.bEnabled) { continue; }
            bool containsDoor =
                door.AdjacentTiles[0].X == x1 && door.AdjacentTiles[0].Y == y1 &&
                door.AdjacentTiles[1].X == x2 && door.AdjacentTiles[1].Y == y2;
            if (containsDoor) {
                return true;
            }

            containsDoor =
                door.AdjacentTiles[1].X == x1 && door.AdjacentTiles[1].Y == y1 &&
                door.AdjacentTiles[0].X == x2 && door.AdjacentTiles[0].Y == y2;
            if (containsDoor) {
                return true;
            }
        }
        return false;
    }

private:
    TMap<FIntVector, TMap<FIntVector, FCellDoor>> doorLookupCache;
    TArray<FCellDoor> doors;

};

USTRUCT(Blueprintable)
struct DUNGEONARCHITECTRUNTIME_API FStairInfo {
    GENERATED_USTRUCT_BODY()

    UPROPERTY(BlueprintReadOnly, Category = Dungeon)
    int32 OwnerCell;

    UPROPERTY(BlueprintReadOnly, Category = Dungeon)
    int32 ConnectedToCell;

    UPROPERTY(BlueprintReadOnly, Category = Dungeon)
    FVector Position;

    UPROPERTY(BlueprintReadOnly, Category = Dungeon)
    FQuat Rotation;

    UPROPERTY(BlueprintReadOnly, Category = Dungeon)
    FIntVector IPosition;
};

/**
	* 
	*/
UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API UGridDungeonModel : public UDungeonModel {
    GENERATED_BODY()
public:

    FDoorManager DoorManager;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Dungeon)
    DungeonModelBuildState BuildState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    TArray<FCell> Cells;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    TArray<FCellDoor> Doors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    TArray<FStairInfo> Stairs;

    void BuildCellLookup();

    FCell* GetCell(int32 Id) const {
        return CellLookup.Contains(Id) ? CellLookup[Id] : nullptr;
    }

    bool ContainsStairAtLocation(int x, int y);
    bool ContainsStairBetweenCells(int32 CellIdA, int32 CellIdB);

    TMap<int32, TArray<FStairInfo>> CellStairsLookup;
    TMap<int32, FCell*> CellLookup;
    TMap<int32, TMap<int32, FGridCellInfo>> GridCellInfoLookup;

    UFUNCTION(BlueprintCallable, Category = Dungeon)
    FGridCellInfo GetGridCellLookup(int32 x, int32 y) const;

    virtual void Cleanup() override;
    virtual void Reset() override;
};

