//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/Domains/Tilemap/Tasks/GridFlowTaskTilemapBase.h"
#include "GridFlowTaskTilemap_Optimize.generated.h"

UCLASS(Meta = (TilemapTask, Title = "Optimize Tilemap", Tooltip = "Optimize Tilemap", MenuPriority = 2500))
class DUNGEONARCHITECTRUNTIME_API UGridFlowTaskTilemap_Optimize : public UGridFlowTaskTilemapBase {
    GENERATED_BODY()

public:
    /**
        If the distance of a tile from the nearest layout tile is
        greater than the specified amount, it is discarded

        Variable Name: DiscardDistanceFromLayout
    */
    UPROPERTY(EditAnywhere, Category = "Optimize")
    int32 DiscardDistanceFromLayout = 3;

public:
    virtual void Execute(const FFlowExecutionInput& Input, const FFlowTaskExecutionSettings& InExecSettings, FFlowExecutionOutput& Output) override;
    virtual bool GetParameter(const FString& InParameterName, FDAAttribute& OutValue) override;
    virtual bool SetParameter(const FString& InParameterName, const FDAAttribute& InValue) override;
    virtual bool SetParameterSerialized(const FString& InParameterName, const FString& InSerializedText) override;

private:
    void DiscardDistantTiles(UGridFlowTilemap* Tilemap);
};

