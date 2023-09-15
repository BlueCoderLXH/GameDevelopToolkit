//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Common/FlowTaskAbstractBase.h"
#include "FlowTaskAbstract_PathBuilderBase.generated.h"

typedef TSharedPtr<class IFlowAGNodeGroupGenerator> IFlowAGNodeGroupGeneratorPtr;
typedef TSharedPtr<class FFlowAbstractGraphConstraints> FFlowAbstractGraphConstraintsPtr;

class UFlowLayoutNodeCreationConstraint;
struct FFlowAGStaticGrowthState;
struct FFlowAGGrowthState;

UCLASS(Abstract)
class DUNGEONARCHITECTRUNTIME_API UFlowTaskAbstract_PathBuilderBase : public UFlowTaskAbstractBase {
    GENERATED_BODY()

protected:
    virtual void FinalizePath(const FFlowAGStaticGrowthState& StaticState, FFlowAGGrowthState& State) const;
    virtual IFlowAGNodeGroupGeneratorPtr CreateNodeGroupGenerator(TWeakPtr<const IFlowDomain> InDomain) const;
    virtual FFlowAbstractGraphConstraintsPtr CreateGraphConstraints(TWeakPtr<const IFlowDomain> InDomain) const;
    virtual UFlowLayoutNodeCreationConstraint* GetNodeCreationConstraintLogic() const { return nullptr; }
    void ExtendNode(UFlowAbstractNode* InNode) const;
    
    struct FPathGrowthItem {
        FGuid NodeId;
        FGuid PreviousNodeId;
    };
};

