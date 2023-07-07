//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Builders/Grid/GridDungeonModel.h"
#include "Builders/Grid/GridDungeonToolData.h"
#include "Core/Utils/DungeonModelHelper.h"
#include "GridDungeonModelHelper.generated.h"

class ADungeon;
struct FGridToolPaintStrokeData;

DECLARE_LOG_CATEGORY_EXTERN(DungeonModelHelperLog, Log, All);

/**
 * 
 */
UCLASS()
class DUNGEONARCHITECTRUNTIME_API UGridDungeonModelHelper : public UDungeonModelHelper {
    GENERATED_UCLASS_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = Dungeon)
    static void GetCellConnectedRooms(const FCell& Cell, TArray<int32>& ConnectedRooms);

    UFUNCTION(BlueprintCallable, Category = Dungeon)
    static void GetCellMSTRooms(const FCell& Cell, TArray<int32>& MSTRooms);

    UFUNCTION(BlueprintCallable, Category = Dungeon)
    static void GetCellCenter(const FCell& Cell, FVector& Center);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    static void ToWorldCoords(const FRectangle& Bounds, const FVector& GridSize, FVector& Location, FVector& Size);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    static void GetBoundingBox(const TArray<FCell>& Cells, FRectangle& Bounds);

    /** Tries to find a free edge in the room bounds that does not contain a door */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    static void GetRoomFreeEdge(UGridDungeonModel* Model, const FCell& RoomCell, TArray<FVector>& FreeEdgeCenters,
                                TArray<float>& FreeEdgeAngles);

    UFUNCTION(BlueprintCallable, Category = Dungeon)
    static void GetDoorExits(const FCellDoor& Door, FVector& ExitA, FVector& ExitB);

    UFUNCTION(BlueprintCallable, Category = Dungeon)
    static FVector GetRandomCellLocation(UGridDungeonModel* Model, UGridDungeonConfig* Config);

    UFUNCTION(BlueprintCallable, Category = Dungeon)
    static void AddPaintCell(const FGridToolPaintStrokeData& CellData, ADungeon* Dungeon, bool bAutomaticRebuild);

    UFUNCTION(BlueprintCallable, Category = Dungeon)
    static void RemovePaintCell(const FGridToolPaintStrokeData& CellData, ADungeon* Dungeon, bool bAutomaticRebuild);

};

