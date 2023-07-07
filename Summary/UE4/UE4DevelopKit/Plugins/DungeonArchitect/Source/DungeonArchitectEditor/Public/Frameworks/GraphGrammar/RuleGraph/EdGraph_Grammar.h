//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/GraphGrammar/RuleGraph/Nodes/EdGraphNode_GrammarNode.h"
#include "Frameworks/GraphGrammar/Script/GrammarRuleScript.h"

#include "EdGraph/EdGraph.h"
#include "EdGraph_Grammar.generated.h"

class UGrammarNodeType;

UCLASS()
class DUNGEONARCHITECTEDITOR_API UEdGraph_Grammar : public UEdGraph {
    GENERATED_UCLASS_BODY()

public:
    UEdGraphNode_GrammarNode* AddNewNode(TWeakObjectPtr<UGrammarNodeType> InNodeType);

    template <typename T>
    T* AddNewNodeOfType(TWeakObjectPtr<UGrammarNodeType> InNodeType) {
        T* Node = NewObject<T>(this);
        Node->InitializeNode_Runtime();
        Node->TypeInfo = InNodeType;
        Node->bDisplayIndex = true;
        AddNode(Node);
        return Node;
    }

    //// Begin UEdGraph Interface
    virtual void NotifyGraphChanged() override;
    //// End UEdGraph Interface

    virtual void LayoutGraph(int32 HorizontalSpacing = 150, int32 VerticalSpacing = 100);
};

DUNGEONARCHITECTEDITOR_API void DACopyScriptNodeData(class UGrammarRuleScriptGraphNode* ScriptNode,
                                                     class UEdGraphNode_GrammarNode* RuleEdNode);

