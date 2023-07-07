//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/SnapMapEditor/SnapMapEditorUtils.h"

#include "Builders/SnapMap/SnapMapAsset.h"
#include "Frameworks/GraphGrammar/ExecutionGraph/EdGraph_FlowExec.h"
#include "Frameworks/GraphGrammar/ExecutionGraph/Nodes/EdGraphNode_ExecEntryNode.h"
#include "Frameworks/GraphGrammar/ExecutionGraph/Nodes/EdGraphNode_ExecRuleNode.h"
#include "Frameworks/GraphGrammar/GrammarScriptCompiler.h"
#include "Frameworks/GraphGrammar/GraphGrammar.h"
#include "Frameworks/GraphGrammar/RuleGraph/Debugging/EdGraphNode_DebugGrammarNode.h"
#include "Frameworks/GraphGrammar/RuleGraph/Debugging/EdGraph_DebugGrammar.h"
#include "Frameworks/GraphGrammar/RuleGraph/EdGraph_Grammar.h"
#include "Frameworks/GraphGrammar/RuleGraph/Nodes/EdGraphNode_GrammarNode.h"
#include "Frameworks/GraphGrammar/Script/GrammarExecutionScript.h"
#include "Frameworks/GraphGrammar/Script/GrammarRuleScript.h"

#include "EdGraph/EdGraphPin.h"

#define LOCTEXT_NAMESPACE "SnapMapEditorUtils"

DEFINE_LOG_CATEGORY_STATIC(LogSnapMapEditorUtils, Log, All);

/////////////////////////////// FSnapMapEditorUtils /////////////////////////////// 

void FSnapMapEditorUtils::InitializeFlowAsset(USnapMapAsset* InAsset) {
    InAsset->MissionGrammar = NewObject<UGraphGrammar>(InAsset, "MissionGrammar");
    InitializeMissionGrammar(InAsset->MissionGrammar);
}

void FSnapMapEditorUtils::InitializeMissionGrammar(UGraphGrammar* InGrammar) {
    // Register a start node and a task node type
    UGrammarNodeType* StartNodeType = AddNodeType(InGrammar);
    StartNodeType->TypeName = "S";
    StartNodeType->Description = LOCTEXT("StartNodeDescriptoin", "Start Node");

    UGrammarNodeType* TaskNodeType = AddNodeType(InGrammar);


    // Add a default start rule
    UGraphGrammarProduction* DefaultRule = AddNewRule(InGrammar);
    DefaultRule->RuleName = LOCTEXT("DefaultStartRuleName", "Start Rule");


    // Map the default start rule from the start node to the task node
    UEdGraph_Grammar* LHS = GetEdGraph(DefaultRule->SourceGraph);
    if (LHS) {
        LHS->AddNewNode(StartNodeType);
    }

    // Add a default mapping from the start node
    UEdGraph_Grammar* RHS = DefaultRule->DestGraphs.Num() > 0 ? GetEdGraph(DefaultRule->DestGraphs[0]) : nullptr;
    if (RHS) {
        RHS->AddNewNode(TaskNodeType);
    }

    InGrammar->ExecutionGraphScript = NewObject<UGrammarExecutionScript>(InGrammar, "ExecutionGraph");
    InitializeExecutionScript(InGrammar->ExecutionGraphScript, DefaultRule);

    CompileGrammarToScript(InGrammar);
}

UGraphGrammarProduction* FSnapMapEditorUtils::AddNewRule(UGraphGrammar* InGrammar) {
    UGraphGrammarProduction* Rule = NewObject<UGraphGrammarProduction>(InGrammar);
    InitializeProductionRule(Rule);
    InGrammar->ProductionRules.Add(Rule);
    InGrammar->Modify();
    return Rule;
}

UGrammarNodeType* FSnapMapEditorUtils::AddNodeType(UGraphGrammar* InGrammar) {
    UGrammarNodeType* NodeType = NewObject<UGrammarNodeType>(InGrammar);
    InGrammar->NodeTypes.Add(NodeType);
    InGrammar->Modify();
    return NodeType;
}

