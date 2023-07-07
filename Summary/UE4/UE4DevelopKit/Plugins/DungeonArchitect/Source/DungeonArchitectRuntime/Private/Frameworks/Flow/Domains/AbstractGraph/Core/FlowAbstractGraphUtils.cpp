//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraphUtils.h"

#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraph.h"

#include "Containers/Queue.h"

namespace {
    struct FNodeWeightInfo {
        FNodeWeightInfo() {
        }

        FNodeWeightInfo(const FGuid& InNodeId, float InWeight)
            : NodeId(InNodeId)
              , Weight(InWeight) {
        }

        FGuid NodeId;
        float Weight = 0;
    };

    void CalculateIslandWeights(const FFlowAbstractGraphQuery& GraphQuery, const FGuid& StartNode, float LockedWeight, TMap<FGuid, float>& OutWeights,
                const TSet<FGuid>& InLockedNodes, TSet<FGuid>& OutVisited) {
        const FFlowAbstractGraphTraversal& GraphTraversal = GraphQuery.GetTraversal();
        
        TQueue<FNodeWeightInfo> Queue;
        Queue.Enqueue(FNodeWeightInfo(StartNode, 0));
        OutVisited.Add(StartNode);

        while (!Queue.IsEmpty()) {
            FNodeWeightInfo Front;
            Queue.Dequeue(Front);
            OutVisited.Add(Front.NodeId);

            {
                float* WeightPtr = OutWeights.Find(Front.NodeId);
                if (WeightPtr) {
                    *WeightPtr = FMath::Min(*WeightPtr, Front.Weight);
                }
                else {
                    OutWeights.Add(Front.NodeId, Front.Weight);
                }
            }

            // Traverse the children
            TArray<FFlowAbstractGraphTraversal::FNodeInfo> OutgoingNodes;
            GraphTraversal.GetOutgoingNodes(Front.NodeId, OutgoingNodes);

            // Traverse the teleporter, if any
            {
                FGuid TeleporterNodeId;
                if (GraphTraversal.GetTeleportNode(Front.NodeId, TeleporterNodeId)) {
                    FFlowAbstractGraphTraversal::FNodeInfo OtherNodeInfo;
                    OtherNodeInfo.NodeId = TeleporterNodeId;
                    OtherNodeInfo.LinkId = FGuid();
                    OutgoingNodes.Add(OtherNodeInfo);
                }
            }
            
            for (const FFlowAbstractGraphTraversal::FNodeInfo& OutgoingNode : OutgoingNodes) {
                bool bTraversedChild = true;
                if (InLockedNodes.Contains(OutgoingNode.NodeId)) {
                    bTraversedChild = false;
                }
                else if (OutVisited.Contains(OutgoingNode.NodeId)) {
                    // The child node has already been traversed.  Do not traverse if the child's weight
                    // is less than the current weight
                    const float CurrentWeight = Front.Weight;
                    const float ChildWeight = OutWeights[OutgoingNode.NodeId];
                    if (CurrentWeight > ChildWeight) {
                        bTraversedChild = false;
                    }
                }

                if (bTraversedChild) {
                    float NodeWeight = 1;
                    UFlowAbstractLink* OutgoingLink = GraphQuery.GetLink(OutgoingNode.LinkId);
                    if (OutgoingLink && FFlowAbstractGraphUtils::ContainsItem(OutgoingLink->LinkItems, EFlowGraphItemType::Lock)) {
                        NodeWeight = LockedWeight;
                    }

                    Queue.Enqueue(FNodeWeightInfo(OutgoingNode.NodeId, Front.Weight + NodeWeight));
                }
            }
        }
    }

    void GetIslandNodesImpl(const FFlowAbstractGraphTraversal& GraphTraversal, const FGuid& NodeId, TSet<FGuid>& Visited, TArray<FGuid>& OutIslandNodes) {
        if (Visited.Contains(NodeId)) return;
        Visited.Add(NodeId);
        OutIslandNodes.Add(NodeId);

        TArray<FFlowAbstractGraphTraversal::FNodeInfo> ConnectedNodes;
        GraphTraversal.GetConnectedNodes(NodeId, ConnectedNodes);
        for (const FFlowAbstractGraphTraversal::FNodeInfo& ConnectedNode : ConnectedNodes) {
            if (!Visited.Contains(ConnectedNode.NodeId)) {
                GetIslandNodesImpl(GraphTraversal, ConnectedNode.NodeId, Visited, OutIslandNodes);
            }
        }
    }
    
