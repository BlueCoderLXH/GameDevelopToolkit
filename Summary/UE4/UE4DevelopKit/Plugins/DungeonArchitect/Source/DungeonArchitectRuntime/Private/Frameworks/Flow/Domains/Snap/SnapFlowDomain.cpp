//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/Snap/SnapFlowDomain.h"

#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Common/FlowTaskAbstract_PathBuilderBase.h"
#include "Frameworks/Flow/Domains/Snap/SnapFlowAbstractGraphSupport.h"

#define LOCTEXT_NAMESPACE "FlowDomainAbstractGraph"

const FName FSnapFlowDomain::DomainID = TEXT("SnapFlowDomain");

FName FSnapFlowDomain::GetDomainID() const {
    return DomainID;
}

FText FSnapFlowDomain::GetDomainDisplayName() const {
    return LOCTEXT("DTMDisplayName", "Snap");
}

void FSnapFlowDomain::GetDomainTasks(TArray<UClass*>& OutTaskClasses) const {
    static const TArray<UClass*> DomainTasks = { };
    OutTaskClasses = DomainTasks;
}

#if WITH_EDITOR
TArray<TSubclassOf<UFlowExecTaskExtender>> FSnapFlowDomain::GetSupportedTaskExtensions(TSubclassOf<UFlowExecTask> InTaskClass) {
    TArray<TSubclassOf<UFlowExecTaskExtender>> Extenders;
    if (InTaskClass->IsChildOf(UFlowTaskAbstract_PathBuilderBase::StaticClass())) {
        Extenders.Add(USnapFlowAGTaskExtender::StaticClass());
    }
    return Extenders;
}
#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE

