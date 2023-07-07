//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Grid3D/Lib/Grid3DFlowAbstractGraphPathUtils.h"

#include "Core/Utils/MathUtils.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Implementations/GridFlowAbstractGraph3D.h"

bool UGrid3DLayoutNodeCreationConstraint::CanCreateLayoutNode_Implementation(const FIntVector& NodeCoord, const FIntVector& GridSize, int32 TotalPathLength, int32 CurrentPathPosition) {
    return true;
}

bool UGrid3DLayoutNodeCreationConstraint::CanCreateNodeAt(const UFlowAbstractNode* Node, int32 TotalPathLength, int32 CurrentPathPosition) {
    const FIntVector Coord = FMathUtils::ToIntVector(Node->Coord, true);
    UGridFlowAbstractGraph3D* Graph3D = Cast<UGridFlowAbstractGraph3D>(Node->GetOuter());
    if (Graph3D) {
        const FIntVector GridSize = Graph3D->GridSize;
        return CanCreateLayoutNode(Coord, GridSize, TotalPathLength, CurrentPathPosition);
    }
    return Super::CanCreateNodeAt(Node, TotalPathLength, CurrentPathPosition);
}

