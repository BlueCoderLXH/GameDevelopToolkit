//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraph.h"

#include "Core/Utils/MathUtils.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraphQuery.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractItem.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractNode.h"

///////////////////////////////// UFlowAbstractGraphBase ///////////////////////////////////////
const FName UFlowAbstractGraphBase::StateTypeID = TEXT("AbstractGraphObject");

void UFlowAbstractGraphBase::Clear() {
    GraphNodes.Reset();
    GraphLinks.Reset();
}

void UFlowAbstractGraphBase::RemoveNode(const FGuid& NodeId) {
    BreakAllLinks(NodeId);
    GraphNodes.RemoveAllSwap([NodeId](const UFlowAbstractNode* Node) { return Node->NodeId == NodeId; });
}

void UFlowAbstractGraphBase::RemoveLink(const FGuid& LinkId) {
    GraphLinks.RemoveAllSwap([LinkId](const UFlowAbstractLink* Link) { return Link->LinkId == LinkId; });
}

UFlowAbstractNode* UFlowAbstractGraphBase::CreateNode() {
    UFlowAbstractNode* Node = NewObject<UFlowAbstractNode>(this, GetNodeClass());
    Node->NodeId = FGuid::NewGuid();
    GraphNodes.Add(Node);
    return Node;
}

UFlowAbstractNode* UFlowAbstractGraphBase::GetNode(const FGuid& NodeId) {
    for (UFlowAbstractNode* Node : GraphNodes) {
        if (Node->NodeId == NodeId) {
            return Node;
        }
    }
    return nullptr;
}

UFlowAbstractLink* UFlowAbstractGraphBase::GetLink(const FGuid& SourceNodeId, const FGuid& DestNodeId) {
    return GetLink(SourceNodeId, DestNodeId, false);
}

UFlowAbstractLink* UFlowAbstractGraphBase::GetLink(const FGuid& SourceNodeId, const FGuid& DestNodeId,
                                                       bool bIgnoreDirection) {
    for (UFlowAbstractLink* Link : GraphLinks) {
        if (Link->Source == SourceNodeId && Link->Destination == DestNodeId) {
            return Link;
        }
        if (bIgnoreDirection) {
            if (Link->Source == DestNodeId && Link->Destination == SourceNodeId) {
                return Link;
            }
        }
    }
    return nullptr;
}

TArray<UFlowAbstractLink*> UFlowAbstractGraphBase::GetLinks(const FGuid& SourceNodeId, const FGuid& DestNodeId) {
    return GetLinks(SourceNodeId, DestNodeId, false);
}

TArray<UFlowAbstractLink*> UFlowAbstractGraphBase::GetLinks(const FGuid& SourceNodeId, const FGuid& DestNodeId, bool bIgnoreDirection) {
    TArray<UFlowAbstractLink*> Result;

    for (UFlowAbstractLink* Link : GraphLinks) {
        if (Link->Source == SourceNodeId && Link->Destination == DestNodeId) {
            Result.Add(Link);
        }
        else if (bIgnoreDirection && Link->Source == DestNodeId && Link->Destination == SourceNodeId) {
            Result.Add(Link);
        }
    }

    return Result;
}

UFlowAbstractLink* UFlowAbstractGraphBase::FindLink(const FGuid& LinkId) {
    for (UFlowAbstractLink* Link : GraphLinks) {
        if (Link->LinkId == LinkId) {
            return Link;
        }
    }
    return nullptr;
}

UFlowAbstractNode* UFlowAbstractGraphBase::FindSubNode(const FGuid& InSubNodeId) {
    for (UFlowAbstractNode* Node : GraphNodes) {
        for (UFlowAbstractNode* SubNode : Node->MergedCompositeNodes) {
            if (SubNode->NodeId == InSubNodeId) {
                return SubNode;
            }  
        }
    }
    return nullptr;
}

