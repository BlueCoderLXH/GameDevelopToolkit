//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/GraphGrammar/ExecutionGraph/Nodes/EdGraphNode_ExecEntryNode.h"

#include "EdGraph/EdGraphPin.h"

#define LOCTEXT_NAMESPACE "EdGraphNode_ExecEntryNode"


UEdGraphNode_ExecEntryNode::UEdGraphNode_ExecEntryNode(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
#if WITH_EDITOR
    bCanRenameNode = false;
#endif // WITH_EDITOR

}

#if WITH_EDITOR
void UEdGraphNode_ExecEntryNode::AllocateDefaultPins() {
    AllocateDefaultPins_Runtime();
}

FText UEdGraphNode_ExecEntryNode::GetNodeTitle(ENodeTitleType::Type TitleType) const {
    return LOCTEXT("EntryCaption", "Entry");
}


#endif // WITH_EDITOR

void UEdGraphNode_ExecEntryNode::InitializeNode_Runtime() {
    CreateNewGuid_Runtime();
    AllocateDefaultPins_Runtime();
}

void UEdGraphNode_ExecEntryNode::CreateNewGuid_Runtime() {
    NodeGuid = FGuid::NewGuid();
}

void UEdGraphNode_ExecEntryNode::AllocateDefaultPins_Runtime() {
    CreatePin_Runtime(EGPD_Output, TEXT("Transition"), TEXT("Entry"));
}

UEdGraphPin* UEdGraphNode_ExecEntryNode::CreatePin_Runtime(EEdGraphPinDirection Dir, const FName& InPinCategory,
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

UEdGraphPin* UEdGraphNode_ExecEntryNode::GetOutputPin() const {
    return Pins[0];
}


#undef LOCTEXT_NAMESPACE

