//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Common/FlowTaskAbstractBase.h"


#if WITH_EDITOR
FLinearColor UFlowTaskAbstractBase::GetNodeColor() const {
    return FLinearColor(0.08f, 0.08f, 0.08f) + FLinearColor(0.5f, 0, 0.2f) * .25f;

}
#endif // WITH_EDITOR

