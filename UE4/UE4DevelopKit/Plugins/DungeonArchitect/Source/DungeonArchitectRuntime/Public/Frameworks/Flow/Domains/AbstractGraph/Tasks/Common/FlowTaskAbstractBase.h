//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/ExecGraph/FlowExecTask.h"
#include "FlowTaskAbstractBase.generated.h"

UCLASS(Abstract)
class DUNGEONARCHITECTRUNTIME_API UFlowTaskAbstractBase : public UFlowExecTask {
    GENERATED_BODY()
public:
#if WITH_EDITOR
    virtual FLinearColor GetNodeColor() const override;
#endif // WITH_EDITOR
};

UCLASS(Abstract)
class DUNGEONARCHITECTRUNTIME_API UFlowAbstractGraphTaskExtender : public UFlowExecTaskExtender {
    GENERATED_BODY()
public:
    virtual void ExtendNode(UFlowAbstractNode* Node) {}
};


