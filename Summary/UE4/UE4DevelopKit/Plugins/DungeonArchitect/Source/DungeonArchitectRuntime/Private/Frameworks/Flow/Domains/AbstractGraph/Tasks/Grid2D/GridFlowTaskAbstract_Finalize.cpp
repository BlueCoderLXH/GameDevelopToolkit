//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Grid2D/GridFlowTaskAbstract_Finalize.h"

#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraphUtils.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Implementations/GridFlowAbstractGraph.h"
#include "Frameworks/Flow/Domains/Tilemap/GridFlowTilemap.h"
#include "Frameworks/Flow/ExecGraph/FlowExecTaskAttributeMacros.h"

class UFANodeTilemapDomainData;

void UGridFlowTaskAbstract_Finalize::Execute(const FFlowExecutionInput& Input, const FFlowTaskExecutionSettings& InExecSettings, FFlowExecutionOutput& Output)
{
    Super::Execute(Input, InExecSettings, Output);
    if (Output.ExecutionResult == EFlowTaskExecutionResult::Success) {
        UFlowAbstractGraphBase* Graph = Output.State->GetState<UFlowAbstractGraphBase>(UFlowAbstractGraphBase::StateTypeID);
        const FFlowAbstractGraphQuery GraphQuery(Graph);
        AssignRoomTypes(GraphQuery, *Input.Random);
    }
}

void UGridFlowTaskAbstract_Finalize::AssignRoomTypes(const FFlowAbstractGraphQuery& GraphQuery,
                                                     const FRandomStream& Random) const {
    UFlowAbstractGraphBase* Graph = GraphQuery.GetGraph<UFlowAbstractGraphBase>();
    for (UFlowAbstractNode* Node : Graph->GraphNodes) {
        Node->FindOrAddDomainData<UFANodeTilemapDomainData>()->RoomType = GetNodeRoomType(GraphQuery, Node);
    }

    // Make another pass and force assign rooms where a link requires a door
    for (const UFlowAbstractLink* Link : Graph->GraphLinks) {
        const bool bContainsLock = FFlowAbstractGraphUtils::ContainsItem(Link->LinkItems, EFlowGraphItemType::Lock);
        if (bContainsLock || Link->Type == EFlowAbstractLinkType::OneWay) {
            // We need at least one room type that supports doors (rooms and corridors)
            UFlowAbstractNode* NodeA = GraphQuery.GetNode(Link->Source);
            UFlowAbstractNode* NodeB = GraphQuery.GetNode(Link->Destination);
            if (!NodeA || !NodeB) continue;

            const EGridFlowAbstractNodeRoomType RoomTypeA = NodeA->FindOrAddDomainData<UFANodeTilemapDomainData>()->RoomType;
            const EGridFlowAbstractNodeRoomType RoomTypeB = NodeB->FindOrAddDomainData<UFANodeTilemapDomainData>()->RoomType;
            
            const bool bContainsDoorA = (RoomTypeA == EGridFlowAbstractNodeRoomType::Room || RoomTypeA == EGridFlowAbstractNodeRoomType::Corridor);
            const bool bContainsDoorB = (RoomTypeB == EGridFlowAbstractNodeRoomType::Room || RoomTypeB == EGridFlowAbstractNodeRoomType::Corridor);
            if (!bContainsDoorA && !bContainsDoorB) {
                // Promote one of them to a room
                UFlowAbstractNode* NodeToPromote = (Random.FRand() < 0.5f) ? NodeA : NodeB;
                NodeToPromote->FindOrAddDomainData<UFANodeTilemapDomainData>()->RoomType = EGridFlowAbstractNodeRoomType::Room;
            }
        }
    }
}

