//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/GraphGrammar/GraphGrammarProcessor.h"

#include "Frameworks/GraphGrammar/GraphGrammar.h"
#include "Frameworks/GraphGrammar/Script/GrammarExecutionScript.h"
#include "Frameworks/GraphGrammar/Script/GrammarRuleScript.h"

DEFINE_LOG_CATEGORY_STATIC(LogGraphGrammarProcessor, Log, All);

static const FName DATA_WILDCARD = "*";

namespace {
    UGrammarRuleScriptGraphNode* CreateRuleNode(TWeakObjectPtr<UGrammarNodeType> InNodeType,
                                                UGrammarScriptGraph* OwnerGraph) {
        UGrammarRuleScriptGraphNode* Node = NewObject<UGrammarRuleScriptGraphNode>(OwnerGraph);
        Node->TypeInfo = InNodeType;
        Node->bDisplayIndex = false;
        OwnerGraph->Nodes.Add(Node);
        return Node;
    }

    int32 GetRuleIterationCount(UGrammarExecutionScriptRuleNode* ExecNode, const FRandomStream& Random) {
        if (ExecNode->ExecutionMode == ERuleNodeExecutionMode::RunWithProbability) {
            float RunProbability = Random.FRand();
            return RunProbability <= ExecNode->ExecutionConfig.RunProbability ? 1 : 0;
        }
        if (ExecNode->ExecutionMode == ERuleNodeExecutionMode::Iterate) {
            return ExecNode->ExecutionConfig.IterationCount;
        }
        if (ExecNode->ExecutionMode == ERuleNodeExecutionMode::IterateRange) {
            const float Min = ExecNode->ExecutionConfig.IterationCountMin;
            const float Max = ExecNode->ExecutionConfig.IterationCountMax;
            return FMath::RoundToInt(Min + (Max - Min) * Random.FRand());
        }
        return 1;
    }

    UGrammarRuleScript* GetRandomGraph(const TArray<UGrammarRuleScript*>& GrammarScripts, const FRandomStream& Random) {
        if (GrammarScripts.Num() == 0) return nullptr;
        if (GrammarScripts.Num() == 1) return GrammarScripts[0];

        float TotalWeight = 0;
        for (UGrammarRuleScript* GrammarScript : GrammarScripts) {
            TotalWeight += GrammarScript->Weight;
        }

        // TODO: Check me
        float SelectionValue = Random.FRandRange(0, TotalWeight);
        for (UGrammarRuleScript* GrammarScript : GrammarScripts) {
            if (SelectionValue <= GrammarScript->Weight) {
                return GrammarScript;
            }
            SelectionValue -= GrammarScript->Weight;
        }
        return nullptr;
    }

    TMap<int32, UGrammarRuleScriptGraphNode*> CreateNodeByIndexMap(UGrammarRuleScript* RuleScript) {
        TMap<int32, UGrammarRuleScriptGraphNode*> Result;
        for (UGrammarScriptGraphNode* ScriptNode : RuleScript->ScriptGraph->Nodes) {
            if (UGrammarRuleScriptGraphNode* RuleNode = Cast<UGrammarRuleScriptGraphNode>(ScriptNode)) {
                Result.Add(RuleNode->Index, RuleNode);
            }
        }
        return Result;
    }

}

void FGraphGrammarProcessor::Initialize(UGrammarScriptGraph* ResultGraph, UGraphGrammar* Grammar, int32 Seed) {
    Random.Initialize(Seed);

    // Remove all nodes from the graph
    ResultGraph->Nodes.Reset();

    // Find the start node type info
    UGrammarNodeType* StartNodeType = nullptr;
    for (UGrammarNodeType* NodeTypeInfo : Grammar->NodeTypes) {
        if (NodeTypeInfo->TypeName == "S") {
            StartNodeType = NodeTypeInfo;
            break;
        }
    }

    if (StartNodeType) {
        // Add a start node on the result graph
        CreateRuleNode(StartNodeType, ResultGraph);
    }
}

