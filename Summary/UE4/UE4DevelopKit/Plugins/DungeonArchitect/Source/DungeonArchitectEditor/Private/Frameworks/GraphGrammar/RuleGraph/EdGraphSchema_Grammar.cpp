//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/GraphGrammar/RuleGraph/EdGraphSchema_Grammar.h"

#include "Core/Utils/DungeonGraphUtils.h"
#include "Frameworks/GraphGrammar/GraphGrammar.h"
#include "Frameworks/GraphGrammar/RuleGraph/EdGraph_Grammar.h"
#include "Frameworks/GraphGrammar/RuleGraph/Nodes/EdGraphNode_GrammarBase.h"
#include "Frameworks/GraphGrammar/RuleGraph/Nodes/EdGraphNode_GrammarNode.h"

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"

#define LOCTEXT_NAMESPACE "EdGraphSchema_Grammar"

FGrammarGraphSupport* UEdGraphSchema_Grammar::GrammarGraphSupport = nullptr;

UEdGraphSchema_Grammar::UEdGraphSchema_Grammar(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
}

void UEdGraphSchema_Grammar::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const {
    const UEdGraph_Grammar* Graph = Cast<UEdGraph_Grammar>(ContextMenuBuilder.CurrentGraph);

    // Add the node actions
    {
        TArray<TSharedPtr<FEdGraphSchemaAction>> NodeActions;
        GetNodeActionList(NodeActions, ContextMenuBuilder.OwnerOfTemporaries, Graph);

        for (TSharedPtr<FEdGraphSchemaAction> Action : NodeActions) {
            ContextMenuBuilder.AddAction(Action);
        }
    }
}

void UEdGraphSchema_Grammar::GetContextMenuActions(class UToolMenu* Menu,
                                                   class UGraphNodeContextMenuContext* Context) const {
    if (GrammarGraphSupport) {
        GrammarGraphSupport->GetContextMenuActions(Menu, Context);
    }
}

class FGrammarSchemaUtils {
public:
    template <typename T>
    static void AddNodeAction(UGrammarNodeType* InTypeInfo, TArray<TSharedPtr<FEdGraphSchemaAction>>& OutActions,
                              UEdGraph* OwnerOfTemporaries) {
        const FText Category = FText::FromString(TEXT("Nodes"));
        const FText Tooltip = InTypeInfo->Description;

        FText MenuDesc;
        if (InTypeInfo->bWildcard) {
            MenuDesc = LOCTEXT("AddNodeWildcard", "Add Wildcard Node (matches any node)");
        }
        else {
            FFormatNamedArguments Args;
            Args.Add(TEXT("NodeName"), FText::FromName(InTypeInfo->TypeName));
            Args.Add(TEXT("NodeDesc"), InTypeInfo->Description);
            MenuDesc = FText::Format(LOCTEXT("AddNodeTextPattern", "Add Node: {NodeName} ({NodeDesc})"), Args);
        }

        T* GrammarNode = NewObject<T>(OwnerOfTemporaries);
        GrammarNode->TypeInfo = InTypeInfo;

        TSharedPtr<FGrammarSchemaAction_NewNode> NewAction = MakeShared<FGrammarSchemaAction_NewNode>(
            Category, MenuDesc, Tooltip, 0);
        NewAction->NodeTemplate = GrammarNode;
        OutActions.Add(NewAction);
    }

};


void UEdGraphSchema_Grammar::GetNodeActionList(TArray<TSharedPtr<FEdGraphSchemaAction>>& OutActions,
                                               UEdGraph* OwnerOfTemporaries, const UEdGraph_Grammar* Graph) const {
    const UGraphGrammar* Grammar = FDungeonGraphUtils::FindInHierarchy<UGraphGrammar>(Graph);
    if (Grammar) {
        for (UGrammarNodeType* NodeType : Grammar->NodeTypes) {
            FGrammarSchemaUtils::AddNodeAction<UEdGraphNode_GrammarNode>(NodeType, OutActions, OwnerOfTemporaries);
        }

        // Add the Wildcard node
        FGrammarSchemaUtils::AddNodeAction<UEdGraphNode_GrammarNode>(Grammar->WildcardType, OutActions,
                                                                     OwnerOfTemporaries);
    }
}

const FPinConnectionResponse UEdGraphSchema_Grammar::CanCreateConnection(
    const UEdGraphPin* A, const UEdGraphPin* B) const {
    // Make sure the data types match
    if (A->PinType.PinCategory != B->PinType.PinCategory) {
        return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Not allowed"));
    }
    // Make sure they are not the same pins
    if (A->GetOwningNode() == B->GetOwningNode()) {
        return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Not allowed"));
    }
    return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT(""));
}

FLinearColor UEdGraphSchema_Grammar::GetPinTypeColor(const FEdGraphPinType& PinType) const {
    return FColor::Yellow;
}

bool UEdGraphSchema_Grammar::ShouldHidePinDefaultValue(UEdGraphPin* Pin) const {
    return false;
}

FConnectionDrawingPolicy* UEdGraphSchema_Grammar::CreateConnectionDrawingPolicy(
    int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect,
    class FSlateWindowElementList& InDrawElements, class UEdGraph* InGraphObj) const {
    if (GrammarGraphSupport) {
        return GrammarGraphSupport->CreateDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect,
                                                        InDrawElements, InGraphObj);
    }
    return UEdGraphSchema::CreateConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect,
                                                         InDrawElements, InGraphObj);
}

#if WITH_EDITOR
bool UEdGraphSchema_Grammar::TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const {
    UEdGraphNode_GrammarBase* NodeB = Cast<UEdGraphNode_GrammarBase>(B->GetOwningNode());
    UEdGraphPin* InputB = NodeB->GetInputPin();
    bool bConnectionMade = UEdGraphSchema::TryCreateConnection(A, InputB);
    if (bConnectionMade) {
        UEdGraph* Graph = A->GetOwningNode()->GetGraph();
        Graph->NotifyGraphChanged();
    }
    return bConnectionMade;
}

void UEdGraphSchema_Grammar::BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const {
    UEdGraphSchema::BreakPinLinks(TargetPin, bSendsNodeNotifcation);
    TargetPin.GetOwningNode()->GetGraph()->NotifyGraphChanged();
}

void UEdGraphSchema_Grammar::BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) const {
    UEdGraphSchema::BreakSinglePinLink(SourcePin, TargetPin);
    SourcePin->GetOwningNode()->GetGraph()->NotifyGraphChanged();
}

void UEdGraphSchema_Grammar::BreakNodeLinks(UEdGraphNode& TargetNode) const {
    UEdGraphSchema::BreakNodeLinks(TargetNode);
    TargetNode.GetGraph()->NotifyGraphChanged();
}
#endif // WITH_EDITOR

//////////////////////////////////////// FGrammarSchemaAction_NewNode ////////////////////////////////////////
UEdGraphNode* FGrammarSchemaAction_NewNode::PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin,
                                                          const FVector2D Location, bool bSelectNewNode /*= true*/) {
    UEdGraphNode* NewNode = FDungeonSchemaAction_NewNode::PerformAction(ParentGraph, FromPin, Location, bSelectNewNode);
    UEdGraphNode_GrammarNode* NewGrammarNode = Cast<UEdGraphNode_GrammarNode>(NewNode);

#if WITH_EDITOR
    // Find a valid unique index to attach to the node
    if (NewGrammarNode) {
        NewGrammarNode->AssignNextAvailableNodeIndex();
    }
#endif // WITH_EDITOR

    return NewNode;
}

#undef LOCTEXT_NAMESPACE

