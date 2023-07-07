//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Grid2D/GridFlowTaskAbstract_CreateMainPath.h"

#include "Core/Utils/Attributes.h"
#include "Core/Utils/MathUtils.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraphQuery.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Implementations/GridFlowAbstractGraph.h"
#include "Frameworks/Flow/Domains/Snap/SnapFlowAbstractGraphSupport.h"
#include "Frameworks/Flow/Domains/Tilemap/GridFlowTilemap.h"
#include "Frameworks/Flow/ExecGraph/FlowExecTaskAttributeMacros.h"

//////////////////////////////// UGridFlowTaskAbstract_CreateMainPathBase ////////////////////////////////
bool UGridFlowTaskAbstract_CreateMainPath::GetParameter(const FString& InParameterName, FDAAttribute& OutValue) {
    if (Super::GetParameter(InParameterName, OutValue)) {
        return true;
    }
    
    FLOWTASKATTR_GET_BOOL(bOverrideStartGridPosition);
    FLOWTASKATTR_GET_SIZE(StartGridPosition);
    FLOWTASKATTR_GET_BOOL(bFullSizeStartRoom);
    FLOWTASKATTR_GET_BOOL(bFullSizeGoalRoom);

    return false;
}

bool UGridFlowTaskAbstract_CreateMainPath::SetParameter(const FString& InParameterName, const FDAAttribute& InValue) {
    if (Super::SetParameter(InParameterName, InValue)) {
        return true;
    }

    FLOWTASKATTR_SET_BOOL(bOverrideStartGridPosition);
    FLOWTASKATTR_SET_SIZEI(StartGridPosition);
    FLOWTASKATTR_SET_BOOL(bFullSizeStartRoom);
    FLOWTASKATTR_SET_BOOL(bFullSizeGoalRoom);
    
    return false;
}

bool UGridFlowTaskAbstract_CreateMainPath::SetParameterSerialized(const FString& InParameterName, const FString& InSerializedText) {
    if (Super::SetParameterSerialized(InParameterName, InSerializedText)) {
        return true;
    }
    
    FLOWTASKATTR_SET_PARSE_BOOL(bOverrideStartGridPosition);
    FLOWTASKATTR_SET_PARSE_SIZEI(StartGridPosition);
    FLOWTASKATTR_SET_PARSE_BOOL(bFullSizeStartRoom);
    FLOWTASKATTR_SET_PARSE_BOOL(bFullSizeGoalRoom);
    
    return false;
}

TArray<int32> UGridFlowTaskAbstract_CreateMainPath::GetPossibleEntranceIndices(UFlowAbstractGraphBase* InGraph, const FRandomStream& InRandom) const {
    TArray<int32> EntranceIndices;
    if (bOverrideStartGridPosition) {
        // Find the node index that has override coordinate
        for (int i = 0; i < InGraph->GraphNodes.Num(); i++) {
            const UFlowAbstractNode* Node = InGraph->GraphNodes[i];
            if (Node && FMathUtils::ToIntPoint(FVector2D(Node->Coord), true) == StartGridPosition) {
                EntranceIndices.Add(i);
                break;
            }
        }
        return EntranceIndices;
    }
    
    return Super::GetPossibleEntranceIndices(InGraph, InRandom);
}

void UGridFlowTaskAbstract_CreateMainPath::FinalizePath(const FFlowAGStaticGrowthState& StaticState, FFlowAGGrowthState& State) const {
    Super::FinalizePath(StaticState, State);
    
    for (int i = 0; i < State.Path.Num(); i++) {
        const FFlowAGGrowthState_PathItem& PathItem = State.Path[i];
        UFlowAbstractNode* Node = StaticState.GraphQuery->GetNode(PathItem.NodeId);
        if (!Node) continue;
        
        UFANodeTilemapDomainData* TilemapDomainData = Node->FindOrAddDomainData<UFANodeTilemapDomainData>(); 
        if (i == 0) {
            TilemapDomainData->bDisablePerturb = bFullSizeStartRoom;
        }
        else if (i == State.Path.Num() - 1) {
            TilemapDomainData->bDisablePerturb = bFullSizeGoalRoom;
        }      
    }
}

