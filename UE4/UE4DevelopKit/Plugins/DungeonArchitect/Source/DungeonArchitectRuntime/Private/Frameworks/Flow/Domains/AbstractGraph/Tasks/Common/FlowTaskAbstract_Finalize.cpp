//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Common/FlowTaskAbstract_Finalize.h"

#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraph.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraphUtils.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractLink.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Implementations/GridFlowAbstractGraph.h"
#include "Frameworks/Flow/ExecGraph/FlowExecTaskAttributeMacros.h"

void UFlowTaskAbstract_Finalize::Execute(const FFlowExecutionInput& Input, const FFlowTaskExecutionSettings& InExecSettings, FFlowExecutionOutput& Output) {
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

    TMap<FGuid, FFlowGraphItemContainer> ItemMap;
    for (UFlowAbstractNode* Node : Graph->GraphNodes) {
        if (!Node) continue;
        for (UFlowGraphItem* Item : Node->NodeItems) {
            FFlowGraphItemContainer& ItemInfo = ItemMap.FindOrAdd(Item->ItemId);
            ItemInfo.ItemId = Item->ItemId;
            ItemInfo.HostNode = Node;
        }
    }
    for (UFlowAbstractLink* Link : Graph->GraphLinks) {
        for (UFlowGraphItem* Item : Link->LinkItems) {
            FFlowGraphItemContainer& ItemInfo = ItemMap.FindOrAdd(Item->ItemId);
            ItemInfo.ItemId = Item->ItemId;
            ItemInfo.HostLink = Link;
        }
    }

    TMap<FGuid, float> Weights = FFlowAbstractGraphUtils::CalculateNodeWeights(GraphQuery, 10);

    // Make the links one directional if the difference in the source/dest nodes is too much
    for (UFlowAbstractLink* Link : Graph->GraphLinks) {
        if (Link->Type == EFlowAbstractLinkType::Unconnected) continue;
        UFlowAbstractNode* Source = GraphQuery.GetNode(Link->Source);
        UFlowAbstractNode* Destination = GraphQuery.GetNode(Link->Destination);
        if (!Source || !Destination) continue;
        if (!Source->bActive || !Destination->bActive) continue;

        float* WeightSourcePtr = Weights.Find(Link->Source);
        float* WeightDestinationPtr = Weights.Find(Link->Destination);
        if (WeightSourcePtr && WeightDestinationPtr) {
            const float WeightDiff = *WeightSourcePtr - *WeightDestinationPtr + 1;
            if (WeightDiff > OneWayDoorPromotionWeight) {
                if (FFlowAbstractGraphUtils::CanPromoteToOneWayLink(GraphQuery, Link->LinkId)) {
                    Link->Type = EFlowAbstractLinkType::OneWay;
                }
            }
        }
    }

    // Remove unconnected links
    TArray<UFlowAbstractLink*> LinksCopy = Graph->GraphLinks;
    for (UFlowAbstractLink* LinkCopy : LinksCopy) {
        if (LinkCopy->Type == EFlowAbstractLinkType::Unconnected) {
            Graph->GraphLinks.Remove(LinkCopy);
        }
    }

    // Emit debug info
    if (bShowDebugData) {
        for (UFlowAbstractNode* Node : Graph->GraphNodes) {
            if (!Node) continue;
            float* WeightPtr = Weights.Find(Node->NodeId);
            if (!WeightPtr) continue;
            
            UFlowGraphItem* DebugItem = Node->CreateNewItem<UFlowGraphItem>();
            DebugItem->ItemType = EFlowGraphItemType::Custom;
            DebugItem->CustomInfo.PreviewText = FString::Printf(TEXT("%d"), FMath::RoundToInt(*WeightPtr));
            DebugItem->CustomInfo.PreviewBackgroundColor = FLinearColor::Black;
            DebugItem->CustomInfo.PreviewTextColor = FLinearColor::White;
            DebugItem->CustomInfo.FontScale = 0.75f;
        }
    }

    
    Output.ExecutionResult = EFlowTaskExecutionResult::Success;
}


bool UFlowTaskAbstract_Finalize::GetParameter(const FString& InParameterName, FDAAttribute& OutValue) {
    FLOWTASKATTR_GET_FLOAT(OneWayDoorPromotionWeight);

    return false;
}

bool UFlowTaskAbstract_Finalize::SetParameter(const FString& InParameterName, const FDAAttribute& InValue) {
    FLOWTASKATTR_SET_FLOAT(OneWayDoorPromotionWeight);

    return false;
}

bool UFlowTaskAbstract_Finalize::SetParameterSerialized(const FString& InParameterName, const FString& InSerializedText) {
    FLOWTASKATTR_SET_PARSE_FLOAT(OneWayDoorPromotionWeight);

    return false;
}

