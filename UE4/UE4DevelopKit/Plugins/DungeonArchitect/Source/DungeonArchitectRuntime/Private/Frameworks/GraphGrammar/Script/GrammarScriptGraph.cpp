//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/GraphGrammar/Script/GrammarScriptGraph.h"


UGrammarScriptGraphNode::UGrammarScriptGraphNode() {
    NodeId = FGuid::NewGuid();
}

UGrammarScriptGraphNode* UGrammarScriptGraph::FindRootNode() const {
    if (Nodes.Num() == 0) {
        // No nodes exist
        return nullptr;
    }

    UGrammarScriptGraphNode* Node = Nodes[0];
    TSet<UGrammarScriptGraphNode*> Visited;
    while (true) {
        TArray<UGrammarScriptGraphNode*> IncomingNodes = Node->IncomingNodes;
        if (IncomingNodes.Num() == 0) {
            return Node;
        }
        Visited.Add(Node);

        UGrammarScriptGraphNode* ParentNode = nullptr;
        for (UGrammarScriptGraphNode* IncomingNode : IncomingNodes) {
            UGrammarScriptGraphNode* NextNode = Cast<UGrammarScriptGraphNode>(IncomingNode);
            if (NextNode && !Visited.Contains(NextNode)) {
                ParentNode = NextNode;
                break;
            }
        }

        if (!ParentNode) {
            return nullptr;
        }
        Node = ParentNode;
    }
}

UGrammarScriptGraph::UGrammarScriptGraph(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
}

