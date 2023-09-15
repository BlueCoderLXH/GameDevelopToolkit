//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Common/FlowTaskAbstract_PathBuilderBase.h"
#include "FlowTaskAbstract_CreatePath.generated.h"

class UFlowAbstractNode;
struct FFlowAGGrowthState;
struct FFlowAGStaticGrowthState;

UCLASS(Abstract)
class DUNGEONARCHITECTRUNTIME_API UFlowTaskAbstract_CreatePath : public UFlowTaskAbstract_PathBuilderBase {
    GENERATED_BODY()

    /**
        The minimum path size

        Variable Name: MinPathSize
    */
    UPROPERTY(EditAnywhere, Category = "Create Path")
    int32 MinPathSize = 3;

    /**
        The maximum path size

        Variable Name: MaxPathSize
    */
    UPROPERTY(EditAnywhere, Category = "Create Path")
    int32 MaxPathSize = 5;

    /**
        The name of the path.  This name can be used later to reference this path

        Variable Name: PathName
    */
    UPROPERTY(EditAnywhere, Category = "Create Path")
    FString PathName = "branch";

    /**
        The color of the nodes in this path

        Variable Name: [N/A]
    */
    UPROPERTY(EditAnywhere, Category = "Create Path")
    FLinearColor NodeColor = FLinearColor(1.0f, 0.5f, 0.0f);

    /**
        The name of an existing path that this branch is going to grow out of

        Variable Name: StartFromPath
    */
    UPROPERTY(EditAnywhere, Category = "Create Path")
    FString StartFromPath = "main";

    /**
        The name of an existing path that this branch should connect back to.
        Leave it empty if the path doesn't connect back to anything

        Variable Name: EndOnPath
    */
    UPROPERTY(EditAnywhere, Category = "Create Path")
    FString EndOnPath = "";

    /**
        If set, you'd enter this path through a teleporter, and there won't be a door connecting from the StartFromPath branch to this

        Variable Name: bEnterThroughTeleporter
    */
    UPROPERTY(EditAnywhere, Category = Teleporter)
    bool bEnterThroughTeleporter = false;

    /**
        The marker to spawn for the teleporter

        Variable Name: TeleporterMarkerName
    */
    UPROPERTY(EditAnywhere, Category = Teleporter, Meta=(EditCondition="bEnterThroughTeleporter"))
    FString TeleporterMarkerName = "";

    /**
        Override the path name of the first node in the path.  Useful for connecting other paths to it

        VariableName: StartNodePathNameOverride
    */
    UPROPERTY(EditAnywhere, Category=Advanced)
    FString StartNodePathNameOverride = "";
    
    /**
        Override the path name of the first node in the path.  Useful for connecting other paths to it

        VariableName: EndNodePathNameOverride
    */
    UPROPERTY(EditAnywhere, Category=Advanced)
    FString EndNodePathNameOverride = "";


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

};

