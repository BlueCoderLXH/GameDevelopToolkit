//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/Tilemap/Tasks/GridFlowTaskTilemapBase.h"


#if WITH_EDITOR
FLinearColor UGridFlowTaskTilemapBase::GetNodeColor() const {
    return FLinearColor(0.08f, 0.08f, 0.08f) + FLinearColor(0, 0, 0.5f) * .25f;
}
#endif // WITH_EDITOR

