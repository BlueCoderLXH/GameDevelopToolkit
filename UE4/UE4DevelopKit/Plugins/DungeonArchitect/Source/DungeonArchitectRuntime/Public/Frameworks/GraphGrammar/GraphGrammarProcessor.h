//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

class UGraphGrammarProduction;
class UGrammarNodeType;
class UGraphGrammar;

class UGrammarScriptGraph;
class UGrammarScriptGraphNode;
class UGrammarRuleScriptGraphNode;
class UGrammarExecutionScriptRuleNode;

class UGrammarRuleScript;
class UGrammarExecutionScript;

typedef TMap<UGrammarScriptGraphNode*, UGrammarScriptGraphNode*> NodeToNodeMap_t;

class DUNGEONARCHITECTRUNTIME_API FGraphGrammarProcessor {
public:
    void Initialize(UGrammarScriptGraph* ResultGraph, UGraphGrammar* Grammar, int32 Seed);
    void Execute(UGrammarScriptGraph* ResultGraph, UGraphGrammar* Grammar);

private:
    void ApplyRule(UGrammarScriptGraph* DataGraph, UGraphGrammarProduction* Rule);
    void ApplyRuleSubstitution(UGrammarScriptGraph* DataGraph, UGrammarRuleScript* RuleLHS, UGrammarRuleScript* RuleRHS,
                               const NodeToNodeMap_t& PatternToDataNode);

private:
    FRandomStream Random;
};


struct FGraphPatternMatch {
    NodeToNodeMap_t PatternToDataNode;
};

class FGraphPatternMatcher {
public:
    static void Match(UGrammarScriptGraph* InDataGraph, UGrammarScriptGraph* InPatternGraph,
                      TArray<FGraphPatternMatch>& OutMatches);

private:
    static bool IsNodeDataEqual(UGrammarRuleScriptGraphNode* A, UGrammarRuleScriptGraphNode* B);
    static bool _TraverseChildren(const TArray<UGrammarRuleScriptGraphNode*>& DataChildNodes,
                                  const TArray<UGrammarRuleScriptGraphNode*>& PatternChildNodes,
                                  const TSet<UGrammarScriptGraphNode*>& UnmatchedNodes, FGraphPatternMatch& OutMatch,
                                  TSet<UGrammarScriptGraphNode*>& OutVisited);

    static TArray<UGrammarRuleScriptGraphNode*> GetIncomingNodes(UGrammarRuleScriptGraphNode* InNode);
    static TArray<UGrammarRuleScriptGraphNode*> GetOutgoingNodes(UGrammarRuleScriptGraphNode* InNode);

    static bool MatchRecursive(UGrammarRuleScriptGraphNode* InDataNode, UGrammarRuleScriptGraphNode* InPatternNode,
                               const TSet<UGrammarScriptGraphNode*>& InUnmatchedNodes, FGraphPatternMatch& OutMatch,
                               TSet<UGrammarScriptGraphNode*>& OutVisited);
};

