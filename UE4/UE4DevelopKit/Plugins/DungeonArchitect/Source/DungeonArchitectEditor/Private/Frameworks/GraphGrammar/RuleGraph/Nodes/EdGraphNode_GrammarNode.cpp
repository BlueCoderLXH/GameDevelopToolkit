//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/GraphGrammar/RuleGraph/Nodes/EdGraphNode_GrammarNode.h"

#include "Frameworks/GraphGrammar/GraphGrammar.h"
#include "Frameworks/GraphGrammar/RuleGraph/EdGraph_Grammar.h"

#include "EdGraph/EdGraphPin.h"

#define LOCTEXT_NAMESPACE "EdGraphNode_GrammarNode"


UEdGraphNode_GrammarNode::UEdGraphNode_GrammarNode(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
    Index = 0;
    bDisplayIndex = true;
    NodeId = FGuid::NewGuid();

#if WITH_EDITOR
    bCanRenameNode = false;
#endif // WITH_EDITOR

}

#if WITH_EDITOR
void UEdGraphNode_GrammarNode::AllocateDefaultPins() {
    AllocateDefaultPins_Runtime();
}

FText UEdGraphNode_GrammarNode::GetNodeTitle(ENodeTitleType::Type TitleType) const {
    FName Caption = TypeInfo.IsValid() ? TypeInfo->TypeName : "[DELETED]";
    if (bDisplayIndex && TypeInfo.IsValid()) {
        FFormatNamedArguments Arguments;
        Arguments.Add(TEXT("Title"), FText::FromName(Caption));
        Arguments.Add(TEXT("Index"), FText::FromString(FString::FromInt(Index)));
        return FText::Format(LOCTEXT("NodeTitle", "{Title}:{Index}"), Arguments);
    }
    return FText::FromName(Caption);
}

void UEdGraphNode_GrammarNode::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) {
    UEdGraphNode_GrammarBase::PostEditChangeProperty(PropertyChangedEvent);
}

void UEdGraphNode_GrammarNode::AutowireNewNode(UEdGraphPin* FromPin) {
    if (!FromPin) return;
    UEdGraphNode_GrammarNode* OtherNode = Cast<UEdGraphNode_GrammarNode>(FromPin->GetOwningNode());
    if (OtherNode) {
        if (OtherNode->GetOutputPin() == FromPin) {
            UEdGraphPin* OutputPin = FromPin;
            UEdGraphPin* InputPin = GetInputPin();
            const UEdGraphSchema* Schema = GetGraph()->GetSchema();
            const FPinConnectionResponse ConnectionValid = Schema->CanCreateConnection(OutputPin, InputPin);
            if (ConnectionValid.Response == CONNECT_RESPONSE_MAKE) {
                OutputPin->MakeLinkTo(InputPin);
            }
        }
    }
}

void UEdGraphNode_GrammarNode::PinConnectionListChanged(UEdGraphPin* Pin) {
    TArray<FGuid> OutgoingNodes;
    for (UEdGraphPin* OutgoingPin : GetOutputPin()->LinkedTo) {
        UEdGraphNode* OutgoingNode = OutgoingPin->GetOwningNode();
        OutgoingNodes.Add(OutgoingNode->NodeGuid);
    }

    TArray<FGuid> ItemsToRemove;
    for (const FGuid& Item : DependentNodes) {
        if (!OutgoingNodes.Contains(Item)) {
            ItemsToRemove.Add(Item);
        }
    }

    for (const FGuid& Item : ItemsToRemove) {
        DependentNodes.Remove(Item);
    }
}

void UEdGraphNode_GrammarNode::AssignNextAvailableNodeIndex() {
    TSet<int32> IndicesInUse;
    for (UEdGraphNode* Node : GetGraph()->Nodes) {
        if (Node == this) continue;
        UEdGraphNode_GrammarNode* GrammarNode = Cast<UEdGraphNode_GrammarNode>(Node);
        if (GrammarNode) {
            IndicesInUse.Add(GrammarNode->Index);
        }
    }

    Index = 0;
    const int32 MAX_SEARCH_LIMIT = 500;
    for (int i = 0; i < MAX_SEARCH_LIMIT; i++) {
        if (!IndicesInUse.Contains(i)) {
            Index = i;
            break;
        }
    }
}

#endif // WITH_EDITOR

void UEdGraphNode_GrammarNode::InitializeNode_Runtime() {
    CreateNewGuid_Runtime();
    AllocateDefaultPins_Runtime();
}

void UEdGraphNode_GrammarNode::CreateNewGuid_Runtime() {
    NodeGuid = FGuid::NewGuid();
}

void UEdGraphNode_GrammarNode::AllocateDefaultPins_Runtime() {
    CreatePin_Runtime(EGPD_Input, TEXT("Transition"), TEXT("In"));
    CreatePin_Runtime(EGPD_Output, TEXT("Transition"), TEXT("Out"));
}

UEdGraphPin* UEdGraphNode_GrammarNode::CreatePin_Runtime(EEdGraphPinDirection Dir, const FName& InPinCategory,
                                                         const FName& PinName, int32 InIndex /*= INDEX_NONE*/) {
    UEdGraphPin* NewPin = UEdGraphPin::CreatePin(this);
    NewPin->PinName = PinName;
    NewPin->Direction = Dir;

    FEdGraphPinType PinType;
    PinType.PinCategory = InPinCategory;

    NewPin->PinType = PinType;

    Modify(false);
    if (Pins.IsValidIndex(InIndex)) {
        Pins.Insert(NewPin, InIndex);
    }
    else {
        Pins.Add(NewPin);
    }
    return NewPin;
}

UEdGraphPin* UEdGraphNode_GrammarNode::GetInputPin() const {
    return Pins[0];
}

UEdGraphPin* UEdGraphNode_GrammarNode::GetOutputPin() const {
    return Pins[1];
}


#undef LOCTEXT_NAMESPACE