    void GetIslandNodes(const FFlowAbstractGraphTraversal& GraphTraversal, const FGuid& NodeId, TArray<FGuid>& OutIslandNodes) {
        TSet<FGuid> Visited;
        GetIslandNodesImpl(GraphTraversal, NodeId, Visited, OutIslandNodes);
    }
    
    void GetIslandSourceNodes(const FFlowAbstractGraphTraversal& GraphTraversal, const TArray<FGuid>& InIslandNodes, TArray<FGuid>& OutSourceNodes) {
        OutSourceNodes.Reset();

        for (const FGuid& NodeId : InIslandNodes) {
            TArray<FFlowAbstractGraphTraversal::FNodeInfo> IncomingNodes;
            GraphTraversal.GetIncomingNodes(NodeId, IncomingNodes);

            if (IncomingNodes.Num() == 0) {
                // No incoming nodes
                OutSourceNodes.Add(NodeId);
            }
        }
    }
}

TMap<FGuid, float> FFlowAbstractGraphUtils::CalculateNodeWeights(const FFlowAbstractGraphQuery& GraphQuery, float LockedWeight) {
    TMap<FGuid, float> Weights;
    TSet<FGuid> VisitedNodes;
    
    TArray<FGuid> EntranceNodes;
    FindNodesWithItemType(GraphQuery, EFlowGraphItemType::Entrance, EntranceNodes);
    for (const FGuid& EntranceNode : EntranceNodes) {
        CalculateIslandWeights(GraphQuery, EntranceNode, LockedWeight, Weights, TSet<FGuid>(), VisitedNodes);
    }

    const TSet<FGuid> LockedNodes = VisitedNodes;
    
    // Handle all unprocessed nodes
    const FFlowAbstractGraphTraversal& GraphTraversal = GraphQuery.GetTraversal();
    UFlowAbstractGraphBase* Graph = GraphQuery.GetGraph<UFlowAbstractGraphBase>();
    if (Graph) {
        for (UFlowAbstractNode* Node : Graph->GraphNodes) {
            if (!Node->bActive) continue;
            if (!VisitedNodes.Contains(Node->NodeId)) {
                // This node has not been visited. Process this island
                TArray<FGuid> IslandNodes;
                GetIslandNodes(GraphTraversal, Node->NodeId, IslandNodes);

                if (IslandNodes.Num() > 0) {
                    TArray<FGuid> IslandSourceNodes;
                    GetIslandSourceNodes(GraphTraversal, IslandNodes, IslandSourceNodes);
                
                    if (IslandSourceNodes.Num() == 0) {
                        // This island has no source node (i.e. a node with no incoming links).  This is a cycle. start with the first island node
                        CalculateIslandWeights(GraphQuery, Node->NodeId, LockedWeight, Weights, LockedNodes, VisitedNodes);
                    }
                    else {
                        for (const FGuid& IslandSourceNode : IslandSourceNodes) {
                            CalculateIslandWeights(GraphQuery, IslandSourceNode, LockedWeight, Weights, LockedNodes, VisitedNodes);
                        } 
                    }
                }
            }
        } 
    }
    
    return Weights;
}

bool FFlowAbstractGraphUtils::FindNodeWithItemType(const FFlowAbstractGraphQuery& GraphQuery, EFlowGraphItemType ItemType, FGuid& OutNodeId) {
    TArray<FGuid> NodeIds;
    FindNodesWithItemType(GraphQuery, ItemType, NodeIds);
    if (NodeIds.Num() == 0) {
        return false;
    }
    OutNodeId = NodeIds[0];
    return true;
}

