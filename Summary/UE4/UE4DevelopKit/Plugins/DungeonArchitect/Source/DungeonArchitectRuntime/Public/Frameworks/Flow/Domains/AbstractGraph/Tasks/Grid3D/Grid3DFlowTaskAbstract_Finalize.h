//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Common/FlowTaskAbstract_Finalize.h"
#include "Grid3DFlowTaskAbstract_Finalize.generated.h"

UCLASS(Meta = (AbstractTask, Title = "Finalize Graph", Tooltip = "Call this to finalize the layout graph", MenuPriority = 1500))
class DUNGEONARCHITECTRUNTIME_API UGrid3DFlowTaskAbstract_Finalize : public UFlowTaskAbstract_Finalize {
    GENERATED_BODY()
public:

};

