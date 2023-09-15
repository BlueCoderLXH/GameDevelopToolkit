//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Builders/Isaac/IsaacDungeonModel.h"
#include "IsaacRoomLayoutBuilder.generated.h"

UCLASS(EditInlineNew, DefaultToInstanced, BlueprintType, Blueprintable, abstract, HideDropDown)
class DUNGEONARCHITECTRUNTIME_API UIsaacRoomLayoutBuilder : public UObject {
    GENERATED_BODY()
public:
    virtual FIsaacRoomLayout GenerateLayout(FIsaacRoomPtr room, FRandomStream& random, int roomWidth, int roomHeight);

};

