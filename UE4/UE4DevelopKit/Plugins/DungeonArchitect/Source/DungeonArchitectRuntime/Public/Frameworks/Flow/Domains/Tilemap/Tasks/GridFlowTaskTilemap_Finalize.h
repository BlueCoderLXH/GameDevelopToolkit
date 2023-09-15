//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/Domains/Tilemap/Tasks/GridFlowTaskTilemapBase.h"
#include "GridFlowTaskTilemap_Finalize.generated.h"

struct FGridFlowTilemapCell;
class UGridFlowTilemap;
class UGridFlowAbstractGraph;

UCLASS(Meta = (TilemapTask, Title = "Finalize Tilemap", Tooltip = "Finalize Tilemap", MenuPriority = 2600))
class DUNGEONARCHITECTRUNTIME_API UGridFlowTaskTilemap_Finalize : public UGridFlowTaskTilemapBase {
    GENERATED_BODY()
public:
    /**
        Debug: Show tiles that are not reachable due to overlays blocking them

        Variable Name: [N/A]
    */
    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDebugUnwalkableCells = false;

public:
    virtual void Execute(const FFlowExecutionInput& Input, const FFlowTaskExecutionSettings& InExecSettings, FFlowExecutionOutput& Output) override;

private:
    TArray<FGridFlowTilemapCell*> FilterWalkablePath(const TArray<FGridFlowTilemapCell*>& Cells) const;
    void UpdateLayoutTilemapMetadata(UGridFlowAbstractGraph* InGraph, UGridFlowTilemap* InTilemap) const;
};

