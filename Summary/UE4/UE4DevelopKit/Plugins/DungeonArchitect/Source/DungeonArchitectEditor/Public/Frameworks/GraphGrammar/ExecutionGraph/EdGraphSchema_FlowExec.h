//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Utils/DungeonGraphUtils.h"

#include "EdGraph/EdGraphSchema.h"
#include "EdGraphSchema_FlowExec.generated.h"

struct FGraphContextMenuBuilder;
class UEdGraph;
class UEdGraph_FlowExec;
class UEdGraphNode_ExecRuleNode;

class DUNGEONARCHITECTEDITOR_API FFlowExecGraphSupport {
public:
    virtual ~FFlowExecGraphSupport() {
    }

    virtual FConnectionDrawingPolicy* CreateDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor,
                                                          const FSlateRect& InClippingRect,
                                                          class FSlateWindowElementList& InDrawElements,
                                                          class UEdGraph* InGraphObj) const { return nullptr; }

    virtual void GetContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const {
    }
};

UCLASS()
class DUNGEONARCHITECTEDITOR_API UEdGraphSchema_FlowExec : public UEdGraphSchema {
    GENERATED_UCLASS_BODY()
public:

    // Begin EdGraphSchema interface
    virtual void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;
    virtual void GetContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const override;
    virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const override;
    virtual class FConnectionDrawingPolicy* CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID,
                                                                          float InZoomFactor, const FSlateRect& InClippingRect,
                                                                          class FSlateWindowElementList& InDrawElements,
                                                                          class UEdGraph* InGraphObj) const override;
    virtual FLinearColor GetPinTypeColor(const FEdGraphPinType& PinType) const override;
    virtual bool ShouldHidePinDefaultValue(UEdGraphPin* Pin) const override;

#if WITH_EDITOR
    virtual bool TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const override;
    virtual void BreakNodeLinks(UEdGraphNode& TargetNode) const override;
    virtual void BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const override;
    virtual void BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) const override;
#endif // WITH_EDITOR
    // End EdGraphSchema interface

private:
    void GetNodeActionList(TArray<TSharedPtr<FEdGraphSchemaAction>>& OutActions, UEdGraph* OwnerOfTemporaries,
                           const UEdGraph_FlowExec* Graph) const;


public:
    static FFlowExecGraphSupport* ExecGraphSupport;
};


/** Action to add a node to the graph */
USTRUCT()
struct DUNGEONARCHITECTEDITOR_API FFlowExecSchemaAction_NewNode : public FDungeonSchemaAction_NewNode {
    GENERATED_USTRUCT_BODY();

    FFlowExecSchemaAction_NewNode()
        : FDungeonSchemaAction_NewNode() {
    }

    FFlowExecSchemaAction_NewNode(const FText& InNodeCategory, const FText& InMenuDesc, const FText& InToolTip,
                                  const int32 InGrouping)
        : FDungeonSchemaAction_NewNode(InNodeCategory, InMenuDesc, InToolTip, InGrouping) {
    }

    virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, FVector2D Location,
                                        bool bSelectNewNode = true) override;
};

