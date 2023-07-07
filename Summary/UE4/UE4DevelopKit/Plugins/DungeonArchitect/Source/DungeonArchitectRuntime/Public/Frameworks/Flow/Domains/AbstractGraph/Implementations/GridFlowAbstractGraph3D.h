//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraph.h"
#include "Frameworks/Flow/Domains/FlowDomain.h"
#include "Frameworks/Flow/ExecGraph/FlowExecTask.h"
#include "GridFlowAbstractGraph3D.generated.h"

typedef TSharedPtr<class IFlowAGNodeGroupGenerator> IFlowAGNodeGroupGeneratorPtr;
typedef TSharedPtr<class FFlowAbstractGraphConstraints> FFlowAbstractGraphConstraintsPtr;

///////////////////////////// Grid Flow Abstract Graph 3D ///////////////////////////// 
UCLASS()
class DUNGEONARCHITECTRUNTIME_API UGridFlowAbstractGraph3D : public UFlowAbstractGraphBase, public IFlowExecCloneableState {
    GENERATED_BODY()
public:
    virtual void CloneFromStateObject(UObject* SourceObject) override;

public:
    UPROPERTY()
    FIntVector GridSize;
};

class DUNGEONARCHITECTRUNTIME_API FGridFlowAbstractGraph3DDomain : public IFlowDomain {
public:
    virtual FName GetDomainID() const override;
    virtual FText GetDomainDisplayName() const override;
    virtual void GetDomainTasks(TArray<UClass*>& OutTaskClasses) const override;
    
#if WITH_EDITOR
    virtual UFlowExecTask* TryCreateCompatibleTask(UFlowExecTask* InTaskObject) override;
#endif // WITH_EDITOR

    FFlowAbstractGraphConstraintsPtr GetGraphConstraints() const;
    IFlowAGNodeGroupGeneratorPtr GetGroupGenerator() const;
    
    void SetGraphConstraints(FFlowAbstractGraphConstraintsPtr InConstraints);
    void SetGroupGenerator(IFlowAGNodeGroupGeneratorPtr InGroupGenerator);

    public:
    static const FName DomainID;

private:
    FFlowAbstractGraphConstraintsPtr GraphConstraints;
    IFlowAGNodeGroupGeneratorPtr GroupGenerator;
};

