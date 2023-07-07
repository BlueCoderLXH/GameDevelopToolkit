//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/FloorPlan/FloorPlanTransformLogic.h"

#include "Builders/FloorPlan/FloorPlanModel.h"

void UFloorPlanTransformLogic::GetNodeOffset_Implementation(UFloorPlanModel* Model, UFloorPlanConfig* Config,
                                                            const FRandomStream& RandomStream, int32 GridX, int32 GridY,
                                                            FTransform& Offset) {
    Offset = FTransform::Identity;
}

