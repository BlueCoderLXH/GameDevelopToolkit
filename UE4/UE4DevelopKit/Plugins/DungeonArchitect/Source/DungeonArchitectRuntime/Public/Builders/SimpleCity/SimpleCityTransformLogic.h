//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/ThemeEngine/Rules/DungeonTransformLogic.h"
#include "SimpleCityTransformLogic.generated.h"

struct FCell;
class USimpleCityModel;

/**
*
*/
UCLASS(Blueprintable, HideDropDown)
class DUNGEONARCHITECTRUNTIME_API USimpleCityTransformLogic : public UDungeonTransformLogic {
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintNativeEvent, Category = "Dungeon")
    void GetNodeOffset(USimpleCityModel* Model, FTransform& Offset);
    virtual void GetNodeOffset_Implementation(USimpleCityModel* Model, FTransform& Offset);

};

