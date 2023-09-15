//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractItem.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractLink.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractNode.h"
#include "FlowAbstractGraph.generated.h"

class UFlowAbstractNode;

UCLASS(Abstract)
class DUNGEONARCHITECTRUNTIME_API UFlowAbstractGraphBase : public UObject {
    GENERATED_BODY()

public:
    UPROPERTY() 
    TArray<UFlowAbstractNode*> GraphNodes;

    UPROPERTY()
    TArray<UFlowAbstractLink*> GraphLinks;

    static const FName StateTypeID;

public:
    virtual TSubclassOf<UFlowAbstractNode> GetNodeClass() const;
    virtual TSubclassOf<UFlowAbstractLink> GetLinkClass() const;

    void Clear();
    void RemoveNode(const FGuid& NodeId);
    void RemoveLink(const FGuid& LinkId);

    UFlowAbstractNode* CreateNode();
    UFlowAbstractLink* CreateLink(const FGuid& SourceNodeId, const FGuid& DestNodeId);

    UFlowAbstractNode* GetNode(const FGuid& NodeId);
    UFlowAbstractLink* GetLink(const FGuid& SourceNodeId, const FGuid& DestNodeId);
    UFlowAbstractLink* GetLink(const FGuid& SourceNodeId, const FGuid& DestNodeId, bool bIgnoreDirection);
    TArray<UFlowAbstractLink*> GetLinks(const FGuid& SourceNodeId, const FGuid& DestNodeId);
    TArray<UFlowAbstractLink*> GetLinks(const FGuid& SourceNodeId, const FGuid& DestNodeId, bool bIgnoreDirection);
    UFlowAbstractLink* FindLink(const FGuid& LinkId);
    UFlowAbstractNode* FindSubNode(const FGuid& InSubNodeId);

    void BreakLink(const FGuid& SourceNodeId, const FGuid& DestNodeId);
    void BreakAllOutgoingLinks(const FGuid& NodeId);
    void BreakAllIncomingLinks(const FGuid& NodeId);
    void BreakAllLinks(const FGuid& NodeId);

    TArray<FGuid> GetOutgoingNodes(const FGuid& NodeId);
    TArray<FGuid> GetIncomingNodes(const FGuid& NodeId);
    TArray<FGuid> GetOutgoingLinks(const FGuid& NodeId);
    TArray<FGuid> GetIncomingLinks(const FGuid& NodeId);
    TArray<FGuid> GetConnectedNodes(const FGuid& NodeId);

    void GetAllItems(TArray<UFlowGraphItem*>& OutItems);
    bool VerifyIntegrity();

protected:
    void CopyStateFrom(UFlowAbstractGraphBase* SourceObject);

    
};

