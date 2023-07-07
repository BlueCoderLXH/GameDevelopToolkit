//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/SimpleCity/SimpleCityTransformLogic.h"

#include "Builders/SimpleCity/SimpleCityModel.h"

void USimpleCityTransformLogic::GetNodeOffset_Implementation(USimpleCityModel* Model, FTransform& Offset) {
    Offset = FTransform::Identity;
}