void FFlowAbstractGraphUtils::FindNodesWithItemType(const FFlowAbstractGraphQuery& GraphQuery, EFlowGraphItemType ItemType, TArray<FGuid>& OutNodeIds) {
    for (UFlowAbstractNode* BaseNode : GraphQuery.GetGraph<UFlowAbstractGraphBase>()->GraphNodes) {
        UFlowAbstractNode* Node = Cast<UFlowAbstractNode>(BaseNode);
        for (const UFlowGraphItem* Item : Node->NodeItems) {
            if (Item->ItemType == ItemType) {
                OutNodeIds.Add(Node->NodeId);
            }
        }
    }
}

bool FFlowAbstractGraphUtils::ContainsItem(const TArray<UFlowGraphItem*>& Items, EFlowGraphItemType ItemType) {
    for (const UFlowGraphItem* Item : Items) {
        if (Item->ItemType == ItemType) {
            return true;
        }
    }
    return false;
}

UFlowGraphItem* FFlowAbstractGraphUtils::FindItem(const FGuid& ItemId, const TArray<UFlowGraphItem*>& Items) {
    for (UFlowGraphItem* Item : Items) {
        if (Item->ItemId == ItemId) {
            return Item;
        }
    }
    return nullptr;
}

TArray<FGuid> FFlowAbstractGraphUtils::FindNodesOnPath(UFlowAbstractGraphBase* InGraph, const FString& PathName) {
    TArray<FGuid> PathNodes;
    for (const UFlowAbstractNode* BaseNode : InGraph->GraphNodes) {
        const UFlowAbstractNode* Node = Cast<UFlowAbstractNode>(BaseNode);
        if (Node && Node->PathName == PathName) {
            PathNodes.Add(Node->NodeId);
        }
    }
    return PathNodes;
}

TArray<FGuid> FFlowAbstractGraphUtils::FilterNodes(const TArray<FGuid>& NodeIds, float MinWeight, float MaxWeight,
                                              const TMap<FGuid, float>& Weights) {
    TArray<FGuid> ValidNodes;

    for (const FGuid& NodeId : NodeIds) {
        const float* WeightPtr = Weights.Find(NodeId);
        if (WeightPtr) {
            float Weight = *WeightPtr;
            if (Weight >= MinWeight && Weight <= MaxWeight) {
                ValidNodes.Add(NodeId);
            }
        }
    }

    return ValidNodes;
}

bool FFlowAbstractGraphUtils::ResolveKeyLocks(const FFlowAbstractGraphQuery& GraphQuery,
                                              struct FFlowGraphItemContainer& KeyInfo,
                                              struct FFlowGraphItemContainer& LockInfo)
{
    UFlowAbstractNode* LockNode = LockInfo.HostNode.Get();
    if (!LockNode) return false;

    UFlowAbstractGraphBase* Graph = GraphQuery.GetGraph<UFlowAbstractGraphBase>();
    TArray<UFlowAbstractLink*> IncomingLinks;
    TArray<UFlowAbstractLink*> OutgoingLinks;
    {
        for (UFlowAbstractLink* Link : Graph->GraphLinks) {
            if (Link->Destination == LockNode->NodeId && Link->Type != EFlowAbstractLinkType::Unconnected) {
                IncomingLinks.Add(Link);
            }
            if (Link->Source == LockNode->NodeId && Link->Type != EFlowAbstractLinkType::Unconnected) {
                OutgoingLinks.Add(Link);
            }
        }
    }

    bool bCanLockIncoming = true;
    bool bCanLockOutgoing = true;
    if (IncomingLinks.Num() == 0) {
        bCanLockIncoming = false;
    }
    if (OutgoingLinks.Num() == 0) {
        bCanLockOutgoing = false;
    }

    {
        void* KeyParent = KeyInfo.GetParentObject();
        void* LockParent = LockInfo.GetParentObject();
        if (KeyParent == LockParent && LockParent) {
            bCanLockIncoming = false;
        }
    }

    if (!bCanLockIncoming && !bCanLockOutgoing) {
        return false;
    }

    UFlowGraphItem* LockItem = LockInfo.GetItem();
    LockNode->NodeItems.Remove(LockItem);
    {
        UFlowGraphItem* KeyItem = KeyInfo.GetItem();
        KeyItem->ReferencedItemIds.Remove(LockInfo.ItemId);
    }

