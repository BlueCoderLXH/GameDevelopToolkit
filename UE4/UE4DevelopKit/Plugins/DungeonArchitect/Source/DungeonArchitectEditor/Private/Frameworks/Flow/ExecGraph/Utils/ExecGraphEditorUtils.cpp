//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/ExecGraph/Utils/ExecGraphEditorUtils.h"

#include "Frameworks/Flow/Domains/FlowDomain.h"
#include "Frameworks/Flow/ExecGraph/FlowExecTask.h"

void FExecGraphEditorUtils::AddDomainTaskExtensions(UFlowExecTask* InTask, const TArray<IFlowDomainPtr>& InDomains) {
    // Grab all the domain extenders for this task
    TSet<TSubclassOf<UFlowExecTaskExtender>> ExtensionsToAdd;
    for (IFlowDomainPtr Domain : InDomains) {
        if (Domain.IsValid()) {
            TArray<TSubclassOf<UFlowExecTaskExtender>> DomainExtensions = Domain->GetSupportedTaskExtensions(InTask->GetClass());
            ExtensionsToAdd.Append(DomainExtensions);
        }
    }

    // Remove the extensions that were already added
    for (UFlowExecTaskExtender* Extender : InTask->Extenders) {
        if (!Extender) continue;
        ExtensionsToAdd.Remove(Extender->GetClass());
    }
        
    // Add the new extensions
    for (TSubclassOf<UFlowExecTaskExtender> ExtensionClass : ExtensionsToAdd) {
        UFlowExecTaskExtender* Extender = NewObject<UFlowExecTaskExtender>(InTask, ExtensionClass);
        InTask->Extenders.Add(Extender);
    }

    if (ExtensionsToAdd.Num() > 0) {
        InTask->Modify();
    }
}

