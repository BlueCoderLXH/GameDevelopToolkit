//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Common/FlowTaskAbstract_PathBuilderBase.h"

#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraphConstraints.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractGraphQuery.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Implementations/GridFlowAbstractGraph3D.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Tasks/Common/Lib/FlowAbstractGraphPathUtils.h"
#include "Frameworks/Flow/Domains/FlowDomain.h"

void UFlowTaskAbstract_PathBuilderBase::FinalizePath(const FFlowAGStaticGrowthState& StaticState, FFlowAGGrowthState& State) const {
    FFlowAbstractGraphPathUtils::FinalizePath(StaticState, State);
    if (Extenders.Num() > 0) {
        const FFlowAbstractGraphQuery GraphQuery(StaticState.Graph);
        const int32 PathLength = State.Path.Num();
        for (int PathIdx = 0; PathIdx < PathLength; PathIdx++) {
            const FFlowAGGrowthState_PathItem& Path = State.Path[PathIdx];
            UFlowAbstractNode* Node = GraphQuery.GetNode(Path.NodeId);
            ExtendNode(Node);
        }
    }
}

void UFlowTaskAbstract_PathBuilderBase::ExtendNode(UFlowAbstractNode* InNode) const {
    for (UFlowExecTaskExtender* Extender : Extenders) {
        if (UFlowAbstractGraphTaskExtender* AbstractGraphExtender = Cast<UFlowAbstractGraphTaskExtender>(Extender)) {
            AbstractGraphExtender->ExtendNode(InNode);   
        }
    }
}

IFlowAGNodeGroupGeneratorPtr UFlowTaskAbstract_PathBuilderBase::CreateNodeGroupGenerator(TWeakPtr<const IFlowDomain> InDomainPtr) const {
    IFlowAGNodeGroupGeneratorPtr GroupGenerator;
    
    const TSharedPtr<const IFlowDomain> Domain = InDomainPtr.Pin();
    if (Domain.IsValid()) {
        if (Domain->GetDomainID() == FGridFlowAbstractGraph3DDomain::DomainID) {
            const TSharedPtr<const FGridFlowAbstractGraph3DDomain> DomainAbstract3D = StaticCastSharedPtr<const FGridFlowAbstractGraph3DDomain>(Domain);
            GroupGenerator = DomainAbstract3D->GetGroupGenerator();
        }
    }

    if (!GroupGenerator.IsValid()) {
        GroupGenerator = MakeShareable(new FNullFlowAGNodeGroupGenerator);
    }
    
    return GroupGenerator;
}

FFlowAbstractGraphConstraintsPtr UFlowTaskAbstract_PathBuilderBase::CreateGraphConstraints(TWeakPtr<const IFlowDomain> InDomainPtr) const {
    FFlowAbstractGraphConstraintsPtr GraphConstraint;

    const TSharedPtr<const IFlowDomain> Domain = InDomainPtr.Pin();
    if (Domain.IsValid()) {
        if (Domain->GetDomainID() == FGridFlowAbstractGraph3DDomain::DomainID) {
            const TSharedPtr<const FGridFlowAbstractGraph3DDomain> DomainAbstract3D = StaticCastSharedPtr<const FGridFlowAbstractGraph3DDomain>(Domain);
            GraphConstraint = DomainAbstract3D->GetGraphConstraints();
        }
    }

    if (!GraphConstraint.IsValid()) {
        GraphConstraint = MakeShareable(new FNullFlowAbstractGraphConstraints);
    }
    
    return GraphConstraint;
}

