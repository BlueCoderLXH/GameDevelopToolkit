//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Common/FlowTaskAbstract_PathBuilderBase.h"
#include "FlowTaskAbstract_CreateMainPath.generated.h"

class UGridFlowAbstractGraph;
class UFlowAbstractGraphBase;

UCLASS(Abstract)
class DUNGEONARCHITECTRUNTIME_API UFlowTaskAbstract_CreateMainPath : public UFlowTaskAbstract_PathBuilderBase {
    GENERATED_BODY()

public:
    /**
        The size of the main path

        Variable Name: PathSize
    */
    UPROPERTY(EditAnywhere, Category = "Create Main Path")
    int32 PathSize = 12;

    /**
        The name of this path.  It can be later referenced with this name

        Variable Name: PathName
    */
    UPROPERTY(EditAnywhere, Category = "Create Main Path")
    FString PathName = "main";

    /**
        The color of the nodes in this path

        Variable Name: [N/A]
    */
    UPROPERTY(EditAnywhere, Category = "Create Main Path")
    FLinearColor NodeColor = FLinearColor::Green;

    /**
        Marker name to emit at the spawn point.   Add this marker name in the theme file and emit your spawn blueprints (e.g. player start)

        Variable Name: StartMarkerName
    */
    UPROPERTY(EditAnywhere, Category = "Create Main Path")
    FString StartMarkerName = "SpawnPoint";

    /**
        Marker name to emit at the goal point.   Add this marker name in the theme file and emit your level exit blueprints

        Variable Name: GoalMarkerName
    */
    UPROPERTY(EditAnywhere, Category = "Create Main Path")
    FString GoalMarkerName = "LevelGoal";

    /**
        Optionally, specify a different path name for the start node in the path

        Variable Name: StartNodePathName
    */
    UPROPERTY(EditAnywhere, Category = "Create Main Path")
    FString StartNodePathName = "main_start";

    /**
        Optionally, specify a different path name for the end node in the path

        Variable Name: GoalNodePathName
    */
    UPROPERTY(EditAnywhere, Category = "Create Main Path")  
    FString GoalNodePathName = "main_goal";

    /**
      Number of searches to perform at once.  This helps converge to a solution faster if we are stuck on a single search path.
      However, this might increase the overall search time by a little bit.
    */
    UPROPERTY(EditAnywhere, Category=Advanced)
    int32 NumParallelSearches = 10;
    
    UPROPERTY(EditAnywhere, Category=Advanced)
    int64 MaxFramesToProcess = 1000;
    
public:
    virtual void Execute(const FFlowExecutionInput& Input, const FFlowTaskExecutionSettings& InExecSettings, FFlowExecutionOutput& Output) override;
    virtual bool GetParameter(const FString& InParameterName, FDAAttribute& OutValue) override;
    virtual bool SetParameter(const FString& InParameterName, const FDAAttribute& InValue) override;
    virtual bool SetParameterSerialized(const FString& InParameterName, const FString& InSerializedText) override;

protected:
    virtual TArray<int32> GetPossibleEntranceIndices(UFlowAbstractGraphBase* InGraph, const FRandomStream& InRandom) const;
};

