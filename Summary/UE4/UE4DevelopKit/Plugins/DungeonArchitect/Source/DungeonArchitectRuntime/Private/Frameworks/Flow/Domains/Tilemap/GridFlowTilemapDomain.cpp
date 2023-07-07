//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/Tilemap/GridFlowTilemapDomain.h"

#include "Frameworks/Flow/Domains/Tilemap/Tasks/GridFlowTaskTilemap_CreateElevations.h"
#include "Frameworks/Flow/Domains/Tilemap/Tasks/GridFlowTaskTilemap_CreateOverlay.h"
#include "Frameworks/Flow/Domains/Tilemap/Tasks/GridFlowTaskTilemap_Finalize.h"
#include "Frameworks/Flow/Domains/Tilemap/Tasks/GridFlowTaskTilemap_Initialize.h"
#include "Frameworks/Flow/Domains/Tilemap/Tasks/GridFlowTaskTilemap_Merge.h"
#include "Frameworks/Flow/Domains/Tilemap/Tasks/GridFlowTaskTilemap_Optimize.h"

#define LOCTEXT_NAMESPACE "FlowDomainTilemap"

const FName FFlowTilemapDomain::DomainID = TEXT("Tilemap");

FName FFlowTilemapDomain::GetDomainID() const {
    return DomainID;
}

FText FFlowTilemapDomain::GetDomainDisplayName() const {
    return LOCTEXT("DTMDisplayName", "Tilemap");
}

void FFlowTilemapDomain::GetDomainTasks(TArray<UClass*>& OutTaskClasses) const {
    static const TArray<UClass*> DomainTasks = {
        UGridFlowTaskTilemap_Initialize::StaticClass(),
        UGridFlowTaskTilemap_CreateElevations::StaticClass(),
        UGridFlowTaskTilemap_CreateOverlay::StaticClass(),
        UGridFlowTaskTilemap_Merge::StaticClass(),
        UGridFlowTaskTilemap_Optimize::StaticClass(),
        UGridFlowTaskTilemap_Finalize::StaticClass()
    };
    OutTaskClasses = DomainTasks;
}


#undef LOCTEXT_NAMESPACE

