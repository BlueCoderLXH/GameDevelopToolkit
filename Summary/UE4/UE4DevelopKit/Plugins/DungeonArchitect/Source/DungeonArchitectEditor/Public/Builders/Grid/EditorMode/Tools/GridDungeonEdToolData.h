//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Builders/Grid/GridDungeonToolData.h"
#include "Core/Dungeon.h"
#include "GridDungeonEdToolData.generated.h"

UCLASS()
class UGridDungeonEdToolData : public UObject {
    GENERATED_BODY()
public:
    UGridDungeonEdToolData(const FObjectInitializer& ObjectInitializer);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Edit Mode")
    int32 FloorIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Edit Mode")
    int32 BrushSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Edit Mode")
    EGridPaintToolCellType CellType;
};

