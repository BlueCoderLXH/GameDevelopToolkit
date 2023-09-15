//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Builders/Grid/SpatialConstraints/GridSpatialConstraintCellData.h"
#include "Core/DungeonSpatialConstraint.h"
#include "GridSpatialConstraint2x2.generated.h"

USTRUCT(BlueprintType)
struct DUNGEONARCHITECTRUNTIME_API FGridSpatialConstraint2x2Data {
    GENERATED_USTRUCT_BODY()
    FGridSpatialConstraint2x2Data() {
        Cells.SetNum(4);
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Setup")
    TArray<FGridSpatialConstraintCellData> Cells;
};


UCLASS(DefaultToInstanced, Blueprintable)
class DUNGEONARCHITECTRUNTIME_API UGridSpatialConstraint2x2 : public UDungeonSpatialConstraint {
    GENERATED_UCLASS_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Setup")
    FGridSpatialConstraint2x2Data Configuration;
};

