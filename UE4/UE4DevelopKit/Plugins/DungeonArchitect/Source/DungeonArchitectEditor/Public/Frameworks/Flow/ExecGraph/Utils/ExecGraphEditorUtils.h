//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

class UFlowExecTask;
typedef TSharedPtr<class IFlowDomain> IFlowDomainPtr;

class FExecGraphEditorUtils {
public:
    /** Adds missing task extensions from the specified domains */
    static void AddDomainTaskExtensions(UFlowExecTask* InTask, const TArray<IFlowDomainPtr>& InDomains);
    
};

