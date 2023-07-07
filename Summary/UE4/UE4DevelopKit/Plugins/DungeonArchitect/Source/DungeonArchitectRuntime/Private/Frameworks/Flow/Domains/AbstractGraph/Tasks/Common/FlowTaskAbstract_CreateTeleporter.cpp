//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Common/FlowTaskAbstract_CreateTeleporter.h"

#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraphQuery.h"
#include "Frameworks/Flow/ExecGraph/FlowExecTaskAttributeMacros.h"

class UFlowAbstractGraphBase;

void UFlowTaskAbstract_CreateTeleporter::Execute(const FFlowExecutionInput& Input, const FFlowTaskExecutionSettings& InExecSettings, FFlowExecutionOutput& Output) {
    if (Input.IncomingNodeOutputs.Num() == 0) {
        Output.ErrorMessage = "Missing Input";
        Output.ExecutionResult = EFlowTaskExecutionResult::FailHalt;
        return;
    }
    if (Input.IncomingNodeOutputs.Num() > 1) {
        Output.ErrorMessage = "Only one input allowed";
        Output.ExecutionResult = EFlowTaskExecutionResult::FailHalt;
        return;
    }

    Output.State = Input.IncomingNodeOutputs[0].State->Clone();
    UFlowAbstractGraphBase* Graph = Output.State->GetState<UFlowAbstractGraphBase>(UFlowAbstractGraphBase::StateTypeID);
    FFlowAbstractGraphQuery GraphQuery(Graph);

    TMap<FString, TArray<FGuid>> NodesByPath;
    for (UFlowAbstractNode* Node : Graph->GraphNodes) {
        if (Node->bActive && Node->PathName.Len() > 0) {
            TArray<FGuid>& PathNodes = NodesByPath.FindOrAdd(Node->PathName);
            PathNodes.Add(Node->NodeId);
        }
    }

    TArray<FGuid>& FirstPathNodes = NodesByPath.FindOrAdd(FirstPathName);
    TArray<FGuid>& SecondPathNodes = NodesByPath.FindOrAdd(SecondPathName);

    if (FirstPathNodes.Num() == 0 || SecondPathNodes.Num() == 0) {
        Output.ErrorMessage = "Path not found";
        Output.ExecutionResult = EFlowTaskExecutionResult::FailHalt;
        return;
    }

    const int32 FirstPathIdx = Input.Random->RandRange(0, FirstPathNodes.Num() - 1);
    const int32 SecondPathIdx = Input.Random->RandRange(0, SecondPathNodes.Num() - 1);

    UFlowAbstractNode* FirstNode = GraphQuery.GetNode(FirstPathNodes[FirstPathIdx]);
    check(FirstNode);

    // Create the first item
    UFlowGraphItem* FirstItem = FirstNode->CreateNewItem<UFlowGraphItem>();
    FirstItem->ItemType = EFlowGraphItemType::Teleporter;
    FirstItem->MarkerName = TeleporterMarkerName;
    
    UFlowAbstractNode* SecondNode = GraphQuery.GetNode(SecondPathNodes[SecondPathIdx]);
    check(SecondNode);
    
    // Create the second item
    UFlowGraphItem* SecondItem = SecondNode->CreateNewItem<UFlowGraphItem>();
    SecondItem->ItemType = EFlowGraphItemType::Teleporter;
    SecondItem->MarkerName = TeleporterMarkerName;

    // Link the items together
    FirstItem->ReferencedItemIds.Add(SecondItem->ItemId);
    SecondItem->ReferencedItemIds.Add(FirstItem->ItemId);
    
    Output.ExecutionResult = EFlowTaskExecutionResult::Success;
}

bool UFlowTaskAbstract_CreateTeleporter::GetParameter(const FString& InParameterName, FDAAttribute& OutValue) {
    FLOWTASKATTR_GET_STRING(FirstPathName);
    FLOWTASKATTR_GET_STRING(SecondPathName);
    FLOWTASKATTR_GET_STRING(TeleporterMarkerName);

    return false;
}

bool UFlowTaskAbstract_CreateTeleporter::SetParameter(const FString& InParameterName, const FDAAttribute& InValue) {
    FLOWTASKATTR_SET_STRING(FirstPathName);
    FLOWTASKATTR_SET_STRING(SecondPathName);
    FLOWTASKATTR_SET_STRING(TeleporterMarkerName);

    return false;
}

bool UFlowTaskAbstract_CreateTeleporter::SetParameterSerialized(const FString& InParameterName, const FString& InSerializedText) {
    FLOWTASKATTR_SET_PARSE_STRING(FirstPathName);
    FLOWTASKATTR_SET_PARSE_STRING(SecondPathName);
    FLOWTASKATTR_SET_PARSE_STRING(TeleporterMarkerName);

    return false;
}

