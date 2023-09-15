//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Builders/Grid/GridDungeonConfig.h"
#include "Builders/Grid/GridDungeonModel.h"
#include "Core/DungeonQuery.h"
#include "GridDungeonQuery.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGridDungeonQuery, Log, All);

class UDungeonBuilder;

/**
*
*/
UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API UGridDungeonQuery : public UDungeonQuery {
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    TArray<int32> GetCells();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    TArray<int32> GetCellsOfType(FCellType CellType);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    void GetCellDimension(int32 CellId, FVector& OutCenter, FVector& OutSize);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    void GetPathBetweenCells(int32 CellA, int32 CellB, TArray<int32>& OutResult, bool& bOutSuccess);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    void GetFurthestRooms(int32& OutRoomA, int32& OutRoomB);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    void GetCellAtLocation(const FVector& WorldLocation, int32& OutCellId, bool& bOutValid);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    FCellType GetCellType(int32 CellId);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    int32 GetRandomCell();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    int32 GetRandomCellOfType(FCellType CellType);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    int32 GetRandomCellFromStream(FRandomStream& RandomStream);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    int32 GetRandomCellOfTypeFromStream(FCellType CellType, FRandomStream& RandomStream);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    bool ContainsStairBetween(int32 CellA, int32 CellB);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    bool ContainsDoorBetween(int32 CellA, int32 CellB);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    FStairInfo GetStairBetween(int32 CellA, int32 CellB);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    FCellDoor GetDoorBetween(int32 CellA, int32 CellB);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    FVector GetOpeningPointBetweenAdjacentCells(int32 CellA, int32 CellB);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    void GetAdjacentCellsOnEdge(const FTransform& WallMarkerTransform, int32& CellA, int32& CellB);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    void IsNearMarker(const FTransform& CurrentMarkerTransform, const FString& NearbyMarkerName, float NearbyDistance,
                      UDungeonBuilder* Builder, bool& bIsNear, int32& NumFound);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    void GetAdjacentCells(int32 CellId, TArray<int32>& OutAdjacentCells);


private:
    FCell GetCell(int32 CellId);

    /** Generates a cell lookup.  This is a copy and modifying the cells will not change the original values! */
    void GenerateCellLookup(TMap<int32, FCell>& OutLookup);
    void GetFurthestRoomsRecursive(const struct LongestPathSearchNode& Top, int32 StartRoomId,
                                   TMap<int32, FCell>& CellLookup,
                                   const FVector& GridSize, TSet<int>& Visited, int32& OutBestStartRoom,
                                   int32& OutBestEndRoom, int32& OutBestDistance);

protected:
    virtual void InitializeImpl(UDungeonConfig* InConfig, UDungeonModel* InModel) override;

private:
    UPROPERTY()
    UGridDungeonConfig* Config;

    UPROPERTY()
    UGridDungeonModel* Model;

};