void FGraphGrammarProcessor::Execute(UGrammarScriptGraph* ResultGraph, UGraphGrammar* Grammar) {
    if (!Grammar || !Grammar->ExecutionGraphScript) {
        UE_LOG(LogGraphGrammarProcessor, Error, TEXT("Invalid Grammar model reference"));
        return;
    }

    UGrammarExecutionScript* ExecutionGraph = Grammar->ExecutionGraphScript;
    if (!ExecutionGraph || !ExecutionGraph->EntryNode) {
        UE_LOG(LogGraphGrammarProcessor, Error, TEXT("Invalid execution graph reference"));
        return;
    }


    UGrammarExecutionScriptEntryNode* EntryNode = ExecutionGraph->EntryNode;


    TSet<UGrammarScriptGraphNode*> Visited;
    Visited.Add(EntryNode);

    TArray<UGrammarScriptGraphNode*> Nodes = EntryNode->OutgoingNodes;
    while (Nodes.Num() > 0) {
        UGrammarScriptGraphNode* Node = Nodes[0];
        if (Visited.Contains(Node)) {
            break;
        }
        Visited.Add(Node);

        if (UGrammarExecutionScriptRuleNode* ExecRuleNode = Cast<UGrammarExecutionScriptRuleNode>(Node)) {
            UGraphGrammarProduction* ProductionRule = ExecRuleNode->Rule.Get();
            if (ProductionRule) {
                int32 NumIterations = GetRuleIterationCount(ExecRuleNode, Random);
                UE_LOG(LogGraphGrammarProcessor, Log, TEXT("Running rule node %d times"), NumIterations);
                for (int i = 0; i < NumIterations; i++) {
                    ApplyRule(ResultGraph, ProductionRule);
                }
            }
        }

        Nodes = Node->OutgoingNodes;
    }
}


void FGraphGrammarProcessor::ApplyRule(UGrammarScriptGraph* DataGraph, UGraphGrammarProduction* Rule) {
    if (Rule->DestGraphs.Num() == 0) return;

    TArray<FGraphPatternMatch> Matches;
    FGraphPatternMatcher::Match(DataGraph, Rule->SourceGraph->ScriptGraph, Matches);

    for (const FGraphPatternMatch& Match : Matches) {
        // Pick a random RHS substitution graph from the rule
        UGrammarRuleScript* LHSGraph = Rule->SourceGraph;
        UGrammarRuleScript* RHSGraph = GetRandomGraph(Rule->DestGraphs, Random);

        // Perform the replacements
        ApplyRuleSubstitution(DataGraph, LHSGraph, RHSGraph, Match.PatternToDataNode);
    }
}

void FGraphGrammarProcessor::ApplyRuleSubstitution(UGrammarScriptGraph* DataGraph, UGrammarRuleScript* RuleLHS,
                                                   UGrammarRuleScript* RuleRHS, const NodeToNodeMap_t& LHSToDataNode) {
    // Break the links
    {
        TArray<UGrammarScriptGraphNode*> DataNodes;
        LHSToDataNode.GenerateValueArray(DataNodes);
        for (UGrammarScriptGraphNode* DataNode : DataNodes) {
            if (!DataNode->IsA<UGrammarRuleScriptGraphNode>()) continue;
            TArray<UGrammarScriptGraphNode*> OutgoingNodes = DataNode->OutgoingNodes;
            for (UGrammarScriptGraphNode* OutgoingNode : OutgoingNodes) {
                if (!OutgoingNode->IsA<UGrammarRuleScriptGraphNode>()) continue;

                if (DataNodes.Contains(OutgoingNode)) {
                    // Break this link
                    //TaskNode->GetOutputPin()->BreakLinkTo(LinkedPin);
                    DataNode->OutgoingNodes.Remove(OutgoingNode);
                    OutgoingNode->IncomingNodes.Remove(DataNode);
                }
            }
        }
    }

    TMap<int32, UGrammarRuleScriptGraphNode*> LHSNodesByIndex = CreateNodeByIndexMap(RuleLHS);
    TMap<int32, UGrammarRuleScriptGraphNode*> RHSNodesByIndex = CreateNodeByIndexMap(RuleRHS);


    NodeToNodeMap_t RHSToDataNode;

    // Delete unused nodes
    {
        // Add all the nodes to the deletion list and then remove the ones that are in use
        TArray<UGrammarScriptGraphNode*> NodeDeletionList;
        LHSToDataNode.GenerateValueArray(NodeDeletionList);

        for (auto& Entry : RHSNodesByIndex) {
            int32 IndexToRetain = Entry.Key;
            UGrammarRuleScriptGraphNode* RHSNode = Entry.Value;
            if (LHSNodesByIndex.Contains(IndexToRetain)) {
                UGrammarScriptGraphNode* LHSNodeToRetain = LHSNodesByIndex[IndexToRetain];
                if (LHSToDataNode.Contains(LHSNodeToRetain)) {
                    UGrammarScriptGraphNode* DataNodeToRetain = LHSToDataNode[LHSNodeToRetain];
                    NodeDeletionList.Remove(DataNodeToRetain);
                    // This node is used in the mapping . Remove it from the deletion list

                    // Update the caption
                    UGrammarRuleScriptGraphNode* DataTaskNode = Cast<UGrammarRuleScriptGraphNode>(DataNodeToRetain);
                    if (!RHSNode->TypeInfo->bWildcard) {
                        DataTaskNode->TypeInfo = RHSNode->TypeInfo;
                    }

                    RHSToDataNode.Add(RHSNode, DataTaskNode);
                }
            }
        }

        // We are now left with the unreferenced nodes that are in the LHS but not the RHS
        for (UGrammarScriptGraphNode* DataNodeToDelete : NodeDeletionList) {
            DataGraph->Nodes.Remove(DataNodeToDelete);
        }
    }


    // Add new nodes
    TArray<UGrammarScriptGraphNode*> NewNodes;
    {
        TArray<int32> RHSNodeIndices;
        RHSNodesByIndex.GenerateKeyArray(RHSNodeIndices);
        for (auto& LHSEntry : LHSNodesByIndex) {
            int32 LHSIndex = LHSEntry.Key;
            RHSNodeIndices.Remove(LHSIndex);
        }

        // We are now left with rhs indices that are not present in the LHS
        // Create them in the data graph
        for (int32 RHSIndexToCreate : RHSNodeIndices) {
            UGrammarRuleScriptGraphNode* RHSNode = RHSNodesByIndex[RHSIndexToCreate];
            UGrammarRuleScriptGraphNode* NewTaskNode = CreateRuleNode(RHSNode->TypeInfo, DataGraph);
            RHSToDataNode.Add(RHSNode, NewTaskNode);

            NewNodes.Add(NewTaskNode);
        }
    }

    // Make the links
    {
        for (auto& Entry : RHSNodesByIndex) {
            UGrammarRuleScriptGraphNode* RHSNode = Entry.Value;
            for (UGrammarScriptGraphNode* OutgoingNode : RHSNode->OutgoingNodes) {
                UGrammarRuleScriptGraphNode* OutgoingRHSNode = Cast<UGrammarRuleScriptGraphNode>(OutgoingNode);

                // Create a link between these two
                UGrammarRuleScriptGraphNode* RHSStartNode = RHSNode;
                UGrammarRuleScriptGraphNode* RHSEndNode = OutgoingRHSNode;
                if (RHSToDataNode.Contains(RHSStartNode) && RHSToDataNode.Contains(RHSEndNode)) {

                    UGrammarRuleScriptGraphNode* DataNodeLinkStart = Cast<UGrammarRuleScriptGraphNode>(
                        RHSToDataNode[RHSStartNode]);
                    UGrammarRuleScriptGraphNode* DataNodeLinkEnd = Cast<UGrammarRuleScriptGraphNode>(
                        RHSToDataNode[RHSEndNode]);

                    // Create a dependency, if specified
                    // TODO: Handle dependency
                    /*
                    if (RHSStartNode->DependentNodes.Contains(RHSEndNode->NodeGuid)) {
                        DataNodeLinkStart->DependentNodes.Add(DataNodeLinkEnd->NodeGuid);
                    }
                    */

                    //DataNodeLinkStart->GetOutputPin()->MakeLinkTo(DataNodeLinkEnd->GetInputPin());
                    DataNodeLinkStart->OutgoingNodes.Add(DataNodeLinkEnd);
                    DataNodeLinkEnd->IncomingNodes.Add(DataNodeLinkStart);
                }
            }
        }
    }
}

///////////////////////////// FGraphPatternMatcher ///////////////////////////// 

void FGraphPatternMatcher::Match(UGrammarScriptGraph* InDataGraph, UGrammarScriptGraph* InPatternGraph,
                                 TArray<FGraphPatternMatch>& OutMatches) {
    OutMatches.Reset();
    if (InPatternGraph->Nodes.Num() == 0) return;
    TSet<UGrammarScriptGraphNode*> UnmatchedNodes(InDataGraph->Nodes);

    while (true) {
        bool bFoundMatch = false;
        for (UGrammarScriptGraphNode* DataNode : InDataGraph->Nodes) {
            if (!UnmatchedNodes.Contains(DataNode)) {
                // This node was matched previously. Ignore it
                continue;
            }
            FGraphPatternMatch Match;
            TSet<UGrammarScriptGraphNode*> Visited;

            if (MatchRecursive(
                Cast<UGrammarRuleScriptGraphNode>(DataNode),
                Cast<UGrammarRuleScriptGraphNode>(InPatternGraph->Nodes[0]),
                UnmatchedNodes, Match, Visited)) {

                // Register the matched result
                OutMatches.Add(Match);
                bFoundMatch = true;

                // Remove the matches nodes from the valid node list
                // so that they are not matched again
                for (auto& Entry : Match.PatternToDataNode) {
                    UGrammarScriptGraphNode* MatchedDataNode = Entry.Value;
                    UnmatchedNodes.Remove(MatchedDataNode);
                }
            }
        }
        if (!bFoundMatch) {
            break;
        }
    }
}

