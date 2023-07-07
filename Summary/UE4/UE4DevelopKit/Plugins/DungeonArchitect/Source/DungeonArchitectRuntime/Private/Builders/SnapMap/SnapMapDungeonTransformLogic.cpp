//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/SnapMap/SnapMapDungeonTransformLogic.h"

#include "Builders/SnapMap/SnapMapDungeonModel.h"

void USnapMapDungeonTransformLogic::GetNodeOffset_Implementation(USnapMapDungeonModel* Model, FTransform& Offset) {
    Offset = FTransform::Identity;
}

