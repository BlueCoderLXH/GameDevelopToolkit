//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/AbstractGraph/Implementations/GridFlowAbstractGraph3D.h"

#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Grid3D/Grid3DFlowTaskAbstract_CreateGrid.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Grid3D/Grid3DFlowTaskAbstract_CreateKeyLock.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Grid3D/Grid3DFlowTaskAbstract_CreateMainPath.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Grid3D/Grid3DFlowTaskAbstract_CreatePath.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Grid3D/Grid3DFlowTaskAbstract_Finalize.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Grid3D/Grid3DFlowTaskAbstract_SpawnItems.h"

#define LOCTEXT_NAMESPACE "FlowDomainAbstractGraph3D"

///////////////////////////////// UGridFlowAbstractGraph3D ///////////////////////////////////////
void UGridFlowAbstractGraph3D::CloneFromStateObject(UObject* SourceObject) {
    UGridFlowAbstractGraph3D* SourceGraph = Cast<UGridFlowAbstractGraph3D>(SourceObject);
    if (!SourceGraph) return;
    CopyStateFrom(SourceGraph);
    GridSize = SourceGraph->GridSize;
}


////////////////////////////// FFlowDomainAbstractGraph3D //////////////////////////////
const FName FGridFlowAbstractGraph3DDomain::DomainID = TEXT("AbstractGraph3D");
FName FGridFlowAbstractGraph3DDomain::GetDomainID() const {
    return DomainID;
}

FText FGridFlowAbstractGraph3DDomain::GetDomainDisplayName() const {
    return LOCTEXT("DTM3DDisplayName", "Layout Graph 3D");
}

void FGridFlowAbstractGraph3DDomain::GetDomainTasks(TArray<UClass*>& OutTaskClasses) const {
    static const TArray<UClass*> DomainTasks = {
        UGrid3DFlowTaskAbstract_CreateGrid::StaticClass(),
        UGrid3DFlowTaskAbstract_CreateMainPath::StaticClass(),
        UGrid3DFlowTaskAbstract_CreatePath::StaticClass(),
        UGrid3DFlowTaskAbstract_SpawnItems::StaticClass(),
        UGrid3DFlowTaskAbstract_CreateKeyLock::StaticClass(),
        UGrid3DFlowTaskAbstract_Finalize::StaticClass()
    };
    OutTaskClasses = DomainTasks;
}

#if WITH_EDITOR
UFlowExecTask* FGridFlowAbstractGraph3DDomain::TryCreateCompatibleTask(UFlowExecTask* InTaskObject) {
    if (!InTaskObject) {
        return nullptr;
    }

    TSubclassOf<UFlowExecTask> TargetTaskClass = nullptr;
    if (InTaskObject->IsA<UFlowTaskAbstract_CreateMainPath>()) {
        TargetTaskClass = UGrid3DFlowTaskAbstract_CreateMainPath::StaticClass();
    }
    else if (InTaskObject->IsA<UFlowTaskAbstract_CreatePath>()) {
        TargetTaskClass = UGrid3DFlowTaskAbstract_CreatePath::StaticClass();
    }
    else if (InTaskObject->IsA<UFlowTaskAbstract_SpawnItems>()) {
        TargetTaskClass = UGrid3DFlowTaskAbstract_SpawnItems::StaticClass();
    }
    else if (InTaskObject->IsA<UFlowTaskAbstract_CreateKeyLock>()) {
        TargetTaskClass = UGrid3DFlowTaskAbstract_CreateKeyLock::StaticClass();
    }
    else if (InTaskObject->IsA<UFlowTaskAbstract_Finalize>()) {
        TargetTaskClass = UGrid3DFlowTaskAbstract_Finalize::StaticClass();
    }

    if (!TargetTaskClass) {
        return nullptr;
    }

    UFlowExecTask* CompatibleTask = NewObject<UFlowExecTask>(InTaskObject->GetOuter(), TargetTaskClass);
    UEngine::CopyPropertiesForUnrelatedObjects(InTaskObject, CompatibleTask);
    return CompatibleTask;
}
#endif // WITH_EDITOR

FFlowAbstractGraphConstraintsPtr FGridFlowAbstractGraph3DDomain::GetGraphConstraints() const {
    return GraphConstraints;
}

void FGridFlowAbstractGraph3DDomain::SetGraphConstraints(FFlowAbstractGraphConstraintsPtr InConstraints) {
    GraphConstraints = InConstraints;
}

IFlowAGNodeGroupGeneratorPtr FGridFlowAbstractGraph3DDomain::GetGroupGenerator() const {
    return GroupGenerator;
}

void FGridFlowAbstractGraph3DDomain::SetGroupGenerator(IFlowAGNodeGroupGeneratorPtr InGroupGenerator) {
    GroupGenerator = InGroupGenerator;
}

#undef LOCTEXT_NAMESPACE

