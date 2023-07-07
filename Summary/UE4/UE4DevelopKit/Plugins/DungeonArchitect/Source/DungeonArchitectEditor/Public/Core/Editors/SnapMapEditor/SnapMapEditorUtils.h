//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

class USnapMapAsset;
class UGraphGrammar;
class UGrammarRuleScript;
class UGrammarExecutionScript;
class UGraphGrammarProduction;
class UGrammarNodeType;
class UGrammarScriptGraph;
class UEdGraph_Grammar;
class UEdGraph_DebugGrammar;
class UEdGraph_FlowExec;
class UGrammarScriptGraphNode;
class UEdGraphNode_GrammarNode;

class DUNGEONARCHITECTEDITOR_API FSnapMapEditorUtils {
public:
    static void InitializeFlowAsset(USnapMapAsset* InAsset);

    static UGrammarNodeType* AddNodeType(UGraphGrammar* InGrammar);
    static UGraphGrammarProduction* AddNewRule(UGraphGrammar* InGrammar);
    static UGrammarRuleScript* AddProductionRHSGraph(UGraphGrammarProduction* InProduction);

    static void CompileGrammarToScript(UGraphGrammar* InGrammar);

    static void BuildEdGraphFromScript(UGrammarScriptGraph* ScriptGraph, UEdGraph_Grammar* EdGraph);
    static void BuildDebugEdGraphFromScript(UGrammarScriptGraph* ScriptGraph, UEdGraph_DebugGrammar* EdGraph);
    static UEdGraph_Grammar* GetEdGraph(UGrammarRuleScript* RuleScript);
    static UEdGraph_FlowExec* GetEdGraph(UGrammarExecutionScript* ExecScript);

private:
    static void InitializeMissionGrammar(UGraphGrammar* InGrammar);
    static void InitializeProductionRule(UGraphGrammarProduction* InProduction);
    static void InitializeGraphScript(UGrammarRuleScript* InGraphScript);
    static void InitializeExecutionScript(UGrammarExecutionScript* InExecutionScript,
                                          TWeakObjectPtr<UGraphGrammarProduction> InStartRule);

    template <typename TEdNodeType>
    static void BuildEdGraphFromScriptImpl(UGrammarScriptGraph* ScriptGraph, UEdGraph_Grammar* EdGraph);
};

