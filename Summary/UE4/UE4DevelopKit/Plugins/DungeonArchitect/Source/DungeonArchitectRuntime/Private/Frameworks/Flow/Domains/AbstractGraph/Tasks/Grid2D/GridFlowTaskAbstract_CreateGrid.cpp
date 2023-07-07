//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Grid2D/GridFlowTaskAbstract_CreateGrid.h"

#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraph.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Implementations/GridFlowAbstractGraph.h"
#include "Frameworks/Flow/ExecGraph/FlowExecTaskAttributeMacros.h"

////////////////////////////////////// UGridFlowTaskAbstract_CreateGrid //////////////////////////////////////
void UGridFlowTaskAbstract_CreateGrid::Execute(const FFlowExecutionInput& Input, const FFlowTaskExecutionSettings& InExecSettings, FFlowExecutionOutput& Output) {
    if (Input.IncomingNodeOutputs.Num() != 0) {
        Output.ExecutionResult = EFlowTaskExecutionResult::FailHalt;
        Output.ErrorMessage = "Input not allowed";
        return;
    }

    // Build the graph object
    UGridFlowAbstractGraph* Graph = NewObject<UGridFlowAbstractGraph>();
    Graph->GridSize = GridSize;
    CreateGraph(Graph, FIntVector(GridSize.X, GridSize.Y, 1));

    // Create a new state, since this will our first node
    Output.State = MakeShareable(new FFlowExecNodeState);
    Output.State->SetStateObject(UFlowAbstractGraphBase::StateTypeID, Graph);
    
    Output.ExecutionResult = EFlowTaskExecutionResult::Success;
}

void UGridFlowTaskAbstract_CreateGrid::CreateGraph(UFlowAbstractGraphBase* InGraph, const FIntVector& InGridSize) const {
    TMap<FIntVector, FGuid> Nodes;
    for (int z = 0; z < InGridSize.Z; z++) {
        for (int y = 0; y < InGridSize.Y; y++) {
            for (int x = 0; x < InGridSize.X; x++) {
                FIntVector Coord(x, y, z);
                UFlowAbstractNode* Node = InGraph->CreateNode();
                Node->PreviewLocation = FVector(x, y, z) * SeparationDistance;
                Node->Coord = FVector(x, y, z);
                Nodes.Add(Coord, Node->NodeId);

                if (x > 0) {
                    FGuid PrevNodeX = Nodes[FIntVector(x - 1, y, z)];
                    InGraph->CreateLink(PrevNodeX, Node->NodeId);
                }

                if (y > 0) {
                    FGuid PrevNodeY = Nodes[FIntVector(x, y - 1, z)];
                    InGraph->CreateLink(PrevNodeY, Node->NodeId);
                }
                if (z > 0) {
                    FGuid PrevNodeZ = Nodes[FIntVector(x, y, z - 1)];
                    InGraph->CreateLink(PrevNodeZ, Node->NodeId);
                }
            }
        }
    }
}

bool UGridFlowTaskAbstract_CreateGrid::GetParameter(const FString& InParameterName, FDAAttribute& OutValue) {
    FLOWTASKATTR_GET_SIZE(GridSize);

    return false;
}

bool UGridFlowTaskAbstract_CreateGrid::SetParameter(const FString& InParameterName, const FDAAttribute& InValue) {
    FLOWTASKATTR_SET_SIZEI(GridSize)

    return false;
}

bool UGridFlowTaskAbstract_CreateGrid::SetParameterSerialized(const FString& InParameterName, const FString& InSerializedText) {
    FLOWTASKATTR_SET_PARSE_SIZEI(GridSize)

    return false;
}

