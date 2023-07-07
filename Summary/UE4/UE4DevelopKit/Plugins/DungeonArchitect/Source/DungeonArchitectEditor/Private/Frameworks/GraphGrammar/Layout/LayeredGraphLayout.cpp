//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/GraphGrammar/Layout/LayeredGraphLayout.h"

#include "Frameworks/GraphGrammar/Layout/GraphLayoutUtils.h"
#include "Frameworks/GraphGrammar/RuleGraph/EdGraph_Grammar.h"
#include "Frameworks/GraphGrammar/RuleGraph/Nodes/EdGraphNode_GrammarNode.h"

#include "EdGraph/EdGraphNode.h"

using namespace GraphLayoutUtils;

class FLayeredLayoutUtils {
public:
    typedef TMap<int, float> ContourMap_t;

    static void BuildTree(FLayeredLayoutNodePtr Node) {
        TSet<FLayeredLayoutNodePtr> Visited;
        BuildTree(Node, nullptr, Visited);
    }

    static void CalculateFinalX(FLayeredLayoutNodePtr Node, float TotalMod) {
        Node->X += TotalMod;

        for (FLayeredLayoutNodePtr Child : Node->Children) {
            CalculateFinalX(Child, TotalMod + Node->Mod);
        }
    }

    static void CalculateInitialX(FLayeredLayoutNodePtr Node, FLayeredLayoutNodePtr LeftSibling) {
        FLayeredLayoutNodePtr LeftChild;
        for (FLayeredLayoutNodePtr Child : Node->Children) {
            CalculateInitialX(Child, LeftChild);
            LeftChild = Child;
        }

        bool bIsLeftMost = !LeftSibling.IsValid();
        bool bIsLeaf = Node->Children.Num() == 0;

        if (bIsLeaf) {
            if (bIsLeftMost) {
                Node->X = 0;
            }
            else {
                Node->X = LeftSibling->X + 1;
            }
        }
        else if (Node->Children.Num() == 1) {
            if (bIsLeftMost) {
                Node->X = Node->Children[0]->X;
            }
            else {
                Node->X = LeftSibling->X + 1;
                Node->Mod = Node->X - Node->Children[0]->X;
            }
        }
        else {
            float LeftX = Node->Children[0]->X;
            float RightX = Node->Children.Last()->X;
            float MidX = (LeftX + RightX) / 2.0f;

            if (bIsLeftMost) {
                Node->X = MidX;
            }
            else {
                Node->X = LeftSibling->X + 1;
                Node->Mod = Node->X - MidX;
            }
        }

        if (!bIsLeaf && !bIsLeftMost) {
            ResolveConflicts(Node);
        }
    }

    static void ResolveConflicts(FLayeredLayoutNodePtr Node) {
        float ShiftValue = 0.0f;
        float MinDistance = 1.0f;

        ContourMap_t NodeContour;
        GetLeftContour(Node, 0, NodeContour);

        TArray<int32> NodeLevels;
        NodeContour.GenerateKeyArray(NodeLevels);
        NodeLevels.Sort();

        FLayeredLayoutNodePtr Sibling = GetLeftMostSibling(Node);

        while (Sibling.IsValid() && Sibling != Node) {
            ContourMap_t SiblingContour;
            GetRightContour(Sibling, 0, SiblingContour);

            TArray<int32> SiblingLevels;
            SiblingContour.GenerateKeyArray(SiblingLevels);
            SiblingLevels.Sort();

            int32 MaxNodeLevel = NodeLevels.Last();
            int32 MaxSiblingLevel = SiblingLevels.Last();

            int32 StartLevel = Node->Depth + 1;
            int32 EndLevel = FMath::Min(MaxNodeLevel, MaxSiblingLevel);
            for (int32 Level = StartLevel; Level <= EndLevel; Level++) {
                float Distance = NodeContour[Level] - SiblingContour[Level];
                if (Distance + ShiftValue < MinDistance) {
                    ShiftValue = MinDistance - Distance;
                }
            }

            if (ShiftValue > 0) {
                Node->X += ShiftValue;
                Node->Mod += ShiftValue;

                ShiftValue = 0;
            }

            Sibling = GetNextSibling(Sibling);
        }

    }

