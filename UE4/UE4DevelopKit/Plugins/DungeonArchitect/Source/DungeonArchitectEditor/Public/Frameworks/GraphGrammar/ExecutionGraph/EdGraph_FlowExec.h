//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph_FlowExec.generated.h"

class UEdGraphNode_ExecEntryNode;
class UEdGraphNode_ExecRuleNode;
class UGraphGrammarProduction;
class UGrammarExecutionScript;

UCLASS()
class DUNGEONARCHITECTEDITOR_API UEdGraph_FlowExec : public UEdGraph {
    GENERATED_UCLASS_BODY()

public:
    UEdGraphNode_ExecRuleNode* AddNewNode(TWeakObjectPtr<UGraphGrammarProduction> InRule);

    //// Begin UEdGraph Interface
    virtual void NotifyGraphChanged() override;
    //// End UEdGraph Interface

public:
    UPROPERTY()
    UEdGraphNode_ExecEntryNode* EntryNode;
};

class UGrammarExecutionScriptRuleNode;
class UGrammarExecutionScriptEntryNode;

class DUNGEONARCHITECTEDITOR_API FGrammarExecutionScriptCompiler {
public:
    static void Compile(UGrammarExecutionScript* Script);

private:
    static void CopyNodeData(UGrammarExecutionScriptRuleNode* RuleScriptNode, UEdGraphNode_ExecRuleNode* RuleEdNode);
    static void CopyNodeData(UGrammarExecutionScriptEntryNode* EntryScriptNode,
                             UEdGraphNode_ExecEntryNode* EntryEdNode);
};

