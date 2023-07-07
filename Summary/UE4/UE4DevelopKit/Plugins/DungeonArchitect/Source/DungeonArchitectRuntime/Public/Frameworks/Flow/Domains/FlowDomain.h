//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

class UFlowExecTask;
class UFlowExecTaskExtender;

class DUNGEONARCHITECTRUNTIME_API IFlowDomain {
public:
    virtual ~IFlowDomain() {}
    virtual FName GetDomainID() const = 0;
    virtual FText GetDomainDisplayName() const = 0;
    virtual void GetDomainTasks(TArray<UClass*>& OutTaskClasses) const = 0;

#if WITH_EDITOR
    virtual UFlowExecTask* TryCreateCompatibleTask(UFlowExecTask* InTaskObject) { return nullptr; }
    virtual TArray<TSubclassOf<UFlowExecTaskExtender>> GetSupportedTaskExtensions(TSubclassOf<UFlowExecTask> InTaskClass) { return {}; }
#endif // WITH_EDITOR
    
};
typedef TSharedPtr<class IFlowDomain> IFlowDomainPtr;
typedef TWeakPtr<class IFlowDomain> IFlowDomainWeakPtr;