bool FGraphPatternMatcher::IsNodeDataEqual(UGrammarRuleScriptGraphNode* A, UGrammarRuleScriptGraphNode* B) {
    if (!A->TypeInfo.IsValid() || !B->TypeInfo.IsValid()) return false;

    if (A->TypeInfo->bWildcard || B->TypeInfo->bWildcard) return true;
    return A->TypeInfo == B->TypeInfo;
}

bool FGraphPatternMatcher::_TraverseChildren(const TArray<UGrammarRuleScriptGraphNode*>& DataChildNodes,
                                             const TArray<UGrammarRuleScriptGraphNode*>& PatternChildNodes,
                                             const TSet<UGrammarScriptGraphNode*>& UnmatchedNodes,
                                             FGraphPatternMatch& OutMatch, TSet<UGrammarScriptGraphNode*>& OutVisited) {
    if (PatternChildNodes.Num() == 0) {
        return true;
    }

    bool bFoundAllChildPaths = true;

    // Traverse to children
    for (UGrammarRuleScriptGraphNode* IncomingPatternNode : PatternChildNodes) {
        if (!IncomingPatternNode) continue;
        if (OutVisited.Contains(IncomingPatternNode)) continue;

        // Find an incoming node from the data graph that matches this pattern incoming node
        bool bFoundPath = false;
        for (UGrammarRuleScriptGraphNode* IncomingDataNode : DataChildNodes) {
            if (OutVisited.Contains(IncomingDataNode)) continue;
            if (!IsNodeDataEqual(IncomingPatternNode, IncomingDataNode)) continue;

            if (MatchRecursive(IncomingDataNode, IncomingPatternNode, UnmatchedNodes, OutMatch, OutVisited)) {
                bFoundPath = true;
                break;
            }
        }

        if (!bFoundPath) {
            bFoundAllChildPaths = false;
            break;
        }
    }

    return bFoundAllChildPaths;
}

namespace {
    template <typename T>
    TArray<T*> GetNodesOfType(const TArray<UGrammarScriptGraphNode*>& InNodes) {
        TArray<T*> Result;
        for (UGrammarScriptGraphNode* Node : InNodes) {
            if (T* CastNode = Cast<T>(Node)) {
                Result.Add(CastNode);
            }
        }
        return Result;
    }
}

TArray<UGrammarRuleScriptGraphNode*> FGraphPatternMatcher::GetIncomingNodes(UGrammarRuleScriptGraphNode* InNode) {
    return GetNodesOfType<UGrammarRuleScriptGraphNode>(InNode->IncomingNodes);
}

TArray<UGrammarRuleScriptGraphNode*> FGraphPatternMatcher::GetOutgoingNodes(UGrammarRuleScriptGraphNode* InNode) {
    return GetNodesOfType<UGrammarRuleScriptGraphNode>(InNode->OutgoingNodes);
}

bool FGraphPatternMatcher::MatchRecursive(UGrammarRuleScriptGraphNode* InDataNode,
                                          UGrammarRuleScriptGraphNode* InPatternNode,
                                          const TSet<UGrammarScriptGraphNode*>& InUnmatchedNodes,
                                          FGraphPatternMatch& OutMatch, TSet<UGrammarScriptGraphNode*>& OutVisited) {
    if (!InDataNode || !InPatternNode) return false;
    if (!IsNodeDataEqual(InDataNode, InPatternNode)) return false;
    if (!InUnmatchedNodes.Contains(InDataNode)) return false;


    TSet<UGrammarScriptGraphNode*> SubVisited = OutVisited;
    SubVisited.Add(InDataNode);
    SubVisited.Add(InPatternNode);

    FGraphPatternMatch SubMatch = OutMatch;
    SubMatch.PatternToDataNode.Add(InPatternNode, InDataNode);

    bool bSuccess = _TraverseChildren(GetIncomingNodes(InDataNode), GetIncomingNodes(InPatternNode), InUnmatchedNodes,
                                      SubMatch, SubVisited)
        && _TraverseChildren(GetOutgoingNodes(InDataNode), GetOutgoingNodes(InPatternNode), InUnmatchedNodes, SubMatch,
                             SubVisited);

    if (bSuccess) {
        OutVisited = SubVisited;
        OutMatch = SubMatch;
    }

    return bSuccess;
}

