//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Builders/Isaac/RoomLayouts/IsaacRoomLayoutBuilder.h"
#include "SimpleIsaacRoomLayoutBuilder.generated.h"

UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API USimpleIsaacRoomLayoutBuilder : public UIsaacRoomLayoutBuilder {
    GENERATED_BODY()
public:
    virtual FIsaacRoomLayout GenerateLayout(FIsaacRoomPtr room, FRandomStream& random, int roomWidth, int roomHeight) override;

};

