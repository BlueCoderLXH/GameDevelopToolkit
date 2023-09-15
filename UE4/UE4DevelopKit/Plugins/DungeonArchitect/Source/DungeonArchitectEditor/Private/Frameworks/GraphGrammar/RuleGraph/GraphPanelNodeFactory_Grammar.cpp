//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/GraphGrammar/RuleGraph/GraphPanelNodeFactory_Grammar.h"

#include "Frameworks/GraphGrammar/RuleGraph/Debugging/EdGraphNode_DebugGrammarNode.h"
#include "Frameworks/GraphGrammar/RuleGraph/Debugging/SGraphNode_DebugGrammarNode.h"
#include "Frameworks/GraphGrammar/RuleGraph/Nodes/EdGraphNode_GrammarNode.h"
#include "Frameworks/GraphGrammar/RuleGraph/Widgets/SGraphNode_GrammarNode.h"

TSharedPtr<class SGraphNode> FGraphPanelNodeFactory_Grammar::CreateNode(UEdGraphNode* Node) const {
    if (UEdGraphNode_DebugGrammarNode* DebugTaskNode = Cast<UEdGraphNode_DebugGrammarNode>(Node)) {
        TSharedPtr<SGraphNode_DebugGrammarNode> SNode = SNew(SGraphNode_DebugGrammarNode, DebugTaskNode);
        return SNode;
    }
    if (UEdGraphNode_DebugGrammarDoorNode* DebugDoorNode = Cast<UEdGraphNode_DebugGrammarDoorNode>(Node)) {
        TSharedPtr<SGraphNode_DebugGrammarDoorNode> SNode = SNew(SGraphNode_DebugGrammarDoorNode, DebugDoorNode);
        return SNode;
    }
    if (UEdGraphNode_GrammarNode* TaskNode = Cast<UEdGraphNode_GrammarNode>(Node)) {
        TSharedPtr<SGraphNode_GrammarNode> SNode = SNew(SGraphNode_GrammarNode, TaskNode);
        return SNode;
    }

    return nullptr;
}

