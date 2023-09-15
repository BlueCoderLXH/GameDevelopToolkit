//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "SimpleCitySpatialConstraintCellData.generated.h"

UENUM(BlueprintType)
enum class ESimpleCitySpatialCellOccupation : uint8 {
    Ignore = 0 UMETA(DisplayName = "Ignore"),
    Road = 1 UMETA(DisplayName = "Road"),
    House = 2 UMETA(DisplayName = "House"),
    Park = 3 UMETA(DisplayName = "Park"),
    Outskirts = 4 UMETA(DisplayName = "Outskirts")
};

USTRUCT(BlueprintType)
struct DUNGEONARCHITECTRUNTIME_API FSimpleCitySpatialConstraintCellData {
    GENERATED_USTRUCT_BODY()

    FSimpleCitySpatialConstraintCellData() : OccupationConstraint(ESimpleCitySpatialCellOccupation::Ignore) {
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Setup")
    ESimpleCitySpatialCellOccupation OccupationConstraint;
};

