//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Common/FlowTaskAbstract_Finalize.h"
#include "GridFlowTaskAbstract_Finalize.generated.h"

enum class EGridFlowAbstractNodeRoomType : uint8;

UCLASS(Meta = (AbstractTask, Title = "Finalize Graph", Tooltip = "Call this to finalize the layout graph", MenuPriority = 1500))
class DUNGEONARCHITECTRUNTIME_API UGridFlowTaskAbstract_Finalize : public UFlowTaskAbstract_Finalize {
    GENERATED_BODY()

public:
    /**
        Indicates if corridors are allowed. A corridor is created if it has one entrance and one exit 
        and they are in the same line (i.e. both along X or Y axis)

        Variable Name: bGenerateCorridors
    */
    UPROPERTY(EditAnywhere, Category = "Finalize Graph")
    bool bGenerateCorridors = true;

    /**
        Indicates if caves are allowed. A room node is promoted to a cave 
        if the number of enemies in this node is less than or equal to MaxEnemiesPerCaveNode

        Variable Name: bGenerateCaves
    */
    UPROPERTY(EditAnywhere, Category = "Finalize Graph")
    bool bGenerateCaves = true;

    /**
        The condition for promoting a room to a cave.  If the number of enemies in the room node is 
        less than or equal to this value, it is promoted to a cave.
        Sometimes, it might still be room regardless of this value, if we need a door and there 
        are no nearby rooms (caves chunks don't have doors)

        Variable Name: MaxEnemiesPerCaveNode
    */
    UPROPERTY(EditAnywhere, Category = "Finalize Graph")
    int32 MaxEnemiesPerCaveNode = 3;

public:
    virtual void Execute(const FFlowExecutionInput& Input, const FFlowTaskExecutionSettings& InExecSettings, FFlowExecutionOutput& Output) override;
    virtual bool GetParameter(const FString& InParameterName, FDAAttribute& OutValue) override;
    virtual bool SetParameter(const FString& InParameterName, const FDAAttribute& InValue) override;
    virtual bool SetParameterSerialized(const FString& InParameterName, const FString& InSerializedText) override;

private:
    void AssignRoomTypes(const FFlowAbstractGraphQuery& GraphQuery, const FRandomStream& Random) const;
    EGridFlowAbstractNodeRoomType GetNodeRoomType(const FFlowAbstractGraphQuery& GraphQuery, UFlowAbstractNode* Node) const;
};

