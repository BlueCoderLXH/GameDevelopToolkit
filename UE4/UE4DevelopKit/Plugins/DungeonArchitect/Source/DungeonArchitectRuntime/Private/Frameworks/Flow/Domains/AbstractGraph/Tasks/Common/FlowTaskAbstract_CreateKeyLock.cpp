//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Common/FlowTaskAbstract_CreateKeyLock.h"

#include "Core/Utils/Attributes.h"
#include "Core/Utils/MathUtils.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraph.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraphUtils.h"
#include "Frameworks/Flow/ExecGraph/FlowExecTaskAttributeMacros.h"

void UFlowTaskAbstract_CreateKeyLock::Execute(const FFlowExecutionInput& Input,
                                              const FFlowTaskExecutionSettings& InExecSettings, FFlowExecutionOutput& Output) {
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
    if (KeyPath.Len() == 0) {
        Output.ErrorMessage = "No Key Path specified";
        Output.ExecutionResult = EFlowTaskExecutionResult::FailHalt;
        return;
    }
    if (LockPath.Len() == 0) {
        Output.ErrorMessage = "No Lock Path specified";
        Output.ExecutionResult = EFlowTaskExecutionResult::FailHalt;
        return;
    }

    Output.State = Input.IncomingNodeOutputs[0].State->Clone();
    UFlowAbstractGraphBase* Graph = Output.State->GetState<UFlowAbstractGraphBase>(UFlowAbstractGraphBase::StateTypeID);
    const FFlowAbstractGraphQuery GraphQuery(Graph);

    FGuid KeyNodeId, LockLinkId;
    FString ErrorMessage = "Cannot find Key-Lock";

    if (FindKeyLockSetup(GraphQuery, *Input.Random, KeyNodeId, LockLinkId, ErrorMessage)) {
        UFlowAbstractNode* KeyNode = GraphQuery.GetNode(KeyNodeId);
        UFlowAbstractLink* LockLink = GraphQuery.GetLink(LockLinkId);
        if (KeyNode && LockLink) {
            UFlowGraphItem* KeyItem = KeyNode->CreateNewItem<UFlowGraphItem>();
            KeyItem->ItemType = EFlowGraphItemType::Key;
            KeyItem->MarkerName = KeyMarkerName;
            // TODO: Handle key placement data

            UFlowGraphItem* LockItem = LockLink->CreateNewItem<UFlowGraphItem>();
            LockItem->ItemType = EFlowGraphItemType::Lock;
            LockItem->MarkerName = LockMarkerName;
            // TODO: Handle lock placement data

            // Add a reference
            KeyItem->ReferencedItemIds.Add(LockItem->ItemId);

            Output.ExecutionResult = EFlowTaskExecutionResult::Success;
            return;
        }
    }

    Output.ErrorMessage = ErrorMessage;
    Output.ExecutionResult = EFlowTaskExecutionResult::FailRetry;
}

