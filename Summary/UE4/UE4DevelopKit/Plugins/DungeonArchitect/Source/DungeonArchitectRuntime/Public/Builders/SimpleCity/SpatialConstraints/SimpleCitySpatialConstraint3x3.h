//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Builders/SimpleCity/SpatialConstraints/SimpleCitySpatialConstraintCellData.h"
#include "Core/DungeonSpatialConstraint.h"
#include "SimpleCitySpatialConstraint3x3.generated.h"

USTRUCT(BlueprintType)
struct DUNGEONARCHITECTRUNTIME_API FSimpleCitySpatialConstraint3x3Data {
    GENERATED_USTRUCT_BODY()
    FSimpleCitySpatialConstraint3x3Data() {
        Cells.SetNum(9);
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Setup")
    TArray<FSimpleCitySpatialConstraintCellData> Cells;
};


UCLASS(DefaultToInstanced, Blueprintable)
class DUNGEONARCHITECTRUNTIME_API USimpleCitySpatialConstraint3x3 : public UDungeonSpatialConstraint {
    GENERATED_UCLASS_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spatial Setup")
    FSimpleCitySpatialConstraint3x3Data Configuration;
};

