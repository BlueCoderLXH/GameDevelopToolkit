//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/GraphGrammar/Editor/GraphGrammarValidation.h"

#include "Core/Editors/SnapMapEditor/SnapMapEditorUtils.h"
#include "Frameworks/GraphGrammar/Editor/GraphGrammarValidationSupport.h"
#include "Frameworks/GraphGrammar/GraphGrammar.h"
#include "Frameworks/GraphGrammar/RuleGraph/EdGraph_Grammar.h"
#include "Frameworks/GraphGrammar/Script/GrammarRuleScript.h"

#define LOCTEXT_NAMESPACE "GrammarValidator"

FGrammarValidationResult FGrammarValidator::Validate(UGraphGrammar* Grammar) {
    FGrammarValidationResult Result;

    ValidateBasics(Grammar, Result);
    ValidateNodeTypes(Grammar, Result);
    ValidateGraphs(Grammar, Result);

    Result.Finalize();
    return Result;
}

void FGrammarValidator::ValidateBasics(UGraphGrammar* Grammar, FGrammarValidationResult& OutResult) {
    static const FName StartNodeLabel = "S";
    // Make sure we have a start node
    {
        TMap<FName, TArray<UGrammarNodeType*>> NameToNodeType;
        for (UGrammarNodeType* NodeType : Grammar->NodeTypes) {
            TArray<UGrammarNodeType*>& List = NameToNodeType.FindOrAdd(NodeType->TypeName);
            List.Add(NodeType);
        }

        if (!NameToNodeType.Contains(StartNodeLabel)) {
            const FText ErrorMessage = LOCTEXT("MissingStartNode",
                                               "Start node type is not defined.  Please create a node type with the name S");
            IGrammarFocusActionPtr FocusAction = MakeShareable(new FGrammarFocusAction_FlashNodeTypePanel);
            FGrammarValidationEntryPtr Entry = MakeShareable(
                new FGrammarValidationEntry(EGrammarLogType::Error, ErrorMessage, FocusAction));
            OutResult.Entries.Add(Entry);
        }

        if (Grammar->NodeTypes.Num() == 0) {
            const FText ErrorMessage = LOCTEXT("NoNodeTypesDefined", "No node types defined");
            IGrammarFocusActionPtr FocusAction = MakeShareable(new FGrammarFocusAction_FlashNodeTypePanel);
            FGrammarValidationEntryPtr Entry = MakeShareable(
                new FGrammarValidationEntry(EGrammarLogType::Error, ErrorMessage, FocusAction));
            OutResult.Entries.Add(Entry);
        }
    }

    // Make sure we have atleast one rule define
    {
        if (Grammar->ProductionRules.Num() == 0) {
            FText ErrorMessage = LOCTEXT("ErrorNoRules",
                                         "No Rules have been defined.  Please add atleast one with a start node S on the LHS");
            IGrammarFocusActionPtr FocusAction = MakeShareable(new FGrammarFocusAction_FlashRulePanel);
            FGrammarValidationEntryPtr Entry = MakeShareable(
                new FGrammarValidationEntry(EGrammarLogType::Error, ErrorMessage, FocusAction));
            OutResult.Entries.Add(Entry);
        }
    }
}

struct FGrammarNodeTypeKey {
    FGrammarNodeTypeKey(const FString& InKey) : Key(InKey) {
    }

    FString Key;

    bool operator==(const FGrammarNodeTypeKey& Other) {
        return Key.Equals(Other.Key, ESearchCase::CaseSensitive);
    }
};

