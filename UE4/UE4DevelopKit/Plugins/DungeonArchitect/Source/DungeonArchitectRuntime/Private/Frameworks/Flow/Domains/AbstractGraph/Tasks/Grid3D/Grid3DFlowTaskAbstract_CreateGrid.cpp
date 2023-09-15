//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Grid3D/Grid3DFlowTaskAbstract_CreateGrid.h"

#include "Frameworks/Flow/Domains/AbstractGraph/Implementations/GridFlowAbstractGraph3D.h"
#include "Frameworks/Flow/ExecGraph/FlowExecTaskAttributeMacros.h"

////////////////////////////////////// UGridFlowTaskAbstract_CreateGrid3D //////////////////////////////////////

void UGrid3DFlowTaskAbstract_CreateGrid::Execute(const FFlowExecutionInput& Input, const FFlowTaskExecutionSettings& InExecSettings, FFlowExecutionOutput& Output) {
    if (Input.IncomingNodeOutputs.Num() != 0) {
        Output.ExecutionResult = EFlowTaskExecutionResult::FailHalt;
        Output.ErrorMessage = "Input not allowed";
        return;
    }

    // Build the graph object
    UGridFlowAbstractGraph3D* Graph = NewObject<UGridFlowAbstractGraph3D>();
    Graph->GridSize = GridSize;
    CreateGraph(Graph, GridSize);

    // Create a new state, since this will our first node
    Output.State = MakeShareable(new FFlowExecNodeState);
    Output.State->SetStateObject(UFlowAbstractGraphBase::StateTypeID, Graph);
    
    Output.ExecutionResult = EFlowTaskExecutionResult::Success;
}

void UGrid3DFlowTaskAbstract_CreateGrid::CreateGraph(UFlowAbstractGraphBase* InGraph, const FIntVector& InGridSize) const {
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

bool UGrid3DFlowTaskAbstract_CreateGrid::GetParameter(const FString& InParameterName, FDAAttribute& OutValue) {
    FLOWTASKATTR_GET_VECTOR(GridSize);

    return false;
    
}

bool UGrid3DFlowTaskAbstract_CreateGrid::SetParameter(const FString& InParameterName, const FDAAttribute& InValue) {
    FLOWTASKATTR_SET_VECTORI(GridSize)

    return false;
    
}

bool UGrid3DFlowTaskAbstract_CreateGrid::SetParameterSerialized(const FString& InParameterName, const FString& InSerializedText) {
    FLOWTASKATTR_SET_PARSE_VECTORI(GridSize)

    return false;
}

