//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraphQuery.h"


void FFlowAbstractGraphTraversal::Build(UFlowAbstractGraphBase* InGraph) {
    OutgoingNodes.Reset();
    IncomingNodes.Reset();

    for (UFlowAbstractLink* Link : InGraph->GraphLinks) {
        if (Link->Type == EFlowAbstractLinkType::Unconnected) {
            continue;
        }

        // Add outgoing node
        {
            TArray<FNodeInfo>& Outgoing = OutgoingNodes.FindOrAdd(Link->Source);
            FNodeInfo& Info = Outgoing.AddDefaulted_GetRef();
            Info.NodeId = Link->Destination;
            Info.LinkId = Link->LinkId;
            Info.bOutgoing = true;
        }

        // Add incoming node
        {
            TArray<FNodeInfo>& Incoming = IncomingNodes.FindOrAdd(Link->Destination);
            FNodeInfo& Info = Incoming.AddDefaulted_GetRef();
            Info.NodeId = Link->Source;
            Info.LinkId = Link->LinkId;
            Info.bOutgoing = true;
        }
    }

    // Build the teleporter list
    {
        // Build a mapping of the teleporter item to their host node mapping
        TMap<FGuid, FGuid> ItemParentNodeMap;    // teleporter to owning node mapping
        for (UFlowAbstractNode* Node : InGraph->GraphNodes) {
            if (!Node || !Node->bActive) continue;
            for (UFlowGraphItem* NodeItem : Node->NodeItems) {
                if (NodeItem && NodeItem->ItemType == EFlowGraphItemType::Teleporter) {
                    FGuid& NodeIdRef = ItemParentNodeMap.FindOrAdd(NodeItem->ItemId);
                    NodeIdRef = Node->NodeId;
                }
            }
        }

        // Make another pass to build the teleporter list
        for (UFlowAbstractNode* Node : InGraph->GraphNodes) {
            if (!Node || !Node->bActive) continue;
            for (UFlowGraphItem* NodeItem : Node->NodeItems) {
                if (NodeItem && NodeItem->ItemType == EFlowGraphItemType::Teleporter) {
                    if (NodeItem->ReferencedItemIds.Num() > 0) {
                        FGuid OtherTeleporterId = NodeItem->ReferencedItemIds[0];
                        FGuid* OtherNodeIdPtr = ItemParentNodeMap.Find(OtherTeleporterId);
                        if (OtherNodeIdPtr) {
                            FGuid Node1 = Node->NodeId;
                            FGuid Node2 = *OtherNodeIdPtr;

                            FGuid& Node1Mapping = Teleporters.FindOrAdd(Node1);
                            Node1Mapping = Node2;
                            
                            FGuid& Node2Mapping = Teleporters.FindOrAdd(Node2);
                            Node2Mapping = Node1;
                        }
                    }
                }
            }
        }
    }
}

void FFlowAbstractGraphTraversal::GetOutgoingNodes(const FGuid& InNodeId, TArray<FNodeInfo>& OutResult) const {
    const TArray<FNodeInfo>* SearchResult = OutgoingNodes.Find(InNodeId);
    if (SearchResult) {
        OutResult.Append(*SearchResult);
    } 
}

void FFlowAbstractGraphTraversal::GetIncomingNodes(const FGuid& InNodeId, TArray<FNodeInfo>& OutResult) const {
    const TArray<FNodeInfo>* SearchResult = IncomingNodes.Find(InNodeId);
    if (SearchResult) {
        OutResult.Append(*SearchResult);
    } 
}

void FFlowAbstractGraphTraversal::GetConnectedNodes(const FGuid& InNodeId, TArray<FNodeInfo>& OutResult) const {
    GetOutgoingNodes(InNodeId, OutResult);
    GetIncomingNodes(InNodeId, OutResult);
}

bool FFlowAbstractGraphTraversal::GetTeleportNode(const FGuid& InNodeId, FGuid& OutConnectedNodeId) const {
    const FGuid* SearchResult = Teleporters.Find(InNodeId);
    if (!SearchResult) {
        return false;
    }

    OutConnectedNodeId = *SearchResult;
    return true;
}

