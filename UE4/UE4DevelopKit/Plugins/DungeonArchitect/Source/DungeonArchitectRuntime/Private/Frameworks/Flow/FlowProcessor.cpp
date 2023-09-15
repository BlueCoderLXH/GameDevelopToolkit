//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/FlowProcessor.h"

#include "Frameworks/Flow/Domains/FlowDomain.h"
#include "Frameworks/Flow/ExecGraph/FlowExecGraphScript.h"
#include "Frameworks/Flow/ExecGraph/FlowExecTask.h"

DEFINE_LOG_CATEGORY_STATIC(LogGridFlowProcessor, Log, All);

FFlowProcessorResult FFlowProcessor::Process(UGridFlowExecScript* ExecScript, const FRandomStream& InRandom, const FFlowProcessorSettings& InSettings) {
    NodeStates.Reset();
    VisitedNodes.Reset();
    NodeExecStages.Reset();

    return ExecuteNode(ExecScript->ResultNode, InRandom, InSettings);
}

bool FFlowProcessor::GetNodeState(const FGuid& NodeId, FFlowExecutionOutput& OutNodeState) {
    FFlowExecutionOutput* SearchResult = NodeStates.Find(NodeId);
    if (!SearchResult) {
        return false;
    }

    OutNodeState = *SearchResult;
    return true;
}

EFlowTaskExecutionStage FFlowProcessor::GetNodeExecStage(const FGuid& NodeId) {
    EFlowTaskExecutionStage* SearchResult = NodeExecStages.Find(NodeId);
    return SearchResult ? *SearchResult : EFlowTaskExecutionStage::NotExecuted;
}

void FFlowProcessor::RegisterDomain(TSharedPtr<const IFlowDomain> InDomain) {
    RegisteredDomains.Add(InDomain);
}

FFlowProcessorResult FFlowProcessor::ExecuteNode(UGridFlowExecScriptGraphNode* Node, const FRandomStream& InRandom, const FFlowProcessorSettings& InSettings) {
    check(!VisitedNodes.Contains(Node->NodeId));
    check(!NodeStates.Contains(Node->NodeId));

    {
        EFlowTaskExecutionStage& ExecStage = NodeExecStages.FindOrAdd(Node->NodeId);
        ExecStage = EFlowTaskExecutionStage::WaitingToExecute;
    }

    VisitedNodes.Add(Node->NodeId);

    // Prepare the execution input 
    FFlowExecutionInput NodeInput;
    NodeInput.Random = &InRandom;

    {
        for (UGridFlowExecScriptGraphNode* IncomingNode : Node->IncomingNodes) {
            if (!VisitedNodes.Contains(IncomingNode->NodeId)) {
                const FFlowProcessorResult IncomingResult = ExecuteNode(IncomingNode, InRandom, InSettings);
                if (IncomingResult.ExecResult != EFlowTaskExecutionResult::Success) {
                    return IncomingResult;
                }
            }
            FFlowExecutionOutput* IncomingNodeOutput = NodeStates.Find(IncomingNode->NodeId);
            if (IncomingNodeOutput) {
                NodeInput.IncomingNodeOutputs.Add(*IncomingNodeOutput);
            }
        }
    }

    // Make sure all the incoming nodes succeeded
    bool bIncomingFailRetry = false;
    bool bIncomingFailHalt = false;
    for (const FFlowExecutionOutput& IncomingNodeOutput : NodeInput.IncomingNodeOutputs) {
        if (IncomingNodeOutput.ExecutionResult == EFlowTaskExecutionResult::FailRetry) {
            bIncomingFailRetry = true;
        }
        else if (IncomingNodeOutput.ExecutionResult == EFlowTaskExecutionResult::FailHalt) {
            bIncomingFailHalt = true;
        }
    }

    FFlowTaskExecutionSettings ExecSettings;
    
    // Execute the node task
    FFlowExecutionOutput NodeOutput;

    if (bIncomingFailHalt) {
        NodeOutput.ExecutionResult = EFlowTaskExecutionResult::FailHalt;
        NodeOutput.State = nullptr;
    }
    else if (bIncomingFailRetry) {
        NodeOutput.ExecutionResult = EFlowTaskExecutionResult::FailRetry;
        NodeOutput.State = nullptr;
    }
    else {
        if (UGridFlowExecScriptTaskNode* TaskNode = Cast<UGridFlowExecScriptTaskNode>(Node)) {
            // Duplicate the task so we can apply attribute overrides to it without affect the source asset
            UFlowExecTask* Task = NewObject<UFlowExecTask>(GetTransientPackage(), TaskNode->Task->GetClass(),
                                                                   NAME_None, RF_NoFlags, TaskNode->Task);
            SetTaskAttributes(Task, InSettings);
            NodeInput.Domain = FindDomain(Task->GetClass());
            if (NodeInput.Domain.IsValid()) {
                Task->Execute(NodeInput, ExecSettings, NodeOutput);
            }
            else {
                NodeOutput.ErrorMessage = "Unsupported Node";
                NodeOutput.ExecutionResult = EFlowTaskExecutionResult::FailHalt;
                NodeOutput.State = nullptr;
            }
        }
        else if (UGridFlowExecScriptResultNode* ResultNode = Cast<UGridFlowExecScriptResultNode>(Node)) {
            if (NodeInput.IncomingNodeOutputs.Num() == 0) {
                NodeOutput.ErrorMessage = "Missing Input";
                NodeOutput.ExecutionResult = EFlowTaskExecutionResult::FailHalt;
                NodeOutput.State = nullptr;
            }
            else if (NodeInput.IncomingNodeOutputs.Num() > 1) {
                NodeOutput.ErrorMessage = "Only one input allowed";
                NodeOutput.ExecutionResult = EFlowTaskExecutionResult::FailHalt;
                NodeOutput.State = nullptr;
            }
            else {
                FFlowExecutionOutput IncomingState = NodeInput.IncomingNodeOutputs[0];
                NodeOutput.ExecutionResult = IncomingState.ExecutionResult;
                if (IncomingState.State.IsValid()) {
                    NodeOutput.State = IncomingState.State->Clone();
                }
            }
        }

        EFlowTaskExecutionStage& ExecStage = NodeExecStages.FindOrAdd(Node->NodeId);
        ExecStage = EFlowTaskExecutionStage::Executed;
    }

    NodeStates.Add(Node->NodeId, NodeOutput);

    FFlowProcessorResult Result;
    Result.ExecResult = NodeOutput.ExecutionResult;
    Result.FailReason = NodeOutput.FailureReason;
    return Result;
}