void FGrammarValidator::ValidateNodeTypes(UGraphGrammar* Grammar, FGrammarValidationResult& OutResult) {
    // Make sure the node types do not have the same name
    {
        const FText EmptyNameErrorMessage = LOCTEXT("ErrorEmptyNodeType", "Empty node type name");

        TMap<FName, TArray<UGrammarNodeType*>> NameToNodeType;
        for (UGrammarNodeType* NodeType : Grammar->NodeTypes) {

            if (NodeType->TypeName.ToString().Len() > 0) {
                FName Key = NodeType->TypeName;
                TArray<UGrammarNodeType*>& List = NameToNodeType.FindOrAdd(Key);
                List.Add(NodeType);
            }
            else {
                // Empty name
                IGrammarFocusActionPtr FocusAction = MakeShareable(new FGrammarFocusAction_JumpToNodeType(NodeType));
                FGrammarValidationEntryPtr Entry = MakeShareable(
                    new FGrammarValidationEntry(EGrammarLogType::Error, EmptyNameErrorMessage, FocusAction));
                OutResult.Entries.Add(Entry);
            }
        }


        const FText DuplicateErrorMessagePattern = LOCTEXT("ErrorDuplicateNodeType",
                                                           "Duplicate node type [{NodeTypeName}]. More than one node has the same name");

        // Generate an error for duplicate entries
        for (auto& Entry : NameToNodeType) {
            const TArray<UGrammarNodeType*>& List = Entry.Value;
            if (List.Num() > 1) {
                for (UGrammarNodeType* DuplicateNodeType : List) {
                    IGrammarFocusActionPtr FocusAction = MakeShareable(
                        new FGrammarFocusAction_JumpToNodeType(DuplicateNodeType));

                    FFormatNamedArguments Args;
                    Args.Add(TEXT("NodeTypeName"), FText::FromName(DuplicateNodeType->TypeName));
                    FText ErrorMessage = FText::Format(DuplicateErrorMessagePattern, Args);
                    FGrammarValidationEntryPtr ValidationEntry = MakeShareable(
                        new FGrammarValidationEntry(EGrammarLogType::Error, ErrorMessage, FocusAction));
                    OutResult.Entries.Add(ValidationEntry);
                }
            }
        }
    }
}

void FGrammarValidator::ValidateGraphs(UGraphGrammar* Grammar, FGrammarValidationResult& OutResult) {
    TArray<UEdGraph_Grammar*> GraphsToCheck;
    for (UGraphGrammarProduction* Rule : Grammar->ProductionRules) {
        ValidateGraph(Rule, FSnapMapEditorUtils::GetEdGraph(Rule->SourceGraph), OutResult);
        if (Rule->DestGraphs.Num() > 0) {
            for (UGrammarRuleScript* RHS : Rule->DestGraphs) {
                ValidateGraph(Rule, FSnapMapEditorUtils::GetEdGraph(RHS), OutResult);
            }
        }
        else {
            // No RHS graph defined
            FFormatNamedArguments Args;
            Args.Add(TEXT("RuleName"), Rule->RuleName);
            FText ErrorMessage = FText::Format(
                LOCTEXT("ErrorNoRHSGraph", "No RHS Rule defined for rule [{RuleName}]"), Args);
            IGrammarFocusActionPtr FocusAction = MakeShareable(new FGrammarFocusAction_JumpToRule(Rule));
            FGrammarValidationEntryPtr Entry = MakeShareable(
                new FGrammarValidationEntry(EGrammarLogType::Warning, ErrorMessage, FocusAction));
            OutResult.Entries.Add(Entry);
        }
    }
}

void FGrammarValidator::ValidateGraph(UGraphGrammarProduction* Rule, UEdGraph_Grammar* Graph,
                                      FGrammarValidationResult& OutResult) {
    // Check if the graph is empty
    if (Graph->Nodes.Num() == 0) {
        FFormatNamedArguments Args;
        Args.Add(TEXT("RuleName"), Rule->RuleName);
        FText ErrorMessage = FText::Format(LOCTEXT("ErrorGraphEmpty", "Empty graph found for rule [{RuleName}]"), Args);

        IGrammarFocusActionPtr FocusAction = MakeShareable(new FGrammarFocusAction_JumpToGraph(Graph));
        FGrammarValidationEntryPtr Entry = MakeShareable(
            new FGrammarValidationEntry(EGrammarLogType::Warning, ErrorMessage, FocusAction));
        OutResult.Entries.Add(Entry);
        return;
    }

    // Check if the graph has nodes with duplicate indices
    {
        TMap<int32, TArray<UEdGraphNode_GrammarNode*>> NodesByIndex;
        for (UEdGraphNode* Node : Graph->Nodes) {
            UEdGraphNode_GrammarNode* GrammarNode = Cast<UEdGraphNode_GrammarNode>(Node);
            if (GrammarNode && GrammarNode->TypeInfo.IsValid()) {
                TArray<UEdGraphNode_GrammarNode*>& List = NodesByIndex.FindOrAdd(GrammarNode->Index);
                List.Add(GrammarNode);
            }
        }

        FText ErrorMessagePattern = LOCTEXT("ErrorNodeIndexDuplicate",
                                            "Duplicate node index [{NodeIndex}] found.  Please assign unique numbers.  Node: [{NodeName}]; Rule: [{RuleName}]");
        for (auto& Entry : NodesByIndex) {
            TArray<UEdGraphNode_GrammarNode*>& List = Entry.Value;
            if (List.Num() > 1) {
                // Found multiple nodes with the same id
                for (UEdGraphNode_GrammarNode* Node : List) {
                    FFormatNamedArguments Args;
                    Args.Add(TEXT("NodeIndex"), Node->Index);
                    Args.Add(TEXT("NodeName"), FText::FromName(Node->TypeInfo->TypeName));
                    Args.Add(TEXT("RuleName"), Rule->RuleName);
                    FText ErrorMessage = FText::Format(ErrorMessagePattern, Args);

                    IGrammarFocusActionPtr FocusAction = MakeShareable(new FGrammarFocusAction_JumpToNode(Node));
                    FGrammarValidationEntryPtr ValidationEntry = MakeShareable(
                        new FGrammarValidationEntry(EGrammarLogType::Error, ErrorMessage, FocusAction));
                    OutResult.Entries.Add(ValidationEntry);
                }
            }
        }
    }


    // Check for deleted node type info
    {
        FText ErrorMessage = LOCTEXT("ErrorDeletedNodeType",
                                     "The node's type information has been deleted.  Please recreate the node with an existing type");
        for (UEdGraphNode* Node : Graph->Nodes) {
            UEdGraphNode_GrammarNode* GrammarNode = Cast<UEdGraphNode_GrammarNode>(Node);
            if (GrammarNode && !GrammarNode->TypeInfo.IsValid()) {
                IGrammarFocusActionPtr FocusAction = MakeShareable(new FGrammarFocusAction_JumpToNode(Node));
                FGrammarValidationEntryPtr Entry = MakeShareable(
                    new FGrammarValidationEntry(EGrammarLogType::Error, ErrorMessage, FocusAction));
                OutResult.Entries.Add(Entry);
            }
        }
    }

    // Check if the graph is connected
    ValidateGraphConnectivity(Rule, Graph, OutResult);
}

