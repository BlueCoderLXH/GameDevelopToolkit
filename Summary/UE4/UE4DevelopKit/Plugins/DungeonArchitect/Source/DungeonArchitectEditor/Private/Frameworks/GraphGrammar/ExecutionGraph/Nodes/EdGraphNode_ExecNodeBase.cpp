//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/GraphGrammar/ExecutionGraph/Nodes/EdGraphNode_ExecNodeBase.h"

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphPin.h"

UEdGraphNode_ExecNodeBase::UEdGraphNode_ExecNodeBase(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {

}

#if WITH_EDITOR
void UEdGraphNode_ExecNodeBase::PostEditChangeProperty(struct FPropertyChangedEvent& e) {
    UEdGraphNode::PostEditChangeProperty(e);

    GetGraph()->NotifyGraphChanged();
}

void UEdGraphNode_ExecNodeBase::NodeConnectionListChanged() {
    UEdGraphNode::NodeConnectionListChanged();
}
#endif //WITH_EDITOR

UEdGraphPin* UEdGraphNode_ExecNodeBase::GetPin(const FGuid& PinId) const {
    for (UEdGraphPin* Pin : Pins) {
        if (Pin->PinId == PinId) {
            return Pin;
        }
    }
    return nullptr;
}

