//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/Domains/FlowDomain.h"

class DUNGEONARCHITECTRUNTIME_API FSnapFlowDomain : public IFlowDomain {
    public:
    virtual FName GetDomainID() const override;
    virtual FText GetDomainDisplayName() const override;
    virtual void GetDomainTasks(TArray<UClass*>& OutTaskClasses) const override;
    
#if WITH_EDITOR
    virtual TArray<TSubclassOf<UFlowExecTaskExtender>> GetSupportedTaskExtensions(TSubclassOf<UFlowExecTask> InTaskClass) override;
#endif // WITH_EDITOR
    
public:
    static const FName DomainID;
};