void FGrammarValidator::ValidateGraphConnectivity(UGraphGrammarProduction* Rule, UEdGraph_Grammar* Graph,
                                                  FGrammarValidationResult& OutResult) {
    TArray<UEdGraphNode_GrammarNode*> GrammarNodes;
    for (UEdGraphNode* Node : Graph->Nodes) {
        UEdGraphNode_GrammarNode* GrammarNode = Cast<UEdGraphNode_GrammarNode>(Node);
        if (GrammarNode) {
            GrammarNodes.Add(GrammarNode);
        }
    }

    if (GrammarNodes.Num() < 2) {
        // Too few nodes for a disconnected graph
        return;
    }

    // DFS from the first node 
    TSet<UEdGraphNode_GrammarNode*> Visited;
    TArray<UEdGraphNode_GrammarNode*> Stack;

    Visited.Add(GrammarNodes[0]);
    Stack.Push(GrammarNodes[0]);

    while (Stack.Num() > 0) {
        UEdGraphNode_GrammarNode* Top = Stack.Pop();
        TArray<UEdGraphPin*> ConnectedPins;
        ConnectedPins.Append(Top->GetOutputPin()->LinkedTo);
        ConnectedPins.Append(Top->GetInputPin()->LinkedTo);
        for (UEdGraphPin* ConnectedPin : ConnectedPins) {
            UEdGraphNode_GrammarNode* ConnectedNode = Cast<UEdGraphNode_GrammarNode>(ConnectedPin->GetOwningNode());
            if (ConnectedNode && !Visited.Contains(ConnectedNode)) {
                Visited.Add(ConnectedNode);
                Stack.Push(ConnectedNode);
            }
        }
    }

    // If this is a connected graph, all the nodes should have been visited by now
    bool bConnectedGraph = true;
    for (UEdGraphNode_GrammarNode* GrammarNode : GrammarNodes) {
        // This node was not visited. This is not a fully connected graph
        if (!Visited.Contains(GrammarNode)) {
            bConnectedGraph = false;
            break;
        }
    }

    if (!bConnectedGraph) {
        FFormatNamedArguments Args;
        Args.Add("RuleName", Rule->RuleName);
        FText ErrorMessage = FText::Format(
            LOCTEXT("ErrorNotConnectedGraph",
                    "Connected graph required in rule [{RuleName}], i.e. all nodes must be connected to a single graph"),
            Args);

        IGrammarFocusActionPtr FocusAction = MakeShareable(new FGrammarFocusAction_JumpToGraph(Graph));
        FGrammarValidationEntryPtr Entry = MakeShareable(
            new FGrammarValidationEntry(EGrammarLogType::Error, ErrorMessage, FocusAction));
        OutResult.Entries.Add(Entry);
    }
}

#undef LOCTEXT_NAMESPACE

