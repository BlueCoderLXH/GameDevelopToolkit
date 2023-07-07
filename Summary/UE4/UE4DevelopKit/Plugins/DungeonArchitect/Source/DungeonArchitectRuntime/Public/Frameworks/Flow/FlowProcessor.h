//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Utils/Attributes.h"
#include "Frameworks/Flow/ExecGraph/FlowExecTaskStructs.h"

struct FRandomStream;
class UFlowExecTask;
class UGridFlowExecScript;
class UGridFlowExecScriptGraphNode;
struct FFlowProcessorSettings;
class IFlowDomain;


struct DUNGEONARCHITECTRUNTIME_API FFlowProcessorSettings {
    FDAAttributeList AttributeList;
    TMap<FString, FString> SerializedAttributeList;
};

struct DUNGEONARCHITECTRUNTIME_API FFlowProcessorResult {
    EFlowTaskExecutionResult ExecResult = EFlowTaskExecutionResult::FailHalt;
    EFlowTaskExecutionFailureReason FailReason = EFlowTaskExecutionFailureReason::Unknown;
};

class DUNGEONARCHITECTRUNTIME_API FFlowProcessor {
public:
    FFlowProcessorResult Process(UGridFlowExecScript* ExecScript, const FRandomStream& InRandom, const FFlowProcessorSettings& InSettings);
    bool GetNodeState(const FGuid& NodeId, FFlowExecutionOutput& OutNodeState);
    EFlowTaskExecutionStage GetNodeExecStage(const FGuid& NodeId);
    void RegisterDomain(const TSharedPtr<const IFlowDomain> InDomain);
    
private:
    FFlowProcessorResult ExecuteNode(UGridFlowExecScriptGraphNode* Node, const FRandomStream& InRandom, const FFlowProcessorSettings& InSettings);
    
    static void SetTaskAttributes(UFlowExecTask* Task, const FFlowProcessorSettings& InSettings);
    TSharedPtr<const IFlowDomain> FindDomain(TSubclassOf<UFlowExecTask> InTaskClass) const;

private:
    TSet<FGuid> VisitedNodes;
    TMap<FGuid, FFlowExecutionOutput> NodeStates;
    TMap<FGuid, EFlowTaskExecutionStage> NodeExecStages;
    TArray<TSharedPtr<const IFlowDomain>> RegisteredDomains;
};

class DUNGEONARCHITECTRUNTIME_API IFlowProcessDomainExtender {
public:
    virtual ~IFlowProcessDomainExtender() {}
    virtual void ExtendDomains(FFlowProcessor& InProcessor) = 0;
};

