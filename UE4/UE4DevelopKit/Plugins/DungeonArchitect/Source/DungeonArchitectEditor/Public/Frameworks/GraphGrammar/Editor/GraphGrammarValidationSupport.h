//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/GraphGrammar/RuleGraph/Nodes/EdGraphNode_GrammarNode.h"

class SGrammarEditor;
class UEdGraph_Grammar;
class UGraphGrammarProduction;

///////////////////////////// Focus Actions ///////////////////////////// 

struct FGrammarFocusActionContext {
    TSharedPtr<SGrammarEditor> GrammarEditor;
};

class IGrammarFocusAction {
public:
    virtual ~IGrammarFocusAction() {
    }

    virtual void Focus(const FGrammarFocusActionContext& Context) = 0;

};

class FGrammarFocusAction_JumpToNode : public IGrammarFocusAction {
public:
    FGrammarFocusAction_JumpToNode(TWeakObjectPtr<UEdGraphNode> InNode) : Node(InNode) {
    }

    virtual void Focus(const FGrammarFocusActionContext& Context) override;

private:
    TWeakObjectPtr<UEdGraphNode> Node;
};

class FGrammarFocusAction_JumpToGraph : public IGrammarFocusAction {
public:
    FGrammarFocusAction_JumpToGraph(TWeakObjectPtr<UEdGraph_Grammar> InGraph) : Graph(InGraph) {
    }

    virtual void Focus(const FGrammarFocusActionContext& Context) override;
private:
    TWeakObjectPtr<UEdGraph_Grammar> Graph;
};

class FGrammarFocusAction_JumpToRule : public IGrammarFocusAction {
public:
    FGrammarFocusAction_JumpToRule(TWeakObjectPtr<UGraphGrammarProduction> InRule) : Rule(InRule) {
    }

    virtual void Focus(const FGrammarFocusActionContext& Context) override;
private:
    TWeakObjectPtr<UGraphGrammarProduction> Rule;
};

class FGrammarFocusAction_JumpToNodeType : public IGrammarFocusAction {
public:
    FGrammarFocusAction_JumpToNodeType(TWeakObjectPtr<UGrammarNodeType> InNodeType) : NodeType(InNodeType) {
    }

    virtual void Focus(const FGrammarFocusActionContext& Context) override;
private:
    TWeakObjectPtr<UGrammarNodeType> NodeType;
};

class FGrammarFocusAction_FlashNodeTypePanel : public IGrammarFocusAction {
public:
    virtual void Focus(const FGrammarFocusActionContext& Context) override;
};

class FGrammarFocusAction_FlashRulePanel : public IGrammarFocusAction {
public:
    virtual void Focus(const FGrammarFocusActionContext& Context) override;
};

