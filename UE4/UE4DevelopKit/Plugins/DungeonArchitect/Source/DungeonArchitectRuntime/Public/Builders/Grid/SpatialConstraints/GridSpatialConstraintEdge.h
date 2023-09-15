//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Builders/Grid/SpatialConstraints/GridSpatialConstraintCellData.h"
#include "Core/DungeonSpatialConstraint.h"
#include "GridSpatialConstraintEdge.generated.h"

USTRUCT(BlueprintType)
struct DUNGEONARCHITECTRUNTIME_API FGridSpatialConstraintEdgeData {
    GENERATED_USTRUCT_BODY()
    FGridSpatialConstraintEdgeData() {
        Cells.SetNum(2);
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Setup")
    TArray<FGridSpatialConstraintCellData> Cells;
};


UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API UGridSpatialConstraintEdge : public UDungeonSpatialConstraint {
    GENERATED_UCLASS_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Setup")
    FGridSpatialConstraintEdgeData Configuration;
};