TSharedPtr<const IFlowDomain> FFlowProcessor::FindDomain(TSubclassOf<UFlowExecTask> InTaskClass) const {
    for (TSharedPtr<const IFlowDomain> Domain : RegisteredDomains) {
        TArray<UClass*> DomainTasks;
        Domain->GetDomainTasks(DomainTasks);
        if (DomainTasks.Contains(InTaskClass)) {
            return Domain;
        }
    }
    return nullptr;
}

void FFlowProcessor::SetTaskAttributes(UFlowExecTask* Task, const FFlowProcessorSettings& InSettings) {
    if (Task->NodeVariableName.Len() > 0) {
        // Override the attributes
        const TMap<FString, FDAAttribute>* SearchResult = InSettings.AttributeList.AttributesByNode.Find(Task->NodeVariableName);
        if (SearchResult) {
            const TMap<FString, FDAAttribute>& NodeAttributes = *SearchResult;
            for (auto& Entry : NodeAttributes) {
                FString ParamName = Entry.Key;
                const FDAAttribute& ParamAttrib = Entry.Value;
                if (!Task->SetParameter(ParamName, ParamAttrib)) {
                    UE_LOG(LogGridFlowProcessor, Error, TEXT("Failed to set Parameter: %s.%s"), *Task->NodeVariableName,
                           *ParamName);
                }
            }
        }

        // Override the serialized attributes
        const FString NodePrefix = Task->NodeVariableName + ".";
        for (const auto& Entry : InSettings.SerializedAttributeList) {
            const FString& FullPath = Entry.Key;
            const FString& Value = Entry.Value;
            if (FullPath.StartsWith(NodePrefix)) {
                FString VariableName = FullPath.Mid(NodePrefix.Len());
                if (!Task->SetParameterSerialized(VariableName, Value)) {
                    UE_LOG(LogGridFlowProcessor, Error, TEXT("Failed to set Parameter: %s"), *FullPath);
                }
            }
        }
    }
}

