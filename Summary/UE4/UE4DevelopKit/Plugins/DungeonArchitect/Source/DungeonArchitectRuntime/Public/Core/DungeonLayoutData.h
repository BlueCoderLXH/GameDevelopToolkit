//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

struct DUNGEONARCHITECTRUNTIME_API FDungeonLayoutDataBlockItem {
    int32 ItemId = 0;
    int32 ItemCategory = 0;
    TArray<FVector2D> Outline;
    TArray<FVector2D> FillTriangles;
};

struct DUNGEONARCHITECTRUNTIME_API FDungeonLayoutDataDoorItem {
    TArray<FVector2D> Outline;
};

struct DUNGEONARCHITECTRUNTIME_API FDungeonLayoutDataPointOfInterest {
    FName Id;
    FName Caption;
    FVector2D Location;
};

class DUNGEONARCHITECTRUNTIME_API FDungeonLayoutData {
public:

    void AddQuadItem(const FVector2D& InLocation, const FVector2D& InSize, int32 ItemId = 0, int32 ItemCategory = 0);

public:
    TArray<FDungeonLayoutDataBlockItem> LayoutItems;
    TArray<FDungeonLayoutDataDoorItem> Doors;
    TArray<FDungeonLayoutDataPointOfInterest> PointsOfInterest;
    FTransform WorldToScreen = FTransform::Identity;
};

