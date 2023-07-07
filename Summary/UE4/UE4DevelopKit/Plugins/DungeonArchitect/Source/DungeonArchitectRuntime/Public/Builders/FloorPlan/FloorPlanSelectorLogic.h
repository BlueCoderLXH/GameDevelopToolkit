//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Builders/FloorPlan/FloorPlanModel.h"
#include "Frameworks/ThemeEngine/Rules/DungeonSelectorLogic.h"
#include "FloorPlanSelectorLogic.generated.h"

/**
*
*/
UCLASS(Blueprintable, HideDropDown)
class DUNGEONARCHITECTRUNTIME_API UFloorPlanSelectorLogic : public UDungeonSelectorLogic {
    GENERATED_BODY()

public:

    /** called when something enters the sphere component */
    UFUNCTION(BlueprintNativeEvent, Category = "Dungeon")
    bool SelectNode(UFloorPlanModel* Model, UFloorPlanConfig* Config, const FRandomStream& RandomStream, int32 GridX,
                    int32 GridY, int32 GridZ);
    virtual bool SelectNode_Implementation(UFloorPlanModel* Model, UFloorPlanConfig* Config,
                                           const FRandomStream& RandomStream, int32 GridX, int32 GridY, int32 GridZ);


};

