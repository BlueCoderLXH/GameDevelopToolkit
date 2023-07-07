//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

extern DUNGEONARCHITECTEDITOR_API void DACopyScriptNodeData(class UGrammarRuleScriptGraphNode* ScriptNode,
                                                            class UEdGraphNode_GrammarNode* RuleEdNode);

template <typename TEdNode, typename TScriptNode, typename TScript>
class TGrammarScriptCompiler {
public:
    static void Compile(TScript* Script) {
        if (!Script) {
            return;
        }
        UGrammarScriptGraph* ScriptGraph = NewObject<UGrammarScriptGraph>(Script);

        // Create the nodes
        TMap<TEdNode*, TScriptNode*> EdToScriptNodes;
        for (UEdGraphNode* EdNode : Script->EdGraph->Nodes) {
            if (TEdNode* RuleEdNode = Cast<TEdNode>(EdNode)) {
                TScriptNode* ScriptNode = NewObject<TScriptNode>(ScriptGraph);
                DACopyScriptNodeData(ScriptNode, RuleEdNode);
                ScriptGraph->Nodes.Add(ScriptNode);
                EdToScriptNodes.Add(RuleEdNode, ScriptNode);
            }
        }

        // Create the link mapping
        for (UEdGraphNode* EdNode : Script->EdGraph->Nodes) {
            if (TEdNode* RuleEdNode = Cast<TEdNode>(EdNode)) {
                TScriptNode** SearchResult = EdToScriptNodes.Find(RuleEdNode);
                if (SearchResult) {
                    TScriptNode* ScriptNode = *SearchResult;
                    for (UEdGraphPin* OutgoingPin : RuleEdNode->GetOutputPin()->LinkedTo) {
                        if (!OutgoingPin) continue;
                        if (TEdNode* OutgoingRuleEdNode = Cast<TEdNode>(OutgoingPin->GetOwningNode())) {
                            // Make an outgoing / incoming link
                            TScriptNode** SearchResultOutgoing = EdToScriptNodes.Find(OutgoingRuleEdNode);
                            if (SearchResultOutgoing) {
                                TScriptNode* OutgoingScriptNode = *SearchResultOutgoing;
                                ScriptNode->OutgoingNodes.Add(OutgoingScriptNode);
                                OutgoingScriptNode->IncomingNodes.Add(ScriptNode);
                            }
                        }
                    }
                }
            }
        }

        Script->ScriptGraph = ScriptGraph;
    }
};

