//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/GraphGrammar/RuleGraph/Nodes/EdGraphNode_GrammarBase.h"

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphPin.h"

UEdGraphNode_GrammarBase::UEdGraphNode_GrammarBase(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {

}

#if WITH_EDITOR
void UEdGraphNode_GrammarBase::PostEditChangeProperty(struct FPropertyChangedEvent& e) {
    UEdGraphNode::PostEditChangeProperty(e);

    GetGraph()->NotifyGraphChanged();
}

void UEdGraphNode_GrammarBase::NodeConnectionListChanged() {
    UEdGraphNode::NodeConnectionListChanged();
}
#endif //WITH_EDITOR

UEdGraphPin* UEdGraphNode_GrammarBase::GetPin(const FGuid& PinId) const {
    for (UEdGraphPin* Pin : Pins) {
        if (Pin->PinId == PinId) {
            return Pin;
        }
    }
    return nullptr;
}

