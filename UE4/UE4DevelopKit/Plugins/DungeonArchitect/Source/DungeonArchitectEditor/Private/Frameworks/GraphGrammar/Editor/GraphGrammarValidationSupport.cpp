//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/GraphGrammar/Editor/GraphGrammarValidationSupport.h"

#include "Frameworks/GraphGrammar/Editor/SGrammarEditor.h"

#define LOCTEXT_NAMESPACE "GraphGrammarValidationSupport"

//////////////// Focus Actions //////////////// 
void FGrammarFocusAction_JumpToNode::Focus(const FGrammarFocusActionContext& Context) {
    if (!Node.IsValid()) return;
    Context.GrammarEditor->FocusOnGraphNode(Node.Get());
}

void FGrammarFocusAction_JumpToGraph::Focus(const FGrammarFocusActionContext& Context) {
    if (!Graph.IsValid()) return;
    Context.GrammarEditor->FocusOnGraph(Graph.Get());
}

void FGrammarFocusAction_JumpToRule::Focus(const FGrammarFocusActionContext& Context) {
    if (!Rule.IsValid()) return;
    Context.GrammarEditor->FocusOnRule(Rule.Get());
}

void FGrammarFocusAction_JumpToNodeType::Focus(const FGrammarFocusActionContext& Context) {
    if (!NodeType.IsValid()) return;
    Context.GrammarEditor->FocusOnNodeType(NodeType.Get());
}


void FGrammarFocusAction_FlashNodeTypePanel::Focus(const FGrammarFocusActionContext& Context) {
    Context.GrammarEditor->FlashNodeTypePanel();
}

void FGrammarFocusAction_FlashRulePanel::Focus(const FGrammarFocusActionContext& Context) {
    Context.GrammarEditor->FlashRulesPanel();
}

#undef LOCTEXT_NAMESPACE

