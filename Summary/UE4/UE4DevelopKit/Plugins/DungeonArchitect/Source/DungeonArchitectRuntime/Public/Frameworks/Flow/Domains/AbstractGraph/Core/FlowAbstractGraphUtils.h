//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraphQuery.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractItem.h"

class UFlowAbstractGraphBase;

class DUNGEONARCHITECTRUNTIME_API FFlowAbstractGraphUtils {
public:
    static TMap<FGuid, float> CalculateNodeWeights(const FFlowAbstractGraphQuery& GraphQuery, float LockedWeight);
    static bool FindNodeWithItemType(const FFlowAbstractGraphQuery& GraphQuery, EFlowGraphItemType ItemType, FGuid& OutNodeId);
    static void FindNodesWithItemType(const FFlowAbstractGraphQuery& GraphQuery, EFlowGraphItemType ItemType, TArray<FGuid>& OutNodeIds);
    static bool ContainsItem(const TArray<UFlowGraphItem*>& Items, EFlowGraphItemType ItemType);
    static UFlowGraphItem* FindItem(const FGuid& ItemId, const TArray<UFlowGraphItem*>& Items);
    static TArray<FGuid> FindNodesOnPath(UFlowAbstractGraphBase* InGraph, const FString& PathName);
    static TArray<FGuid> FilterNodes(const TArray<FGuid>& NodeIds, float MinWeight, float MaxWeight, const TMap<FGuid, float>& Weights);
    static bool ResolveKeyLocks(const FFlowAbstractGraphQuery& GraphQuery, struct FFlowGraphItemContainer& KeyInfo, struct FFlowGraphItemContainer& LockInfo);
    static bool CanPromoteToOneWayLink(const FFlowAbstractGraphQuery& GraphQuery, const FGuid& InLinkId);
    static bool CanReachNode(const FFlowAbstractGraphQuery& GraphQuery, const FGuid& StartNode, const FGuid& EndNode, bool bIgnoreDirection,
            bool bIgnoreOneWayDoors, bool bTraverseTeleporters, TFunction<bool(const FFlowAbstractGraphTraversal::FNodeInfo&)> CanTraverse = [](auto&) { return true; });
    
};

