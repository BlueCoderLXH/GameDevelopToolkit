//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Builders/Isaac/RoomLayouts/IsaacRoomLayoutBuilder.h"
#include "StylizedIsaacRoomLayoutBuilder.generated.h"

UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API UStylizedIsaacRoomLayoutBuilder : public UIsaacRoomLayoutBuilder {
    GENERATED_UCLASS_BODY()

public:
    virtual FIsaacRoomLayout GenerateLayout(FIsaacRoomPtr room, FRandomStream& random, int roomWidth, int roomHeight) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LayoutBuilder)
    int32 minBrushSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LayoutBuilder)
    int32 maxBrushSize = 3;

private:
    void ConnectDoors(FIsaacRoomLayout& layout, const FIntVector& doorA, const FIntVector& doorB, int brushSize);
    void PaintTile(FIsaacRoomLayout& layout, int32 x, int32 y, int brushSize, EIsaacRoomTileType tileType);
    void SetTile(FIsaacRoomLayout& layout, int x, int y, int width, int height, EIsaacRoomTileType tileType);

};