bool UFlowTaskAbstract_CreateKeyLock::FindKeyLockSetup(const FFlowAbstractGraphQuery& GraphQuery, const FRandomStream& Random,
                                                           FGuid& OutKeyNodeId, FGuid& OutLockLinkId, FString& OutErrorMessage) const
{
    UFlowAbstractGraphBase* Graph = GraphQuery.GetGraph<UFlowAbstractGraphBase>();
    FGuid EntranceNodeId;
    if (!FFlowAbstractGraphUtils::FindNodeWithItemType(GraphQuery, EFlowGraphItemType::Entrance, EntranceNodeId)) {
        OutErrorMessage = "Missing Entrance Node";
        return false;
    }

    TArray<FGuid> KeyNodeIds = FFlowAbstractGraphUtils::FindNodesOnPath(Graph, KeyPath);
    TArray<FGuid> LockNodeIds = FFlowAbstractGraphUtils::FindNodesOnPath(Graph, LockPath);

    FMathUtils::Shuffle(KeyNodeIds, Random);
    FMathUtils::Shuffle(LockNodeIds, Random);

    TMap<FGuid, float> Weights = FFlowAbstractGraphUtils::CalculateNodeWeights(GraphQuery, 1);

    const FFlowAbstractGraphTraversal& Traversal = GraphQuery.GetTraversal();
    
    for (const FGuid& KeyNodeId : KeyNodeIds) {
        for (const FGuid& LockNodeId : LockNodeIds) {
            // If the key and lock paths are the same, make sure the distance to the key is less than the lock
            if (KeyPath == LockPath) {
                const float* DistanceToKeyPtr = Weights.Find(KeyNodeId);
                const float* DistanceToLockPtr = Weights.Find(LockNodeId);
                if (!DistanceToKeyPtr || !DistanceToLockPtr) return false;
                const float DistanceToKey = *DistanceToKeyPtr;
                const float DistanceToLock = *DistanceToLockPtr;
                if (DistanceToKey > DistanceToLock) {
                    return false;
                }
            }

            // Lock link list creation criteria
            //     1. Get all lock node links that connect to other nodes in the same lock path
            //     2. grab the rest of the links connected to the lock node
            //     3. Filter out the ones that already have a lock on them
            // Lock link selection criteria 
            //     1. Make sure the key node is accessible from the entrance, after blocking off the selected lock link
            //     2. Make sure lock node is not accessible from the entrance after blocking off the selected lock link

            // Generate the lock link array
            TArray<FFlowAbstractGraphTraversal::FNodeInfo> LockNodeLinks;
            {
                TArray<FFlowAbstractGraphTraversal::FNodeInfo> AllLockLinks;
                Traversal.GetConnectedNodes(LockNodeId, AllLockLinks);

                TArray<FFlowAbstractGraphTraversal::FNodeInfo> ResultPrimary;
                TArray<FFlowAbstractGraphTraversal::FNodeInfo> ResultSecondary;
                for (const FFlowAbstractGraphTraversal::FNodeInfo& ConnectionInfo : AllLockLinks) {
                    // Make sure this link doesn't already have a lock
                    UFlowAbstractLink* LockLink = GraphQuery.GetLink(ConnectionInfo.LinkId);
                    if (!LockLink || FFlowAbstractGraphUtils::ContainsItem(LockLink->LinkItems, EFlowGraphItemType::Lock)) {
                        continue;
                    }
                    
                    UFlowAbstractNode* ConnectedNode = GraphQuery.GetNode(ConnectionInfo.NodeId);
                    if (ConnectedNode && ConnectedNode->PathName == LockPath) {
                        ResultPrimary.Add(ConnectionInfo);
                    }
                    else {
                        ResultSecondary.Add(ConnectionInfo);
                    }
                }
                FMathUtils::Shuffle(ResultPrimary, Random);
                LockNodeLinks.Append(ResultPrimary);
                
                FMathUtils::Shuffle(ResultSecondary, Random);
                LockNodeLinks.Append(ResultSecondary);
            }

            // Select the first valid link from the list
            for (const FFlowAbstractGraphTraversal::FNodeInfo& LockConnection : LockNodeLinks) {
                const FGuid LockLinkId = LockConnection.LinkId;
                auto CanTraverse = [&LockLinkId](const FFlowAbstractGraphTraversal::FNodeInfo& TraverseInfo) -> bool {
                    return TraverseInfo.LinkId != LockLinkId;
                };
                
                // 1. Make sure the key node is accessible from the entrance, after blocking off the selected lock link
                const bool bCanReachKey = FFlowAbstractGraphUtils::CanReachNode(GraphQuery, EntranceNodeId, KeyNodeId, false, false, true, CanTraverse);
                if (bCanReachKey) {
                    // 2. Make sure lock node is not accessible from the entrance after blocking off the selected lock link
                    const bool bCanReachLockedNode = FFlowAbstractGraphUtils::CanReachNode(GraphQuery, EntranceNodeId, LockNodeId, false, false, true, CanTraverse);
                    if (!bCanReachLockedNode) {
                        OutKeyNodeId = KeyNodeId;
                        OutLockLinkId = LockLinkId;
                        return true;
                    }
                }
            }
        } 
    }
    return false;
}


bool UFlowTaskAbstract_CreateKeyLock::GetParameter(const FString& InParameterName, FDAAttribute& OutValue) {
    FLOWTASKATTR_GET_STRING(KeyPath);
    FLOWTASKATTR_GET_STRING(LockPath);
    FLOWTASKATTR_GET_STRING(KeyMarkerName);
    FLOWTASKATTR_GET_STRING(LockMarkerName);

    return false;

}

bool UFlowTaskAbstract_CreateKeyLock::SetParameter(const FString& InParameterName,
                                                       const FDAAttribute& InValue) {
    FLOWTASKATTR_SET_STRING(KeyPath);
    FLOWTASKATTR_SET_STRING(LockPath);
    FLOWTASKATTR_SET_STRING(KeyMarkerName);
    FLOWTASKATTR_SET_STRING(LockMarkerName);

    return false;
}

bool UFlowTaskAbstract_CreateKeyLock::SetParameterSerialized(const FString& InParameterName,
                                                                 const FString& InSerializedText) {
    FLOWTASKATTR_SET_PARSE_STRING(KeyPath);
    FLOWTASKATTR_SET_PARSE_STRING(LockPath);
    FLOWTASKATTR_SET_PARSE_STRING(KeyMarkerName);
    FLOWTASKATTR_SET_PARSE_STRING(LockMarkerName);

    return false;
}

