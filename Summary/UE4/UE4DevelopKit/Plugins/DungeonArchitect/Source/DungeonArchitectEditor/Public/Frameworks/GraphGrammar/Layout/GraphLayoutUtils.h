//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/GraphGrammar/RuleGraph/EdGraph_Grammar.h"
#include "Frameworks/GraphGrammar/RuleGraph/Nodes/EdGraphNode_GrammarNode.h"

namespace GraphLayoutUtils {
    template <typename TLayoutNode>
    struct TLayoutGraphGraph {
        TSharedPtr<TLayoutNode> StartNode;
        TArray<TSharedPtr<TLayoutNode>> Nodes;
    };

    template <typename TLayoutNode>
    TLayoutGraphGraph<TLayoutNode> GenerateLinearGraph(UEdGraph_Grammar* Graph) {
        TSharedPtr<TLayoutNode> StartNode;
        // Build the node graph in memory
        TMap<TWeakObjectPtr<UEdGraphNode_GrammarNode>, TSharedPtr<TLayoutNode>> NodeMap;
        for (UEdGraphNode* Node : Graph->Nodes) {
            UEdGraphNode_GrammarNode* GrammarNode = Cast<UEdGraphNode_GrammarNode>(Node);
            if (GrammarNode) {
                TSharedPtr<TLayoutNode> NodePtr = MakeShareable(new TLayoutNode);
                NodePtr->Node = GrammarNode;
                NodeMap.Add(GrammarNode, NodePtr);

                if (!StartNode.IsValid()) {
                    StartNode = NodePtr;
                }
            }
        }

        // Link the nodes
        for (auto& Entry : NodeMap) {
            TWeakObjectPtr<UEdGraphNode_GrammarNode> GrammarNode = Entry.Key;
            TSharedPtr<TLayoutNode> Node = Entry.Value;
            for (UEdGraphPin* OutgoingPin : GrammarNode->GetOutputPin()->LinkedTo) {
                UEdGraphNode_GrammarNode* OutgoingGrammarNode = Cast<UEdGraphNode_GrammarNode>(
                    OutgoingPin->GetOwningNode());
                if (OutgoingGrammarNode && NodeMap.Contains(OutgoingGrammarNode)) {
                    TSharedPtr<TLayoutNode> OutgoingNode = NodeMap[OutgoingGrammarNode];
                    Node->OutgoingNodes.Add(OutgoingNode);
                    OutgoingNode->IncomingNodes.Add(Node);
                }
            }
        }


        // Find the start node
        if (StartNode.IsValid()) {
            while (true) {
                TSharedPtr<TLayoutNode> Parent;
                if (StartNode->IncomingNodes.Num() > 0) {
                    Parent = StartNode->IncomingNodes[0];
                }

                if (!Parent.IsValid()) {
                    break;
                }
                StartNode = Parent;
            }
        }

        TLayoutGraphGraph<TLayoutNode> Result;
        NodeMap.GenerateValueArray(Result.Nodes);
        Result.StartNode = StartNode;

        return Result;
    }
};

