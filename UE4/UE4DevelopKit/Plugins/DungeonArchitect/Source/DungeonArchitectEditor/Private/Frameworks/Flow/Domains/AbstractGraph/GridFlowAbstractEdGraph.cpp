//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/AbstractGraph/GridFlowAbstractEdGraph.h"

#include "Frameworks/Flow/Domains/AbstractGraph/GridFlowAbstractEdGraphSchema.h"

////////////////////////// UEdGraph_FlowExec //////////////////////////

UGridFlowAbstractEdGraph::UGridFlowAbstractEdGraph(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
    Schema = UGridFlowAbstractEdGraphSchema::StaticClass();
}

