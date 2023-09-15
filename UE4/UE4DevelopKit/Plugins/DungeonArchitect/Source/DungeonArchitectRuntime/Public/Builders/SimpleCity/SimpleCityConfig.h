//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Builders/SimpleCity/SimpleCityModel.h"
#include "Core/DungeonConfig.h"
#include "SimpleCityConfig.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(SimpleCityConfigLog, Log, All);


UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API USimpleCityConfig : public UDungeonConfig {
    GENERATED_UCLASS_BODY()

public:

    /**
    * The size of each cell. Set this to the size of your modular asset
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    FVector2D CellSize;

    /**
    * The minimum size of the city in cell units
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    int32 MinCitySize;

    /**
    * The maximum size of the city in cell units
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    int32 MaxCitySize;

    /**
    * The minimum size of each block, in cell units
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    int32 MinBlockSize;

    /**
    * The minimum size of each block, in cell units
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    int32 MaxBlockSize;

    /**
    * The probability that a bigger house would spawn.  Valid range is from [0..1]
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    float BiggerHouseProbability;

    /**
    * Randomly removes certain straight lines of roads to create an uneven and more natural looking city (value between 0..1)
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    float RoadEdgeRemovalProbability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    TArray<FCityBlockDimension> CityBlockDimensions;
};

