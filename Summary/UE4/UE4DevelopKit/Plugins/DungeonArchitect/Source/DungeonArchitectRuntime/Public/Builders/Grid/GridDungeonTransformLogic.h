//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Builders/Grid/GridDungeonModel.h"
#include "Frameworks/ThemeEngine/Rules/DungeonTransformLogic.h"
#include "GridDungeonTransformLogic.generated.h"

struct FCell;
class UGridDungeonModel;
class UGridDungeonConfig;
class UGridDungeonBuilder;
class UGridDungeonQuery;

/**
*
*/
UCLASS(Blueprintable, HideDropDown)
class DUNGEONARCHITECTRUNTIME_API UGridDungeonTransformLogic : public UDungeonTransformLogic {
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintNativeEvent, Category = "Dungeon")
    void GetNodeOffset(UGridDungeonModel* Model, UGridDungeonConfig* Config, UGridDungeonBuilder* Builder,
                       UGridDungeonQuery* Query, const FCell& Cell, const FRandomStream& RandomStream, int32 GridX,
                       int32 GridY, const FTransform& MarkerTransform, FTransform& Offset);
    virtual void GetNodeOffset_Implementation(UGridDungeonModel* Model, UGridDungeonConfig* Config,
                                              UGridDungeonBuilder* Builder, UGridDungeonQuery* Query, const FCell& Cell,
                                              const FRandomStream& RandomStream, int32 GridX, int32 GridY,
                                              const FTransform& MarkerTransform, FTransform& Offset);

};

