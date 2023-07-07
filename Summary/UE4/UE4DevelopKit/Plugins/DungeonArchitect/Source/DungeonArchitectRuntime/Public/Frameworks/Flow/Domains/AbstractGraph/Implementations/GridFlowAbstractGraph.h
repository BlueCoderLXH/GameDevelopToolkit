//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraph.h"
#include "Frameworks/Flow/Domains/FlowDomain.h"
#include "Frameworks/Flow/ExecGraph/FlowExecTask.h"
#include "GridFlowAbstractGraph.generated.h"

///////////////////////////// Grid Flow Abstract Graph /////////////////////////////
UCLASS()
class DUNGEONARCHITECTRUNTIME_API UGridFlowAbstractGraph : public UFlowAbstractGraphBase, public IFlowExecCloneableState {
    GENERATED_BODY()
    
public:
    virtual void CloneFromStateObject(UObject* SourceObject) override;

public:
    UPROPERTY()
    FIntPoint GridSize;
};

class DUNGEONARCHITECTRUNTIME_API FGridFlowAbstractGraphDomain : public IFlowDomain {
public:
    virtual FName GetDomainID() const override;
    virtual FText GetDomainDisplayName() const override;
    virtual void GetDomainTasks(TArray<UClass*>& OutTaskClasses) const override;
    
public:
    static const FName DomainID;
};

