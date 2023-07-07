//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/Grid/GridDungeonTransformLogic.h"


void UGridDungeonTransformLogic::GetNodeOffset_Implementation(UGridDungeonModel* Model, UGridDungeonConfig* Config,
                                                              UGridDungeonBuilder* Builder, UGridDungeonQuery* Query,
                                                              const FCell& Cell, const FRandomStream& RandomStream,
                                                              int32 GridX, int32 GridY,
                                                              const FTransform& MarkerTransform, FTransform& Offset) {
    Offset = FTransform::Identity;
}

