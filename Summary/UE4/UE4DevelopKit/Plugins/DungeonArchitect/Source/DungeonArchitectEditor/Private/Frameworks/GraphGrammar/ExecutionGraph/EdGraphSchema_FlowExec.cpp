//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/GraphGrammar/ExecutionGraph/EdGraphSchema_FlowExec.h"

#include "Core/Utils/DungeonGraphUtils.h"
#include "Frameworks/GraphGrammar/ExecutionGraph/EdGraph_FlowExec.h"
#include "Frameworks/GraphGrammar/ExecutionGraph/Nodes/EdGraphNode_ExecEntryNode.h"
#include "Frameworks/GraphGrammar/ExecutionGraph/Nodes/EdGraphNode_ExecNodeBase.h"
#include "Frameworks/GraphGrammar/ExecutionGraph/Nodes/EdGraphNode_ExecRuleNode.h"
#include "Frameworks/GraphGrammar/GraphGrammar.h"

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"

#define LOCTEXT_NAMESPACE "EdGraphSchema_FlowExec"

FFlowExecGraphSupport* UEdGraphSchema_FlowExec::ExecGraphSupport = nullptr;

UEdGraphSchema_FlowExec::UEdGraphSchema_FlowExec(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
}

void UEdGraphSchema_FlowExec::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const {
    const UEdGraph_FlowExec* Graph = Cast<UEdGraph_FlowExec>(ContextMenuBuilder.CurrentGraph);

    // Add the node actions
    {
        TArray<TSharedPtr<FEdGraphSchemaAction>> NodeActions;
        GetNodeActionList(NodeActions, ContextMenuBuilder.OwnerOfTemporaries, Graph);

        for (TSharedPtr<FEdGraphSchemaAction> Action : NodeActions) {
            ContextMenuBuilder.AddAction(Action);
        }
    }
}

void UEdGraphSchema_FlowExec::GetContextMenuActions(class UToolMenu* Menu,
                                                    class UGraphNodeContextMenuContext* Context) const {
    if (ExecGraphSupport) {
        ExecGraphSupport->GetContextMenuActions(Menu, Context);
    }
}

class FFlowExecSchemaUtils {
public:
    template <typename T>
    static void AddNodeAction(const FText& InMenuDesc, TArray<TSharedPtr<FEdGraphSchemaAction>>& OutActions,
                              UEdGraph* OwnerOfTemporaries, TFunction<void(T*)> InitTemplate = TFunction<void(T*)>()) {
        const FText Category = LOCTEXT("CategoryLabel", "Rules");
        const FText Tooltip = LOCTEXT("TooltipLabel", "Add Rule node");

        T* Node = NewObject<T>(OwnerOfTemporaries);
        if (InitTemplate) {
            InitTemplate(Node);
        }

        TSharedPtr<FFlowExecSchemaAction_NewNode> NewAction = MakeShared<FFlowExecSchemaAction_NewNode>(
            Category, InMenuDesc, Tooltip, 0);
        NewAction->NodeTemplate = Node;
        OutActions.Add(NewAction);
    }
};


void UEdGraphSchema_FlowExec::GetNodeActionList(TArray<TSharedPtr<FEdGraphSchemaAction>>& OutActions,
                                                UEdGraph* OwnerOfTemporaries, const UEdGraph_FlowExec* Graph) const {
    const UGraphGrammar* Grammar = FDungeonGraphUtils::FindInHierarchy<UGraphGrammar>(Graph);
    if (Grammar) {
        for (UGraphGrammarProduction* Rule : Grammar->ProductionRules) {

            FText MenuDesc;
            FFormatNamedArguments Args;
            Args.Add(TEXT("NodeName"), Rule->RuleName);
            MenuDesc = FText::Format(LOCTEXT("AddNodeTextPattern", "Add Rule: {NodeName}"), Args);

            FFlowExecSchemaUtils::AddNodeAction<UEdGraphNode_ExecRuleNode>(MenuDesc, OutActions, OwnerOfTemporaries,
                                                                           [Rule](UEdGraphNode_ExecRuleNode* Node) {
                                                                               Node->Rule = Rule;
                                                                           }
            );
        }
    }
}

