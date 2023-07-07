//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/Isaac/IsaacDungeonTransformLogic.h"

#include "Builders/Isaac/IsaacDungeonModel.h"

void UIsaacDungeonTransformLogic::GetNodeOffset_Implementation(UIsaacDungeonModel* Model, FTransform& Offset) {
    Offset = FTransform::Identity;
}

