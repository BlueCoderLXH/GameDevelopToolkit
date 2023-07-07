//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/ThemeEngine/Rules/DungeonTransformLogic.h"
#include "SnapMapDungeonTransformLogic.generated.h"

struct FCell;
class USnapMapDungeonModel;

/**
*
*/
UCLASS(Blueprintable, HideDropDown)
class DUNGEONARCHITECTRUNTIME_API USnapMapDungeonTransformLogic : public UDungeonTransformLogic {
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintNativeEvent, Category = "Dungeon")
    void GetNodeOffset(USnapMapDungeonModel* Model, FTransform& Offset);
    virtual void GetNodeOffset_Implementation(USnapMapDungeonModel* Model, FTransform& Offset);

};

