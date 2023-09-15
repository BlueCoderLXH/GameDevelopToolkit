//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/Domains/Tilemap/Tasks/GridFlowTaskTilemapBase.h"
#include "GridFlowTaskTilemap_Merge.generated.h"

UCLASS(Meta = (TilemapTask, Title = "Merge Tilemaps", Tooltip = "Merge Tilemaps", MenuPriority = 2400))
class DUNGEONARCHITECTRUNTIME_API UGridFlowTaskTilemap_Merge : public UGridFlowTaskTilemapBase {
    GENERATED_BODY()
public:
    virtual void Execute(const FFlowExecutionInput& Input, const FFlowTaskExecutionSettings& InExecSettings, FFlowExecutionOutput& Output) override;
};