const FPinConnectionResponse UEdGraphSchema_FlowExec::CanCreateConnection(
    const UEdGraphPin* A, const UEdGraphPin* B) const {
    // Make sure the data types match
    if (A->PinType.PinCategory != B->PinType.PinCategory) {
        return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Not allowed"));
    }
    // Make sure they are not the same pins
    if (A->GetOwningNode() == B->GetOwningNode()) {
        return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Not allowed"));
    }

    // Disallow connection on same direction
    if (A->Direction == EGPD_Output && B->GetOwningNode()->IsA<UEdGraphNode_ExecEntryNode>()) {
        return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Not allowed"));
    }

    return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT(""));
}

FLinearColor UEdGraphSchema_FlowExec::GetPinTypeColor(const FEdGraphPinType& PinType) const {
    return FColor::Yellow;
}

bool UEdGraphSchema_FlowExec::ShouldHidePinDefaultValue(UEdGraphPin* Pin) const {
    return false;
}

FConnectionDrawingPolicy* UEdGraphSchema_FlowExec::CreateConnectionDrawingPolicy(
    int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect,
    class FSlateWindowElementList& InDrawElements, class UEdGraph* InGraphObj) const {
    if (ExecGraphSupport) {
        return ExecGraphSupport->CreateDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect,
                                                     InDrawElements, InGraphObj);
    }
    return UEdGraphSchema::CreateConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect,
                                                         InDrawElements, InGraphObj);
}

#if WITH_EDITOR
bool UEdGraphSchema_FlowExec::TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const {
    UEdGraphNode_ExecNodeBase* NodeA = Cast<UEdGraphNode_ExecNodeBase>(A->GetOwningNode());
    UEdGraphNode_ExecNodeBase* NodeB = Cast<UEdGraphNode_ExecNodeBase>(B->GetOwningNode());
    UEdGraphPin* OutputA = NodeA->GetOutputPin();
    UEdGraphPin* InputB = NodeB->GetInputPin();
    if (!OutputA || !InputB) {
        return false;
    }

    bool bConnectionMade = UEdGraphSchema::TryCreateConnection(OutputA, InputB);
    if (bConnectionMade && OutputA && InputB) {
        // Allow only one outgoing link
        TArray<UEdGraphPin*> LinkedPins = A->LinkedTo;
        for (UEdGraphPin* LinkedPin : LinkedPins) {
            if (LinkedPin != InputB) {
                // Break this pin
                OutputA->BreakLinkTo(LinkedPin);
            }
        }
        // Break a reverse link, if it exists
        {
            UEdGraphPin* InputA = NodeA->GetInputPin();
            UEdGraphPin* OutputB = NodeB->GetOutputPin();
            if (InputA && OutputB) {
                OutputB->BreakLinkTo(InputA);
            }
        }

        UEdGraph* Graph = A->GetOwningNode()->GetGraph();
        Graph->NotifyGraphChanged();
    }
    return bConnectionMade;
}

void UEdGraphSchema_FlowExec::BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const {
    UEdGraphSchema::BreakPinLinks(TargetPin, bSendsNodeNotifcation);
    TargetPin.GetOwningNode()->GetGraph()->NotifyGraphChanged();
}

void UEdGraphSchema_FlowExec::BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) const {
    UEdGraphSchema::BreakSinglePinLink(SourcePin, TargetPin);
    SourcePin->GetOwningNode()->GetGraph()->NotifyGraphChanged();
}

void UEdGraphSchema_FlowExec::BreakNodeLinks(UEdGraphNode& TargetNode) const {
    UEdGraphSchema::BreakNodeLinks(TargetNode);
    TargetNode.GetGraph()->NotifyGraphChanged();
}
#endif // WITH_EDITOR

//////////////////////////////////////// FFlowExecSchemaAction_NewNode ////////////////////////////////////////
UEdGraphNode* FFlowExecSchemaAction_NewNode::PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin,
                                                           const FVector2D Location, bool bSelectNewNode /*= true*/) {
    UEdGraphNode* NewNode = FDungeonSchemaAction_NewNode::PerformAction(ParentGraph, FromPin, Location, bSelectNewNode);
    UEdGraphNode_ExecRuleNode* NewRuleNode = Cast<UEdGraphNode_ExecRuleNode>(NewNode);

    return NewNode;
}

#undef LOCTEXT_NAMESPACE

