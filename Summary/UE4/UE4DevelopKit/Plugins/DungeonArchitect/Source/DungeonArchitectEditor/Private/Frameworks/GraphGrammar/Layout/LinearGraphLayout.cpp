//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/GraphGrammar/Layout/LinearGraphLayout.h"

#include "Frameworks/GraphGrammar/Layout/GraphLayoutUtils.h"
#include "Frameworks/GraphGrammar/RuleGraph/EdGraph_Grammar.h"
#include "Frameworks/GraphGrammar/RuleGraph/Nodes/EdGraphNode_GrammarNode.h"

#include "EdGraph/EdGraphNode.h"

using namespace GraphLayoutUtils;

namespace {

    void ArrangeHorizontal(FLinearLayoutNodePtr Node, int32 DistanceFromStart, TSet<FLinearLayoutNodePtr>& Visited) {
        Node->LocationX = FMath::Max(Node->LocationX, DistanceFromStart);

        // Go forward only if all the incoming nodes have been processed
        bool bAllIncomingNodesProcessed = true;
        for (FLinearLayoutNodePtr IncomingNode : Node->IncomingNodes) {
            if (!Visited.Contains(IncomingNode)) {
                bAllIncomingNodesProcessed = false;
                break;
            }
        }

        if (bAllIncomingNodesProcessed) {
            Visited.Add(Node);
            for (FLinearLayoutNodePtr OutgoingNode : Node->OutgoingNodes) {
                ArrangeHorizontal(OutgoingNode, DistanceFromStart + 1, Visited);
            }
        }
    }

    void ArrangeVertical(TArray<FLinearLayoutNodePtr> Nodes) {
        if (Nodes.Num() == 0) return;

        int32 MinX = MAX_int32;
        int32 MaxX = MIN_int32;
        TMap<int32, TArray<FLinearLayoutNodePtr>> DistanceFromStart;
        for (FLinearLayoutNodePtr Node : Nodes) {
            TArray<FLinearLayoutNodePtr>& List = DistanceFromStart.FindOrAdd(Node->LocationX);
            List.Add(Node);

            MinX = FMath::Min(MinX, Node->LocationX);
            MaxX = FMath::Max(MaxX, Node->LocationX);
        }

        for (int x = MinX; x <= MaxX; x++) {
            if (DistanceFromStart.Contains(x)) {
                int32 Y = 0;
                TArray<FLinearLayoutNodePtr>& NodesAtX = DistanceFromStart[x];
                for (FLinearLayoutNodePtr NodeAtX : NodesAtX) {
                    for (FLinearLayoutNodePtr OutgoingNode : NodeAtX->OutgoingNodes) {
                        if (OutgoingNode->LocationX == x + 1) {
                            OutgoingNode->LocationY = Y;
                            Y++;
                        }
                    }
                }
            }
        }
    }
}


void FLinearGraphLayout::PerformLayout(UEdGraph_Grammar* Graph) {
    TLayoutGraphGraph<FLinearLayoutNode> LinearGraph = GenerateLinearGraph<FLinearLayoutNode>(Graph);

    // Arrange the nodes horizontally
    {
        TSet<FLinearLayoutNodePtr> Visited;
        ArrangeHorizontal(LinearGraph.StartNode, 0, Visited);
    }

    // Arrange the nodes vertically
    ArrangeVertical(LinearGraph.Nodes);

    // Finalize the arrangement
    for (FLinearLayoutNodePtr Node : LinearGraph.Nodes) {
        Node->Node->NodePosX = Config.InterDistanceX * Node->LocationX;
        Node->Node->NodePosY = Config.InterDistanceY * Node->LocationY;
    }
}

