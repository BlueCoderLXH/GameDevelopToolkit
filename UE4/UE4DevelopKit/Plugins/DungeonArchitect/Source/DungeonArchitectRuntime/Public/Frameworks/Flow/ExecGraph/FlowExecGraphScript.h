//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "FlowExecGraphScript.generated.h"

class UFlowExecTask;

UCLASS()
class DUNGEONARCHITECTRUNTIME_API UGridFlowExecScriptGraphNode : public UObject {
    GENERATED_BODY()

public:
    UPROPERTY()
    FGuid NodeId;

    UPROPERTY()
    TArray<UGridFlowExecScriptGraphNode*> OutgoingNodes;

    UPROPERTY()
    TArray<UGridFlowExecScriptGraphNode*> IncomingNodes;
};

UCLASS()
class DUNGEONARCHITECTRUNTIME_API UGridFlowExecScriptTaskNode : public UGridFlowExecScriptGraphNode {
    GENERATED_BODY()

public:
    UPROPERTY()
    UFlowExecTask* Task;
};

UCLASS()
class DUNGEONARCHITECTRUNTIME_API UGridFlowExecScriptResultNode : public UGridFlowExecScriptGraphNode {
    GENERATED_BODY()

public:

};

UCLASS()
class DUNGEONARCHITECTRUNTIME_API UGridFlowExecScriptGraph : public UObject {
    GENERATED_BODY()

public:
    UPROPERTY()
    TArray<UGridFlowExecScriptGraphNode*> Nodes;
};


UCLASS()
class DUNGEONARCHITECTRUNTIME_API UGridFlowExecScript : public UObject {
    GENERATED_BODY()

public:
    UPROPERTY()
    UGridFlowExecScriptGraph* ScriptGraph;

    UPROPERTY()
    UGridFlowExecScriptResultNode* ResultNode;
};

