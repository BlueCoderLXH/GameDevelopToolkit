//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/ThemeEngine/Rules/DungeonTransformLogic.h"
#include "FloorPlanTransformLogic.generated.h"

struct FCell;
class UFloorPlanModel;

/**
*
*/
UCLASS(Blueprintable, HideDropDown)
class DUNGEONARCHITECTRUNTIME_API UFloorPlanTransformLogic : public UDungeonTransformLogic {
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintNativeEvent, Category = "Dungeon")
    void GetNodeOffset(UFloorPlanModel* Model, UFloorPlanConfig* Config, const FRandomStream& RandomStream, int32 GridX,
                       int32 GridY, FTransform& Offset);
    virtual void GetNodeOffset_Implementation(UFloorPlanModel* Model, UFloorPlanConfig* Config,
                                              const FRandomStream& RandomStream, int32 GridX, int32 GridY,
                                              FTransform& Offset);

};