    static void TagNodeLevels(FLayeredLayoutNodePtr Node, int32 Depth) {
        Node->Depth = Depth;
        for (FLayeredLayoutNodePtr Child : Node->Children) {
            TagNodeLevels(Child, Depth + 1);
        }
    }

private:
    static float FindLargestX(const TArray<FLayeredLayoutNodePtr>& Nodes) {
        if (Nodes.Num() == 0) return 0;
        float Largest = -MIN_flt;
        for (FLayeredLayoutNodePtr Node : Nodes) {
            Largest = FMath::Max(Largest, Node->X);
        }
        return Largest;
    }

    static void BuildTree(FLayeredLayoutNodePtr Node, FLayeredLayoutNodePtr Parent,
                          TSet<FLayeredLayoutNodePtr>& OutVisited) {
        if (OutVisited.Contains(Node)) {
            return;
        }
        OutVisited.Add(Node);
        Node->Parent = Parent;

        for (FLayeredLayoutNodePtr Child : Node->OutgoingNodes) {
            if (!OutVisited.Contains(Child)) {
                Node->Children.Add(Child);
                BuildTree(Child, Node, OutVisited);
            }
        }
    }

    static void GetLeftContour(FLayeredLayoutNodePtr Node, float ModSum, ContourMap_t& ContourMap) {
        if (!ContourMap.Contains(Node->Depth)) {
            ContourMap.Add(Node->Depth, Node->X + ModSum);
        }
        else {
            ContourMap[Node->Depth] = FMath::Min(ContourMap[Node->Depth], Node->X + ModSum);
        }

        for (FLayeredLayoutNodePtr Child : Node->Children) {
            GetLeftContour(Child, ModSum + Node->Mod, ContourMap);
        }
    }

    static void GetRightContour(FLayeredLayoutNodePtr Node, float ModSum, ContourMap_t& ContourMap) {
        if (!ContourMap.Contains(Node->Depth)) {
            ContourMap.Add(Node->Depth, Node->X + ModSum);
        }
        else {
            ContourMap[Node->Depth] = FMath::Max(ContourMap[Node->Depth], Node->X + ModSum);
        }

        for (FLayeredLayoutNodePtr Child : Node->Children) {
            GetRightContour(Child, ModSum + Node->Mod, ContourMap);
        }
    }

    static FLayeredLayoutNodePtr GetLeftMostSibling(FLayeredLayoutNodePtr Node) {
        if (!Node.IsValid() || !Node->Parent.IsValid()) {
            return nullptr;
        }

        return Node->Parent->Children[0];
    }

    static FLayeredLayoutNodePtr GetNextSibling(FLayeredLayoutNodePtr Node) {
        if (!Node.IsValid() || !Node->Parent.IsValid()) {
            return nullptr;
        }

        int32 NodeIdx = Node->Parent->Children.Find(Node);
        if (NodeIdx == INDEX_NONE || NodeIdx == Node->Parent->Children.Num() - 1) {
            return nullptr;
        }

        return Node->Parent->Children[NodeIdx + 1];
    }

};

void FLayeredGraphLayout::PerformLayout(UEdGraph_Grammar* Graph) {
    TLayoutGraphGraph<FLayeredLayoutNode> LayeredGraph = GenerateLinearGraph<FLayeredLayoutNode>(Graph);

    if (!LayeredGraph.StartNode.IsValid()) {
        return;
    }

    FLayeredLayoutUtils::BuildTree(LayeredGraph.StartNode);

    FLayeredLayoutUtils::TagNodeLevels(LayeredGraph.StartNode, 0);

    FLayeredLayoutUtils::CalculateInitialX(LayeredGraph.StartNode, nullptr);

    FLayeredLayoutUtils::CalculateFinalX(LayeredGraph.StartNode, 0);

    // Finalize the arrangement
    for (FLayeredLayoutNodePtr LayeredNode : LayeredGraph.Nodes) {
        LayeredNode->Node->NodePosX = Config.DepthDistance * LayeredNode->Depth;
        LayeredNode->Node->NodePosY = Config.SiblingDistance * LayeredNode->X;
    }
}

