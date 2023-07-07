//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/ThemeEngine/Rules/DungeonTransformLogic.h"
#include "GridFlowTransformLogic.generated.h"

struct FCell;
class UGridFlowModel;
class UGridFlowQuery;

/**
*
*/
UCLASS(Blueprintable, HideDropDown)
class DUNGEONARCHITECTRUNTIME_API UGridFlowTransformLogic : public UDungeonTransformLogic {
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintNativeEvent, Category = "Dungeon")
    void GetNodeOffset(UGridFlowModel* Model, UGridFlowConfig* Config, UGridFlowQuery* Query,
                       const FRandomStream& RandomStream, int32 GridX, int32 GridY, FTransform& Offset);
    virtual void GetNodeOffset_Implementation(UGridFlowModel* Model, UGridFlowConfig* Config, UGridFlowQuery* Query,
                                              const FRandomStream& RandomStream, int32 GridX, int32 GridY,
                                              FTransform& Offset);

};

