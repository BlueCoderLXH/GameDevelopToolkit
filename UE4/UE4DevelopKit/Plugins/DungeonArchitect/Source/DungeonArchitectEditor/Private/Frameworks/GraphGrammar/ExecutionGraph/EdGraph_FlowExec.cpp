//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/GraphGrammar/ExecutionGraph/EdGraph_FlowExec.h"

#include "Frameworks/GraphGrammar/ExecutionGraph/EdGraphSchema_FlowExec.h"
#include "Frameworks/GraphGrammar/ExecutionGraph/Nodes/EdGraphNode_ExecEntryNode.h"
#include "Frameworks/GraphGrammar/ExecutionGraph/Nodes/EdGraphNode_ExecRuleNode.h"
#include "Frameworks/GraphGrammar/GraphGrammar.h"

DEFINE_LOG_CATEGORY_STATIC(LogFlowExecutionGraphScript, Log, All);

////////////////////////// UEdGraph_FlowExec //////////////////////////

UEdGraph_FlowExec::UEdGraph_FlowExec(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
    Schema = UEdGraphSchema_FlowExec::StaticClass();

    EntryNode = ObjectInitializer.CreateDefaultSubobject<UEdGraphNode_ExecEntryNode>(this, "EntryNode");
    EntryNode->InitializeNode_Runtime();
    AddNode(EntryNode);
}

UEdGraphNode_ExecRuleNode* UEdGraph_FlowExec::AddNewNode(TWeakObjectPtr<UGraphGrammarProduction> InRule) {
    UEdGraphNode_ExecRuleNode* Node = NewObject<UEdGraphNode_ExecRuleNode>(this);
    Node->InitializeNode_Runtime();
    Node->Rule = InRule;
    AddNode(Node);
    return Node;
}


void UEdGraph_FlowExec::NotifyGraphChanged() {
    Super::NotifyGraphChanged();

    UGrammarExecutionScript* Script = Cast<UGrammarExecutionScript>(GetOuter());
    FGrammarExecutionScriptCompiler::Compile(Script);
}


//////////////////////////// FGrammarExecutionScriptCompiler //////////////////////////// 

void FGrammarExecutionScriptCompiler::Compile(UGrammarExecutionScript* Script) {
    if (!Script) {
        return;
    }
    UGrammarScriptGraph* ScriptGraph = NewObject<UGrammarScriptGraph>(Script);

    // Create the nodes
    TMap<UEdGraphNode_ExecNodeBase*, UGrammarScriptGraphNode*> EdToScriptNodes;
    for (UEdGraphNode* EdNode : Script->EdGraph->Nodes) {
        if (UEdGraphNode_ExecEntryNode* EntryEdNode = Cast<UEdGraphNode_ExecEntryNode>(EdNode)) {
            UGrammarExecutionScriptEntryNode* EntryScriptNode = NewObject<UGrammarExecutionScriptEntryNode
            >(ScriptGraph);
            CopyNodeData(EntryScriptNode, EntryEdNode);
            ScriptGraph->Nodes.Add(EntryScriptNode);
            EdToScriptNodes.Add(EntryEdNode, EntryScriptNode);
            Script->EntryNode = EntryScriptNode;
        }
        else if (UEdGraphNode_ExecRuleNode* RuleEdNode = Cast<UEdGraphNode_ExecRuleNode>(EdNode)) {
            UGrammarExecutionScriptRuleNode* RuleScriptNode = NewObject<UGrammarExecutionScriptRuleNode>(ScriptGraph);
            CopyNodeData(RuleScriptNode, RuleEdNode);
            ScriptGraph->Nodes.Add(RuleScriptNode);
            EdToScriptNodes.Add(RuleEdNode, RuleScriptNode);
        }
    }

    // Create the link mapping
    for (UEdGraphNode* EdNode : Script->EdGraph->Nodes) {
        if (UEdGraphNode_ExecNodeBase* RuleEdNode = Cast<UEdGraphNode_ExecNodeBase>(EdNode)) {
            UGrammarScriptGraphNode* ScriptNode = EdToScriptNodes[RuleEdNode];
            for (UEdGraphPin* OutgoingPin : RuleEdNode->GetOutputPin()->LinkedTo) {
                if (UEdGraphNode_ExecNodeBase* OutgoingRuleEdNode = Cast<UEdGraphNode_ExecNodeBase>(
                    OutgoingPin->GetOwningNode())) {
                    // Make an outgoing / incoming link
                    UGrammarScriptGraphNode* OutgoingScriptNode = EdToScriptNodes[OutgoingRuleEdNode];
                    ScriptNode->OutgoingNodes.Add(OutgoingScriptNode);
                    OutgoingScriptNode->IncomingNodes.Add(ScriptNode);
                }
            }
        }
    }

    Script->ScriptGraph = ScriptGraph;
}

void FGrammarExecutionScriptCompiler::CopyNodeData(UGrammarExecutionScriptRuleNode* RuleScriptNode,
                                                   UEdGraphNode_ExecRuleNode* RuleEdNode) {
    RuleScriptNode->Rule = RuleEdNode->Rule;
    RuleScriptNode->ExecutionMode = RuleEdNode->ExecutionMode;
    RuleScriptNode->ExecutionConfig = RuleEdNode->ExecutionConfig;
}

void FGrammarExecutionScriptCompiler::CopyNodeData(UGrammarExecutionScriptEntryNode* EntryScriptNode,
                                                   UEdGraphNode_ExecEntryNode* EntryEdNode) {

}

