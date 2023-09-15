//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/GraphGrammar/ExecutionGraph/Nodes/EdGraphNode_ExecRuleNode.h"

#include "Frameworks/GraphGrammar/GraphGrammar.h"
#include "Frameworks/GraphGrammar/RuleGraph/EdGraph_Grammar.h"

#include "EdGraph/EdGraphPin.h"

#define LOCTEXT_NAMESPACE "EdGraphNode_GrammarNode"


UEdGraphNode_ExecRuleNode::UEdGraphNode_ExecRuleNode(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
#if WITH_EDITOR
    bCanRenameNode = false;
#endif // WITH_EDITOR

}

#if WITH_EDITOR
void UEdGraphNode_ExecRuleNode::AllocateDefaultPins() {
    AllocateDefaultPins_Runtime();
}

FText UEdGraphNode_ExecRuleNode::GetNodeTitle(ENodeTitleType::Type TitleType) const {
    FText Caption = Rule.IsValid() ? Rule->RuleName : LOCTEXT("DeletedCaption", "[DELETED]");
    return Caption;
}

void UEdGraphNode_ExecRuleNode::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) {
    UEdGraphNode_ExecNodeBase::PostEditChangeProperty(PropertyChangedEvent);
}

void UEdGraphNode_ExecRuleNode::AutowireNewNode(UEdGraphPin* FromPin) {
    if (!FromPin) {
        Super::AutowireNewNode(FromPin);
        return;
    }
    UEdGraphNode_ExecRuleNode* OtherNode = Cast<UEdGraphNode_ExecRuleNode>(FromPin->GetOwningNode());
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

void UEdGraphNode_ExecRuleNode::PinConnectionListChanged(UEdGraphPin* Pin) {
    Super::PinConnectionListChanged(Pin);
}

#endif // WITH_EDITOR

void UEdGraphNode_ExecRuleNode::InitializeNode_Runtime() {
    CreateNewGuid_Runtime();
    AllocateDefaultPins_Runtime();
}

void UEdGraphNode_ExecRuleNode::CreateNewGuid_Runtime() {
    NodeGuid = FGuid::NewGuid();
}

void UEdGraphNode_ExecRuleNode::AllocateDefaultPins_Runtime() {
    CreatePin_Runtime(EGPD_Input, TEXT("Transition"), TEXT("In"));
    CreatePin_Runtime(EGPD_Output, TEXT("Transition"), TEXT("Out"));
}

UEdGraphPin* UEdGraphNode_ExecRuleNode::CreatePin_Runtime(EEdGraphPinDirection Dir, const FName& InPinCategory,
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

UEdGraphPin* UEdGraphNode_ExecRuleNode::GetInputPin() const {
    return Pins[0];
}

UEdGraphPin* UEdGraphNode_ExecRuleNode::GetOutputPin() const {
    return Pins[1];
}


#undef LOCTEXT_NAMESPACE