void FSnapMapEditorUtils::InitializeProductionRule(UGraphGrammarProduction* InProduction) {
    InProduction->SourceGraph = NewObject<UGrammarRuleScript>(InProduction, "LHS");
    InitializeGraphScript(InProduction->SourceGraph);

    InProduction->DestGraphs.Reset();
    AddProductionRHSGraph(InProduction);
}

UGrammarRuleScript* FSnapMapEditorUtils::AddProductionRHSGraph(UGraphGrammarProduction* InProduction) {
    UGrammarRuleScript* RHS = NewObject<UGrammarRuleScript>(InProduction);
    InitializeGraphScript(RHS);

    InProduction->DestGraphs.Add(RHS);
    return RHS;
}

namespace {
    void DebugLogScriptCompileState(UGrammarScriptGraph* ScriptGraph) {
        UE_LOG(LogSnapMapEditorUtils, Log, TEXT("Nodes [%d]"), ScriptGraph->Nodes.Num());
    }

    void DebugLogScriptCompileState(UGraphGrammar* InGrammar) {
        UE_LOG(LogSnapMapEditorUtils, Log, TEXT("Compiler State"));

        UE_LOG(LogSnapMapEditorUtils, Log, TEXT("Execution Script"));
        DebugLogScriptCompileState(InGrammar->ExecutionGraphScript->ScriptGraph);

        for (UGraphGrammarProduction* Production : InGrammar->ProductionRules) {
            UE_LOG(LogSnapMapEditorUtils, Log, TEXT("Production Rule Script: %s"),
                   *Production->RuleName.ToString());

            UE_LOG(LogSnapMapEditorUtils, Log, TEXT("Source"));
            DebugLogScriptCompileState(Production->SourceGraph->ScriptGraph);

            for (UGrammarRuleScript* DestGraph : Production->DestGraphs) {
                UE_LOG(LogSnapMapEditorUtils, Log, TEXT("Dest"));
                DebugLogScriptCompileState(DestGraph->ScriptGraph);
            }

            UE_LOG(LogSnapMapEditorUtils, Log, TEXT("=================="));
        }
    }
}

void FSnapMapEditorUtils::CompileGrammarToScript(UGraphGrammar* InGrammar) {
    if (!InGrammar) return;
    if (InGrammar->ExecutionGraphScript && InGrammar->ExecutionGraphScript->EdGraph) {
        FGrammarExecutionScriptCompiler::Compile(InGrammar->ExecutionGraphScript);
    }

    typedef TGrammarScriptCompiler<UEdGraphNode_GrammarNode, UGrammarRuleScriptGraphNode, UGrammarRuleScript>
        FGrammarRuleScriptCompiler;

    for (UGraphGrammarProduction* ProductionRule : InGrammar->ProductionRules) {
        if (!ProductionRule) continue;
        if (ProductionRule->SourceGraph) {
            FGrammarRuleScriptCompiler::Compile(ProductionRule->SourceGraph);
        }

        for (UGrammarRuleScript* DestGraph : ProductionRule->DestGraphs) {
            if (DestGraph) {
                FGrammarRuleScriptCompiler::Compile(DestGraph);
            }
        }
    }
    InGrammar->Modify();

    //DebugLogScriptCompileState(InGrammar);
}

