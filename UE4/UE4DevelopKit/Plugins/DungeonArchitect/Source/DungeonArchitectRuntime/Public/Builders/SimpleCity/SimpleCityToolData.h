//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/DungeonToolData.h"
#include "SimpleCityToolData.generated.h"

UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API USimpleCityToolData : public UDungeonToolData {
    GENERATED_BODY()

public:

    // The cells painted by the "Paint" tool
    UPROPERTY()
    TArray<FIntVector> PaintedCells;

};

