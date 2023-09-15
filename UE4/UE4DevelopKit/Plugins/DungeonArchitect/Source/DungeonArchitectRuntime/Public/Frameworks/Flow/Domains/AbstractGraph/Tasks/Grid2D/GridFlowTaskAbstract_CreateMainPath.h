//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Common/FlowTaskAbstract_CreateMainPath.h"
#include "GridFlowTaskAbstract_CreateMainPath.generated.h"

UCLASS(Meta = (AbstractTask, Title = "Create Main Path", Tooltip = "Create a main path with spawn and goal", MenuPriority = 1100))
class DUNGEONARCHITECTRUNTIME_API UGridFlowTaskAbstract_CreateMainPath : public UFlowTaskAbstract_CreateMainPath {
    GENERATED_BODY()
public:
    
    /**
        Enable this to override the start node position

        Variable Name: bOverrideStartGridPosition
    */
    UPROPERTY(EditAnywhere, Category = "Start Node")
    bool bOverrideStartGridPosition = false;

    /**
        Override the start node position of the path.   
        This is useful if you want static entrances to your dungeon
        This parameter requires bOverrideStartNodePosition flag to be set

        Variable Name: StartGridPosition
    */
    UPROPERTY(EditAnywhere, Category = "Start Node", Meta=(EditCondition = "bOverrideStartGridPosition"))
    FIntPoint StartGridPosition;

    /**
        The room in the tilemap generate from the start node will have it's full size
        (perturb value for this node will be disable)

        Variable Name: bFullSizeStartRoom
    */
    UPROPERTY(EditAnywhere, Category = "Start Node")
    bool bFullSizeStartRoom = false;

    /**
        The room in the tilemap generate from the goal node (last node in the path) will
        have it's full size (perturb value for this node will be disable)

        Variable Name: bFullSizeGoalRoom
    */
    UPROPERTY(EditAnywhere, Category = "Start Node")
    bool bFullSizeGoalRoom = false;

public:
    virtual bool GetParameter(const FString& InParameterName, FDAAttribute& OutValue) override;
    virtual bool SetParameter(const FString& InParameterName, const FDAAttribute& InValue) override;
    virtual bool SetParameterSerialized(const FString& InParameterName, const FString& InSerializedText) override;

protected:
    virtual TArray<int32> GetPossibleEntranceIndices(UFlowAbstractGraphBase* InGraph, const FRandomStream& InRandom) const override;
    virtual void FinalizePath(const FFlowAGStaticGrowthState& StaticState, FFlowAGGrowthState& State) const override;
};

