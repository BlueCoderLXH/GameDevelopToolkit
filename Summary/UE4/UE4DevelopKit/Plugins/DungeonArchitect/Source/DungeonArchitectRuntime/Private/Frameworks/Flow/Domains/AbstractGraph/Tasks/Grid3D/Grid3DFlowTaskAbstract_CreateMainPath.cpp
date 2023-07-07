//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Grid3D/Grid3DFlowTaskAbstract_CreateMainPath.h"

#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Grid3D/Lib/Grid3DFlowAbstractGraphPathUtils.h"

UFlowLayoutNodeCreationConstraint* UGrid3DFlowTaskAbstract_CreateMainPath::GetNodeCreationConstraintLogic() const {
    return bUseNodeCreationConstraint ? NodeCreationConstraint : nullptr;
}