template <typename TEdNodeType>
void FSnapMapEditorUtils::BuildEdGraphFromScriptImpl(UGrammarScriptGraph* ScriptGraph, UEdGraph_Grammar* EdGraph) {
    // Remove all existing nodes
    {
        TArray<class UEdGraphNode*> OldNodes = EdGraph->Nodes;
        for (UEdGraphNode* Node : OldNodes) {
            EdGraph->RemoveNode(Node);
        }
    }

    TMap<UGrammarScriptGraphNode*, UEdGraphNode_GrammarNode*> ScriptToEdNodeMap;

    // Create Ed Nodes and map them for linking later
    for (UGrammarScriptGraphNode* ScriptNode : ScriptGraph->Nodes) {
        if (UGrammarRuleScriptGraphNode* RuleNode = Cast<UGrammarRuleScriptGraphNode>(ScriptNode)) {
            if (RuleNode->TypeInfo.IsValid()) {
                UEdGraphNode_GrammarNode* EdNode = EdGraph->AddNewNodeOfType<TEdNodeType>(RuleNode->TypeInfo.Get());
                EdNode->NodeId = ScriptNode->NodeId;
                EdNode->bDisplayIndex = false;
                ScriptToEdNodeMap.Add(ScriptNode, EdNode);
            }
        }
    }

    // Make the links
    for (auto& Entry : ScriptToEdNodeMap) {
        UGrammarScriptGraphNode* ScriptNode = Entry.Key;
        UEdGraphNode_GrammarNode* EdNode = Entry.Value;

        // Link all outgoing nodes
        for (UGrammarScriptGraphNode* OutgoingNode : ScriptNode->OutgoingNodes) {
            UEdGraphNode_GrammarNode** SearchResult = ScriptToEdNodeMap.Find(OutgoingNode);
            if (SearchResult) {
                UEdGraphNode_GrammarNode* OutgoingEdNode = *SearchResult;
                EdNode->GetOutputPin()->MakeLinkTo(OutgoingEdNode->GetInputPin());
            }
        }
    }
}

void FSnapMapEditorUtils::BuildEdGraphFromScript(UGrammarScriptGraph* ScriptGraph, UEdGraph_Grammar* EdGraph) {
    BuildEdGraphFromScriptImpl<UEdGraphNode_GrammarNode>(ScriptGraph, EdGraph);
}

void FSnapMapEditorUtils::BuildDebugEdGraphFromScript(UGrammarScriptGraph* ScriptGraph,
                                                          UEdGraph_DebugGrammar* EdGraph) {
    EdGraph->Rewind();
    BuildEdGraphFromScriptImpl<UEdGraphNode_DebugGrammarNode>(ScriptGraph, EdGraph);
    EdGraph->OnDebugGraphGenerated();
}

UEdGraph_Grammar* FSnapMapEditorUtils::GetEdGraph(UGrammarRuleScript* RuleScript) {
    return Cast<UEdGraph_Grammar>(RuleScript->EdGraph);
}

UEdGraph_FlowExec* FSnapMapEditorUtils::GetEdGraph(UGrammarExecutionScript* ExecScript) {
    return Cast<UEdGraph_FlowExec>(ExecScript->EdGraph);
}

void FSnapMapEditorUtils::InitializeGraphScript(UGrammarRuleScript* InGraphScript) {
    InGraphScript->EdGraph = NewObject<UEdGraph_Grammar>(InGraphScript, "EdGraph");
}

void FSnapMapEditorUtils::InitializeExecutionScript(UGrammarExecutionScript* InExecutionScript,
                                                        TWeakObjectPtr<UGraphGrammarProduction> InStartRule) {
    UEdGraph_FlowExec* ExecGraph = NewObject<UEdGraph_FlowExec>(InExecutionScript, "EdGraph");
    InExecutionScript->EdGraph = ExecGraph;

    UEdGraphNode_ExecRuleNode* StartExecNode = NewObject<UEdGraphNode_ExecRuleNode>(ExecGraph);
    StartExecNode->InitializeNode_Runtime();
    StartExecNode->Rule = InStartRule;
    StartExecNode->NodePosX = 140;
    StartExecNode->NodePosY = -20;
    ExecGraph->AddNode(StartExecNode);

    ExecGraph->EntryNode->GetOutputPin()->MakeLinkTo(StartExecNode->GetInputPin());
}

#undef LOCTEXT_NAMESPACE