    TArray<UFlowAbstractLink*> LinksToLock;
    if (bCanLockIncoming && bCanLockOutgoing) {
        // We can lock either the incoming or outgoing.  Choose the one that requires less links to be locked
        if (IncomingLinks.Num() == OutgoingLinks.Num()) {
            LinksToLock = IncomingLinks;
        }
        else {
            LinksToLock = (OutgoingLinks.Num() < IncomingLinks.Num()) ? OutgoingLinks : IncomingLinks;
        }
    }
    else {
        LinksToLock = bCanLockOutgoing ? OutgoingLinks : IncomingLinks;
    }

    // Add a lock on the links
    for (UFlowAbstractLink* Link : LinksToLock) {
        // TODO: Reparent the lock item to the link (it is currently parented to the node)
        LockItem->ItemId = FGuid::NewGuid();
        Link->LinkItems.Add(LockItem);

        UFlowGraphItem* KeyItem = KeyInfo.GetItem();
        KeyItem->ReferencedItemIds.Add(LockItem->ItemId);
    }

    return true;
}

bool FFlowAbstractGraphUtils::CanReachNode(const FFlowAbstractGraphQuery& GraphQuery, const FGuid& StartNode, const FGuid& EndNode,
        bool bIgnoreDirection, bool bIgnoreOneWayDoors, bool bTraverseTeleporters, TFunction<bool(const FFlowAbstractGraphTraversal::FNodeInfo&)> CanTraverse) {

    const FFlowAbstractGraphTraversal& Traversal = GraphQuery.GetTraversal();
    
    TSet<FGuid> VisitedNodes;
    TArray<FGuid> Stack;
    Stack.Push(StartNode);
    while (Stack.Num() > 0) {
        FGuid NodeId = Stack.Pop();
        if (NodeId == EndNode) {
            return true;
        }

        VisitedNodes.Add(NodeId);

        // Grab the connected nodes
        TArray<FFlowAbstractGraphTraversal::FNodeInfo> ConnectedNodes;
        Traversal.GetOutgoingNodes(NodeId, ConnectedNodes);
        if (bIgnoreDirection) {
            Traversal.GetIncomingNodes(NodeId, ConnectedNodes);
        }
        if (bTraverseTeleporters) {
            FGuid ConnectedTeleNodeId;
            if (Traversal.GetTeleportNode(NodeId, ConnectedTeleNodeId)) {
                FFlowAbstractGraphTraversal::FNodeInfo& ConnectedNode = ConnectedNodes.AddDefaulted_GetRef();
                ConnectedNode.NodeId = ConnectedTeleNodeId;
                ConnectedNode.LinkId = FGuid();
            }
        }

        // Traverse through them
        for (const FFlowAbstractGraphTraversal::FNodeInfo& ConnectedNode : ConnectedNodes) {
            //if (DisallowedLinks.Contains(ConnectedNode.LinkId)) continue;
            if (!CanTraverse(ConnectedNode)) continue;
            if (VisitedNodes.Contains(ConnectedNode.NodeId)) continue;
            if (!bIgnoreOneWayDoors) {
                UFlowAbstractLink* Link = GraphQuery.GetLink(ConnectedNode.LinkId);
                if (Link && Link->Type == EFlowAbstractLinkType::OneWay) {
                    // Make sure we can pass through the one-way door
                    if (Link->Source != NodeId) {
                        // Cannot pass through the one-way link
                        continue;
                    }
                }
            }

            Stack.Push(ConnectedNode.NodeId);
        }
    }

    return false;
}


bool FFlowAbstractGraphUtils::CanPromoteToOneWayLink(const FFlowAbstractGraphQuery& GraphQuery, const FGuid& InLinkId) {
    // Try to reach the source node from the destination, without traversing through the provided link and other oneway links (that are in the wrong direction)
    // This makes sure that the level is playable after turning it to a one way link, as the player will be able to reach it through another path from the dest node

    UFlowAbstractLink* Link = GraphQuery.GetLink(InLinkId);
    if (!Link) return false;
    
    return CanReachNode(GraphQuery, Link->Destination, Link->Source, true, false, true,
        [InLinkId](const FFlowAbstractGraphTraversal::FNodeInfo& TraverseInfo) -> bool {
            return TraverseInfo.LinkId != InLinkId;
        });
}

