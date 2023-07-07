//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Utils/DungeonGraphUtils.h"

#include "EdGraph/EdGraphSchema.h"
#include "GridFlowExecEdGraphSchema.generated.h"

class UGridFlowExecEdGraph;
typedef TSharedPtr<class IFlowDomain> IFlowDomainPtr;
typedef TWeakPtr<class IFlowDomain> IFlowDomainWeakPtr;

class DUNGEONARCHITECTEDITOR_API FGridFlowExecSchemaDomainFilter {
public:
    void SetAllowedDomains(const TArray<IFlowDomainWeakPtr>& InAllowedDomains) { AllowedDomains = InAllowedDomains; }
    const TArray<IFlowDomainWeakPtr>& GetAllowedDomains() const { return AllowedDomains; }
private:
    TArray<IFlowDomainWeakPtr> AllowedDomains;
};

UCLASS()
class DUNGEONARCHITECTEDITOR_API UGridFlowExecEdGraphSchema : public UEdGraphSchema {
    GENERATED_UCLASS_BODY()
public:
    // Begin EdGraphSchema interface
    virtual void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;
    virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const override;
    virtual class FConnectionDrawingPolicy* CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID,
                                                                  float InZoomFactor, const FSlateRect& InClippingRect,
                                                                  class FSlateWindowElementList& InDrawElements,
                                                                  class UEdGraph* InGraphObj) const override;
    virtual FLinearColor GetPinTypeColor(const FEdGraphPinType& PinType) const override;
    virtual bool ShouldHidePinDefaultValue(UEdGraphPin* Pin) const override;
    virtual UEdGraphNode* CreateSubstituteNode(UEdGraphNode* Node, const UEdGraph* Graph, FObjectInstancingGraph* InstanceGraph, TSet<FName>& InOutExtraNames) const override;
    
#if WITH_EDITOR
    virtual bool TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const override;
    virtual void BreakNodeLinks(UEdGraphNode& TargetNode) const override;
    virtual void BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const override;
    virtual void BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) const override;
#endif // WITH_EDITOR
    // End EdGraphSchema interface

    void GetActionList(TArray<TSharedPtr<FEdGraphSchemaAction>>& OutActions, const UEdGraph* Graph,
                       UEdGraph* OwnerOfTemporaries) const;

    void SetAllowedDomains(const TArray<IFlowDomainWeakPtr>& InAllowedDomains) const;
    TArray<IFlowDomainWeakPtr> GetAllowedDomains() const;
private:
    TSharedPtr<FGridFlowExecSchemaDomainFilter> DomainFilter;
};


/** Action to add a node to the graph */
USTRUCT()
struct DUNGEONARCHITECTEDITOR_API FGridFlowExecSchemaAction_NewNode : public FDungeonSchemaAction_NewNode {
    GENERATED_USTRUCT_BODY();

    FGridFlowExecSchemaAction_NewNode()
        : FDungeonSchemaAction_NewNode() {
    }

    FGridFlowExecSchemaAction_NewNode(const FText& InNodeCategory, const FText& InMenuDesc, const FText& InToolTip,
                                      const int32 InGrouping, const TArray<IFlowDomainWeakPtr>& InDomains)
        : FDungeonSchemaAction_NewNode(InNodeCategory, InMenuDesc, InToolTip, InGrouping)
        , Domains(InDomains)
    {
    }

    virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, FVector2D Location, bool bSelectNewNode = true) override;

    TArray<IFlowDomainWeakPtr> Domains;
};

