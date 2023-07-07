//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Builders/Grid/GridDungeonBuilder.h"
#include "GridCustomDungeonBuilder.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(GridCustomDungeonBuilderLog, Log, All);

struct FCustomGridRoomInfo {
    FCustomGridRoomInfo() : RoomId(-1) {
    }

    FRectangle Bounds;
    int32 RoomId;
};

/**
*
*/
UCLASS()
class DUNGEONARCHITECTRUNTIME_API UGridCustomDungeonBuilder : public UGridDungeonBuilder {
    GENERATED_BODY()

public:
    UGridCustomDungeonBuilder();

    UFUNCTION(BlueprintCallable, Category = Dungeon)
    void RegisterRoom(int32 X, int32 Y, int32 Z, int32 Width, int32 Height, int32& RoomID);

    UFUNCTION(BlueprintCallable, Category = Dungeon)
    void RegisterRoomAt(int32 X, int32 Y, int32 Z, int32& RoomID);

    UFUNCTION(BlueprintCallable, Category = Dungeon)
    void ConnectRooms(int32 Room1, int32 Room2);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = Dungeon)
    void GetRandomOffset(int32 X, int32 Y, float Radius, int32& OutX, int32& OutY);

    /** called when something enters the sphere component */
    UFUNCTION(BlueprintNativeEvent, Category = "Dungeon")
    void GenerateCustomLayout(UGridDungeonConfig* GridConfig);
    virtual void GenerateCustomLayout_Implementation(UGridDungeonConfig* GridConfig);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    bool bUseHeightVariation;

protected:
    virtual void BuildDungeonCells() override;
    virtual void GenerateRoomConnections() override;
    virtual void GenerateDungeonHeights() override;


private:
    TArray<FCustomGridRoomInfo> RoomBounds;
    TArray<FIntVector> RoomConnections;
};

