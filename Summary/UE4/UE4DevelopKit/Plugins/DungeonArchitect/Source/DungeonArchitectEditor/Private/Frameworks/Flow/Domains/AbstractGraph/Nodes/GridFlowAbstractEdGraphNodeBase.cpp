//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/AbstractGraph/Nodes/GridFlowAbstractEdGraphNodeBase.h"

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraph/EdGraphSchema.h"

#define LOCTEXT_NAMESPACE "UGridFlowAbstractEdGraphNodeBase"

UGridFlowAbstractEdGraphNodeBase::UGridFlowAbstractEdGraphNodeBase(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
    bCanRenameNode = false;
}

UEdGraphPin* UGridFlowAbstractEdGraphNodeBase::GetPin(const FGuid& PinId) const {
    for (UEdGraphPin* Pin : Pins) {
        if (Pin->PinId == PinId) {
            return Pin;
        }
    }
    return nullptr;
}

UEdGraphPin* UGridFlowAbstractEdGraphNodeBase::GetInputPin() const {
    return Pins[0];
}

UEdGraphPin* UGridFlowAbstractEdGraphNodeBase::GetOutputPin() const {
    return Pins[1];
}

void UGridFlowAbstractEdGraphNodeBase::InitializeNode() {
    CreateNewGuid();
    AllocateDefaultPins();
}

#if WITH_EDITOR
void UGridFlowAbstractEdGraphNodeBase::PostEditChangeProperty(struct FPropertyChangedEvent& e) {
    UEdGraphNode::PostEditChangeProperty(e);

    GetGraph()->NotifyGraphChanged();
}

void UGridFlowAbstractEdGraphNodeBase::NodeConnectionListChanged() {
    UEdGraphNode::NodeConnectionListChanged();
}

void UGridFlowAbstractEdGraphNodeBase::AllocateDefaultPins() {
    CreatePin(EGPD_Input, TEXT("Transition"), TEXT("In"));
    CreatePin(EGPD_Output, TEXT("Transition"), TEXT("Out"));
}

void UGridFlowAbstractEdGraphNodeBase::AutowireNewNode(UEdGraphPin* FromPin) {
    if (!FromPin) {
        Super::AutowireNewNode(FromPin);
        return;
    }
    UGridFlowAbstractEdGraphNodeBase* OtherNode = Cast<UGridFlowAbstractEdGraphNodeBase>(FromPin->GetOwningNode());
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

UEdGraphPin* UGridFlowAbstractEdGraphNodeBase::CreatePin(EEdGraphPinDirection Dir, const FName& InPinCategory,
                                                         const FName& PinName, int32 Index /*= INDEX_NONE*/) {
    UEdGraphPin* NewPin = UEdGraphPin::CreatePin(this);
    NewPin->PinName = PinName;
    NewPin->Direction = Dir;

    FEdGraphPinType PinType;
    PinType.PinCategory = InPinCategory;

    NewPin->PinType = PinType;

    Modify(false);
    if (Pins.IsValidIndex(Index)) {
        Pins.Insert(NewPin, Index);
    }
    else {
        Pins.Add(NewPin);
    }
    return NewPin;
}

void UGridFlowAbstractEdGraphNodeBase::PinConnectionListChanged(UEdGraphPin* Pin) {
    Super::PinConnectionListChanged(Pin);
}

#endif //WITH_EDITOR

#undef LOCTEXT_NAMESPACE

