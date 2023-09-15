//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/AbstractGraph/Implementations/GridFlowAbstractGraph.h"

#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Grid2D/GridFlowTaskAbstract_CreateGrid.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Grid2D/GridFlowTaskAbstract_CreateKeyLock.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Grid2D/GridFlowTaskAbstract_CreateMainPath.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Grid2D/GridFlowTaskAbstract_CreatePath.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Grid2D/GridFlowTaskAbstract_Finalize.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Grid2D/GridFlowTaskAbstract_SpawnItems.h"

#define LOCTEXT_NAMESPACE "FlowDomainAbstractGraph"

///////////////////////////////// UGridFlowAbstractGraph ///////////////////////////////////////
void UGridFlowAbstractGraph::CloneFromStateObject(UObject* SourceObject) {
    UGridFlowAbstractGraph* SourceGraph = Cast<UGridFlowAbstractGraph>(SourceObject);
    if (!SourceGraph) return;
    CopyStateFrom(SourceGraph);
    GridSize = SourceGraph->GridSize;
}

////////////////////////////// FFlowDomainAbstractGraph //////////////////////////////
const FName FGridFlowAbstractGraphDomain::DomainID = TEXT("AbstractGraph");

FName FGridFlowAbstractGraphDomain::GetDomainID() const {
    return DomainID;
}

FText FGridFlowAbstractGraphDomain::GetDomainDisplayName() const {
    return LOCTEXT("DTMDisplayName", "Layout Graph");
}

void FGridFlowAbstractGraphDomain::GetDomainTasks(TArray<UClass*>& OutTaskClasses) const {
    static const TArray<UClass*> DomainTasks = {
        UGridFlowTaskAbstract_CreateGrid::StaticClass(),
        UGridFlowTaskAbstract_CreateMainPath::StaticClass(),
        UGridFlowTaskAbstract_CreatePath::StaticClass(),
        UGridFlowTaskAbstract_SpawnItems::StaticClass(),
        UGridFlowTaskAbstract_CreateKeyLock::StaticClass(),
        UGridFlowTaskAbstract_Finalize::StaticClass()
    };
    OutTaskClasses = DomainTasks;
}

#undef LOCTEXT_NAMESPACE

