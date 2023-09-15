//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/AbstractGraph/GridFlowAbstractEdGraphSchema.h"

#include "Frameworks/Flow/Domains/AbstractGraph/GridFlowAbstractConnectionDrawingPolicy.h"
#include "Frameworks/Flow/Domains/AbstractGraph/GridFlowAbstractEdGraph.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Nodes/GridFlowAbstractEdGraphNodes.h"

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"

#define LOCTEXT_NAMESPACE "GridFlowAbstractEdGraphSchema"

UGridFlowAbstractEdGraphSchema::UGridFlowAbstractEdGraphSchema(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
}

void UGridFlowAbstractEdGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const {
    const UGridFlowAbstractEdGraph* Graph = Cast<UGridFlowAbstractEdGraph>(ContextMenuBuilder.CurrentGraph);

    // TODO: Add the node actions
}

void UGridFlowAbstractEdGraphSchema::GetContextMenuActions(class UToolMenu* Menu,
                                                           class UGraphNodeContextMenuContext* Context) const {
}

const FPinConnectionResponse UGridFlowAbstractEdGraphSchema::CanCreateConnection(
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
    if (A->Direction == EGPD_Output && B->GetOwningNode()->IsA<UGridFlowAbstractEdGraphNode>()) {
        return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Not allowed"));
    }

    return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT(""));
}

FLinearColor UGridFlowAbstractEdGraphSchema::GetPinTypeColor(const FEdGraphPinType& PinType) const {
    return FColor::Yellow;
}

bool UGridFlowAbstractEdGraphSchema::ShouldHidePinDefaultValue(UEdGraphPin* Pin) const {
    return false;
}

FConnectionDrawingPolicy* UGridFlowAbstractEdGraphSchema::CreateConnectionDrawingPolicy(
    int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect,
    class FSlateWindowElementList& InDrawElements, class UEdGraph* InGraphObj) const {
    return new FGridFlowAbstractConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect,
                                                        InDrawElements, InGraphObj);
}

#if WITH_EDITOR
bool UGridFlowAbstractEdGraphSchema::TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const {
    UGridFlowAbstractEdGraphNodeBase* NodeA = Cast<UGridFlowAbstractEdGraphNodeBase>(A->GetOwningNode());
    UGridFlowAbstractEdGraphNodeBase* NodeB = Cast<UGridFlowAbstractEdGraphNodeBase>(B->GetOwningNode());
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

void UGridFlowAbstractEdGraphSchema::BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const {
    UEdGraphSchema::BreakPinLinks(TargetPin, bSendsNodeNotifcation);
    TargetPin.GetOwningNode()->GetGraph()->NotifyGraphChanged();
}

void UGridFlowAbstractEdGraphSchema::BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) const {
    UEdGraphSchema::BreakSinglePinLink(SourcePin, TargetPin);
    SourcePin->GetOwningNode()->GetGraph()->NotifyGraphChanged();
}

void UGridFlowAbstractEdGraphSchema::BreakNodeLinks(UEdGraphNode& TargetNode) const {
    UEdGraphSchema::BreakNodeLinks(TargetNode);
    TargetNode.GetGraph()->NotifyGraphChanged();
}
#endif // WITH_EDITOR

//////////////////////////////////////// FFlowExecSchemaAction_NewNode ////////////////////////////////////////
UEdGraphNode* FGridFlowAbstractSchemaAction_NewNode::PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin,
                                                                   const FVector2D Location,
                                                                   bool bSelectNewNode /*= true*/) {
    UEdGraphNode* NewNode = FDungeonSchemaAction_NewNode::PerformAction(ParentGraph, FromPin, Location, bSelectNewNode);

    if (UGridFlowAbstractEdGraphNode* TaskNode = Cast<UGridFlowAbstractEdGraphNode>(NewNode)) {
        // TODO: Initialize me
    }

    return NewNode;
}

#undef LOCTEXT_NAMESPACE

