//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/GraphGrammar/ExecutionGraph/GraphPanelNodeFactory_Execution.h"

#include "Frameworks/GraphGrammar/ExecutionGraph/Nodes/EdGraphNode_ExecEntryNode.h"
#include "Frameworks/GraphGrammar/ExecutionGraph/Nodes/EdGraphNode_ExecRuleNode.h"
#include "Frameworks/GraphGrammar/ExecutionGraph/Widgets/SGraphNode_ExecEntryNode.h"
#include "Frameworks/GraphGrammar/ExecutionGraph/Widgets/SGraphNode_ExecRuleNode.h"

TSharedPtr<class SGraphNode> FGraphPanelNodeFactory_Execution::CreateNode(UEdGraphNode* Node) const {
    if (UEdGraphNode_ExecRuleNode* RuleNode = Cast<UEdGraphNode_ExecRuleNode>(Node)) {
        TSharedPtr<SGraphNode_ExecRuleNode> SNode = SNew(SGraphNode_ExecRuleNode, RuleNode);
        return SNode;
    }
    if (UEdGraphNode_ExecEntryNode* EntryNode = Cast<UEdGraphNode_ExecEntryNode>(Node)) {
        TSharedPtr<SGraphNode_ExecEntryNode> SNode = SNew(SGraphNode_ExecEntryNode, EntryNode);
        return SNode;
    }

    return nullptr;
}