UFlowAbstractLink* UFlowAbstractGraphBase::CreateLink(const FGuid& SourceNodeId, const FGuid& DestNodeId) {
    // Make sure we don't already have this link 
    for (UFlowAbstractLink* ExistingLink : GraphLinks) {
        if (ExistingLink->Source == SourceNodeId && ExistingLink->Destination == DestNodeId) {
            return ExistingLink;
        }
    }

    UFlowAbstractLink* Link = NewObject<UFlowAbstractLink>(this, GetLinkClass());
    Link->LinkId = FGuid::NewGuid();
    Link->Source = SourceNodeId;
    Link->Destination = DestNodeId;
    GraphLinks.Add(Link);
    return Link;
}

void UFlowAbstractGraphBase::BreakLink(const FGuid& SourceNodeId, const FGuid& DestNodeId) {
    GraphLinks.RemoveAllSwap(
        [SourceNodeId, DestNodeId](const UFlowAbstractLink* Link) {
            return Link->Source == SourceNodeId && Link->Destination == DestNodeId;
        });
}

void UFlowAbstractGraphBase::BreakAllOutgoingLinks(const FGuid& NodeId) {
    GraphLinks.RemoveAllSwap(
        [NodeId](const UFlowAbstractLink* Link) {
            return Link->Source == NodeId;
        });
}

void UFlowAbstractGraphBase::BreakAllIncomingLinks(const FGuid& NodeId) {
    GraphLinks.RemoveAllSwap(
        [NodeId](const UFlowAbstractLink* Link) {
            return Link->Destination == NodeId;
        });
}

void UFlowAbstractGraphBase::BreakAllLinks(const FGuid& NodeId) {
    GraphLinks.RemoveAllSwap(
        [NodeId](const UFlowAbstractLink* Link) {
            return Link->Source == NodeId || Link->Destination == NodeId;
        });
}

TArray<FGuid> UFlowAbstractGraphBase::GetOutgoingNodes(const FGuid& NodeId) {
    TArray<FGuid> Result;
    for (UFlowAbstractLink* Link : GraphLinks) {
        if (Link->Source == NodeId) {
            Result.Add(Link->Destination);
        }
    }
    return Result;
}

TArray<FGuid> UFlowAbstractGraphBase::GetIncomingNodes(const FGuid& NodeId) {
    TArray<FGuid> Result;
    for (UFlowAbstractLink* Link : GraphLinks) {
        if (Link->Destination == NodeId) {
            Result.Add(Link->Source);
        }
    }
    return Result;
}

TArray<FGuid> UFlowAbstractGraphBase::GetOutgoingLinks(const FGuid& NodeId) {
    TArray<FGuid> Result;
    for (UFlowAbstractLink* Link : GraphLinks) {
        if (Link->Source == NodeId) {
            Result.Add(Link->LinkId);
        }
    }
    return Result;
}

TArray<FGuid> UFlowAbstractGraphBase::GetIncomingLinks(const FGuid& NodeId) {
    TArray<FGuid> Result;
    for (UFlowAbstractLink* Link : GraphLinks) {
        if (Link->Destination == NodeId) {
            Result.Add(Link->LinkId);
        }
    }
    return Result;
}

TArray<FGuid> UFlowAbstractGraphBase::GetConnectedNodes(const FGuid& NodeId) {
    TArray<FGuid> Result;

    for (UFlowAbstractLink* Link : GraphLinks) {
        if (Link->Destination == NodeId) {
            Result.Add(Link->Source);
        }
        else if (Link->Source == NodeId) {
            Result.Add(Link->Destination);
        }
    }

    return Result;
}

void UFlowAbstractGraphBase::GetAllItems(TArray<UFlowGraphItem*>& OutItems) {
    OutItems.Reset();
    for (UFlowAbstractNode* Node : GraphNodes) {
        OutItems.Append(Node->NodeItems);
    }
    for (UFlowAbstractLink* Link : GraphLinks) {
        OutItems.Append(Link->LinkItems);
    }
}

