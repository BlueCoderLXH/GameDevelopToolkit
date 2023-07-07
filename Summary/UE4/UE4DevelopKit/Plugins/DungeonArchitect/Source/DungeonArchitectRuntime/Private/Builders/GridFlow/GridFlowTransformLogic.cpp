//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/GridFlow/GridFlowTransformLogic.h"

#include "Builders/GridFlow/GridFlowModel.h"
#include "Builders/GridFlow/GridFlowQuery.h"

void UGridFlowTransformLogic::GetNodeOffset_Implementation(UGridFlowModel* Model, UGridFlowConfig* Config,
                                                           UGridFlowQuery* Query, const FRandomStream& RandomStream,
                                                           int32 GridX, int32 GridY, FTransform& Offset) {
    Offset = FTransform::Identity;
}