EGridFlowAbstractNodeRoomType UGridFlowTaskAbstract_Finalize::GetNodeRoomType(const FFlowAbstractGraphQuery& GraphQuery, UFlowAbstractNode* Node) const {
    int32 NumEnemies = 0;
    int32 NumKeys = 0;
    int32 NumBonus = 0;
    int32 NumEntrance = 0;
    int32 NumExit = 0;

    for (const UFlowGraphItem* Item : Node->NodeItems) {
        if (Item->ItemType == EFlowGraphItemType::Enemy) NumEnemies++;
        else if (Item->ItemType == EFlowGraphItemType::Key) NumKeys++;
        else if (Item->ItemType == EFlowGraphItemType::Treasure) NumBonus++;
        else if (Item->ItemType == EFlowGraphItemType::Entrance) NumEntrance++;
        else if (Item->ItemType == EFlowGraphItemType::Exit) NumExit++;
    }

    if (NumEntrance > 0 || NumExit > 0 || NumKeys > 0 || NumBonus > 0) {
        return EGridFlowAbstractNodeRoomType::Room;
    }

    UFlowAbstractGraphBase* Graph = GraphQuery.GetGraph<UFlowAbstractGraphBase>();
    TArray<FGuid> IncomingLinks = Graph->GetIncomingLinks(Node->NodeId);
    TArray<FGuid> OutgoingLinks = Graph->GetOutgoingLinks(Node->NodeId);

    // Promote to corridor, if the constraints are satisfied
    if (bGenerateCorridors && IncomingLinks.Num() == 1 && OutgoingLinks.Num() == 1 && NumEnemies == 0) {
        UFlowAbstractLink* IncomingLink = Graph->FindLink(IncomingLinks[0]);
        UFlowAbstractLink* OutgoingLink = Graph->FindLink(OutgoingLinks[0]);
        if (IncomingLink && OutgoingLink) {
            // make sure the incoming and outgoing are in the same line
            UFlowAbstractNode* IncomingNode = GraphQuery.GetNode(IncomingLink->Source);
            UFlowAbstractNode* OutgoingNode = GraphQuery.GetNode(OutgoingLink->Destination);
            if (IncomingNode && OutgoingNode) {
                const FVector CoordIn = IncomingNode->Coord;
                const FVector CoordOut = OutgoingNode->Coord;

                const bool bSameLine = (FMath::IsNearlyEqual(CoordIn.X, CoordOut.X) || FMath::IsNearlyEqual(CoordIn.Y, CoordOut.Y));
                if (bSameLine) {
                    return EGridFlowAbstractNodeRoomType::Corridor;
                }
            }
        }
    }

    if (bGenerateCaves) {
        return NumEnemies <= MaxEnemiesPerCaveNode
                   ? EGridFlowAbstractNodeRoomType::Cave
                   : EGridFlowAbstractNodeRoomType::Room;
    }

    return EGridFlowAbstractNodeRoomType::Room;
}

bool UGridFlowTaskAbstract_Finalize::GetParameter(const FString& InParameterName, FDAAttribute& OutValue) {
    FLOWTASKATTR_GET_BOOL(bGenerateCorridors);
    FLOWTASKATTR_GET_BOOL(bGenerateCaves);
    FLOWTASKATTR_GET_INT(MaxEnemiesPerCaveNode);
    
    return false;
}

bool UGridFlowTaskAbstract_Finalize::SetParameter(const FString& InParameterName, const FDAAttribute& InValue) {
    FLOWTASKATTR_SET_BOOL(bGenerateCorridors);
    FLOWTASKATTR_SET_BOOL(bGenerateCaves);
    FLOWTASKATTR_SET_INT(MaxEnemiesPerCaveNode);
    
    return false;
}

bool UGridFlowTaskAbstract_Finalize::SetParameterSerialized(const FString& InParameterName, const FString& InSerializedText) {
    FLOWTASKATTR_SET_PARSE_BOOL(bGenerateCorridors);
    FLOWTASKATTR_SET_PARSE_BOOL(bGenerateCaves);
    FLOWTASKATTR_SET_PARSE_INT(MaxEnemiesPerCaveNode);
    
    return false;
}