bool UFlowAbstractGraphBase::VerifyIntegrity() {
    const FFlowAbstractGraphQuery GraphQuery(this);
    for (UFlowAbstractLink* Link : GraphLinks) {
        UFlowAbstractNode* SourceNode = GraphQuery.GetNode(Link->Source);
        UFlowAbstractNode* DestNode = GraphQuery.GetNode(Link->Source);
        if (!SourceNode || !DestNode) return false;
    }
    return true;
}

TSubclassOf<UFlowAbstractNode> UFlowAbstractGraphBase::GetNodeClass() const {
    return UFlowAbstractNode::StaticClass();
}

TSubclassOf<UFlowAbstractLink> UFlowAbstractGraphBase::GetLinkClass() const {
    return UFlowAbstractLink::StaticClass();
}

namespace {
    template <typename T>
    void DACloneUObjectArray(UObject* Outer, const TArray<T*>& SourceList, TArray<T*>& DestList) {
        DestList.Reset();
        for (T* Source : SourceList) {
            if (!Source) continue;
            T* Clone = NewObject<T>(Outer, Source->GetClass(), NAME_None, RF_NoFlags, Source);
            DestList.Add(Clone);
        }
    }
    
    TArray<UFlowAbstractNode*> CloneTopLevelNodes(UObject* Outer, const TArray<UFlowAbstractNode*>& InSourceNodes) {
        TArray<UFlowAbstractNode*> ClonedItems;
        for (UFlowAbstractNode* SourceNode : InSourceNodes) {
            if (!SourceNode) continue;
            UFlowAbstractNode* ClonedNode = NewObject<UFlowAbstractNode>(Outer, SourceNode->GetClass(), NAME_None, RF_NoFlags, SourceNode);
            ClonedNode->MergedCompositeNodes.Reset();
            ClonedItems.Add(ClonedNode);
        }
        return ClonedItems;
    }
    
}

void UFlowAbstractGraphBase::CopyStateFrom(UFlowAbstractGraphBase* SourceObject) {
    GraphNodes.Reset();
    GraphLinks.Reset();
    
    for (UFlowAbstractNode* Node : SourceObject->GraphNodes) {
        UFlowAbstractNode* NewNode = Node->Clone(this);
        GraphNodes.Add(NewNode);
    }
    for (UFlowAbstractLink* Link : SourceObject->GraphLinks) {
        UFlowAbstractLink* NewLink = Link->Clone(this);
        GraphLinks.Add(NewLink);
    }
}

bool UFlowAbstractNode::ContainsItem(EFlowGraphItemType ItemType, int& Count) {
    Count = 0;
    for (UFlowGraphItem* Item : NodeItems) {
        if (Item && Item->ItemType == ItemType) {
            Count++;
        }
    }
    return Count > 0;
}

///////////////////////////////// UFlowAbstractNode ///////////////////////////////////////
UFlowAbstractNode* UFlowAbstractNode::Clone(UObject* Outer) {
    UFlowAbstractNode* NewNode = NewObject<UFlowAbstractNode>(Outer, GetClass(), NAME_None, RF_NoFlags, this);
    DACloneUObjectArray(NewNode, NodeItems, NewNode->NodeItems);
    DACloneUObjectArray(NewNode, CrossDomainNodeData, NewNode->CrossDomainNodeData);
    NewNode->MergedCompositeNodes = CloneTopLevelNodes(NewNode, MergedCompositeNodes);
    return NewNode;
}

///////////////////////////////// UFlowAbstractLink ///////////////////////////////////////
UFlowAbstractLink* UFlowAbstractLink::Clone(UObject* Outer) {
    UFlowAbstractLink* NewLink = NewObject<UFlowAbstractLink>(Outer, GetClass(), NAME_None, RF_NoFlags, this);
    DACloneUObjectArray(NewLink, LinkItems, NewLink->LinkItems);
    return NewLink;
}

void UFlowAbstractLink::ReverseDirection() {
    FMathUtils::Swap(Source, Destination);
    FMathUtils::Swap(SourceSubNode, DestinationSubNode);
}

