//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Common/FlowTaskAbstract_CreatePath.h"
#include "GridFlowTaskAbstract_CreatePath.generated.h"

UCLASS(Meta = (AbstractTask, Title = "Create Path", Tooltip = "Create a path on an existing network", MenuPriority = 1200))
class DUNGEONARCHITECTRUNTIME_API UGridFlowTaskAbstract_CreatePath : public UFlowTaskAbstract_CreatePath {
    GENERATED_BODY()
public:
};

