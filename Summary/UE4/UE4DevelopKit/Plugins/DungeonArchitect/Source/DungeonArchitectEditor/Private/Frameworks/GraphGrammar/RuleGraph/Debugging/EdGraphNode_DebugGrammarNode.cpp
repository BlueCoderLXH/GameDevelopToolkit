//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/GraphGrammar/RuleGraph/Debugging/EdGraphNode_DebugGrammarNode.h"


UEdGraphNode_DebugGrammarNode::UEdGraphNode_DebugGrammarNode(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
    ResetState();
}

void UEdGraphNode_DebugGrammarNode::ResetState() {
    bProcessed = false;
    ResetModuleState();
}

void UEdGraphNode_DebugGrammarNode::ResetModuleState() {
    bModuleAssigned = false;
    ModuleLevel = nullptr;
    ModuleBounds = FBox(ForceInit);
    WorldTransform = FTransform::Identity;
    IncomingDoorIndex = -1;
    RemoteDoorIndex = -1;
    IncomingDoorId = FGuid();
    RemoteDoorId = FGuid();
}

