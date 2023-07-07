//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/MiniMap/DungeonMiniMap.h"
#include "GridFlowMiniMap.generated.h"

UCLASS()
class DUNGEONARCHITECTRUNTIME_API AGridFlowMiniMap : public ADungeonMiniMap {
    GENERATED_BODY()

public:
    virtual void BuildLayout(UDungeonModel* DungeonModel, UDungeonConfig* DungeonConfig) override;

private:

};

