//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Utils/MathUtils.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraph.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractNode.h"

class UFlowAbstractGraphBase;
class UFlowAbstractLink;

class DUNGEONARCHITECTRUNTIME_API FFlowAbstractGraphTraversal {
public:
    struct FNodeInfo {
        FGuid NodeId;
        FGuid LinkId;
        bool bOutgoing = false;
    };

    void Build(UFlowAbstractGraphBase* InGraph);
    void GetOutgoingNodes(const FGuid& InNodeId, TArray<FNodeInfo>& OutResult) const;
    void GetIncomingNodes(const FGuid& InNodeId, TArray<FNodeInfo>& OutResult) const;
    void GetConnectedNodes(const FGuid& InNodeId, TArray<FNodeInfo>& OutResult) const;
    bool GetTeleportNode(const FGuid& InNodeId, FGuid& OutConnectedNodeId) const;
    
private:
    TMap<FGuid, TArray<FNodeInfo>> OutgoingNodes;
    TMap<FGuid, TArray<FNodeInfo>> IncomingNodes;
    TMap<FGuid, FGuid> Teleporters;        // Node -> Node mapping of teleporters
};

template<typename TNode>
class DUNGEONARCHITECTRUNTIME_API TFlowAbstractGraphQuery {
public:
    explicit TFlowAbstractGraphQuery(UFlowAbstractGraphBase* InGraph) {
        Graph = InGraph;
        Build();
    }
    
    TNode* GetNode(const FGuid& NodeId) const {
        const TWeakObjectPtr<UFlowAbstractNode>* SearchResult = NodeMap.Find(NodeId);
        if (!SearchResult) return nullptr;
        const TWeakObjectPtr<UFlowAbstractNode> NodePtr = *SearchResult;
        return Cast<TNode>(NodePtr.Get());
    }

    TNode* GetSubNode(const FGuid& NodeId) const {
        const TWeakObjectPtr<UFlowAbstractNode>* SearchResult = SubNodeMap.Find(NodeId);
        if (!SearchResult) return nullptr;
        const TWeakObjectPtr<UFlowAbstractNode> NodePtr = *SearchResult;
        return Cast<TNode>(NodePtr.Get());
    }
    
    FGuid GetNodeAtCoord(const FVector& InCoord) const {
        const FGuid* SearchResult = CoordToNodeMap.Find(InCoord);
        return SearchResult ? *SearchResult : FGuid();
    }
    
    TNode* GetNodeObjectAtCoord(const FIntVector& InCoord) const {
        return GetNodeObjectAtCoord(FMathUtils::ToVector(InCoord));
    }
    
    TNode* GetNodeObjectAtCoord(const FVector& InCoord) const {
        const FGuid* SearchResult = CoordToNodeMap.Find(InCoord);
        return SearchResult ? GetNode(*SearchResult) : nullptr;
    }
    
    UFlowAbstractLink* GetLink(const FGuid& NodeId) const {
        const TWeakObjectPtr<UFlowAbstractLink>* SearchResult = LinkMap.Find(NodeId);
        if (!SearchResult) return nullptr;
        const TWeakObjectPtr<UFlowAbstractLink> LinkPtr = *SearchResult;
        return LinkPtr.Get();
    };
    
    const FFlowAbstractGraphTraversal& GetTraversal() const { return Traversal; }
    
    template<typename T>
    T* GetGraph() const { return Cast<T>(Graph.Get()); }

    void Rebuild() { Build(); }
    
private:
    void Build() {
        NodeMap.Reset();
        LinkMap.Reset();
        
        for (UFlowAbstractNode* Node : Graph->GraphNodes) {
            NodeMap.Add(Node->NodeId, Node);
            FGuid& NodeIdRef = CoordToNodeMap.FindOrAdd(Node->Coord);
            NodeIdRef = Node->NodeId;
            for (UFlowAbstractNode* SubNode : Node->MergedCompositeNodes) {
                SubNodeMap.Add(SubNode->NodeId, SubNode);
                FGuid& SubNodeIdRef = CoordToNodeMap.FindOrAdd(SubNode->Coord);
                SubNodeIdRef = SubNode->NodeId;
            }
        }
        for (UFlowAbstractLink* Link : Graph->GraphLinks) {
            LinkMap.Add(Link->LinkId, Link);
        }
        
        Traversal.Build(Graph.Get());
    }
    
private:
    TMap<FGuid, TWeakObjectPtr<UFlowAbstractNode>> NodeMap;
    TMap<FGuid, TWeakObjectPtr<UFlowAbstractLink>> LinkMap;
    TWeakObjectPtr<UFlowAbstractGraphBase> Graph;
    FFlowAbstractGraphTraversal Traversal;
    
    TMap<FGuid, TWeakObjectPtr<UFlowAbstractNode>> SubNodeMap;
    TMap<FVector, FGuid> CoordToNodeMap;
};

typedef TFlowAbstractGraphQuery<UFlowAbstractNode> FFlowAbstractGraphQuery;

