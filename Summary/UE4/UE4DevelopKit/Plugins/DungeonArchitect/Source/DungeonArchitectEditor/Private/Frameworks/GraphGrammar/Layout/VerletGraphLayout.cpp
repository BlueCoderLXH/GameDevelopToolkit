//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/GraphGrammar/Layout/VerletGraphLayout.h"

#include "Frameworks/GraphGrammar/RuleGraph/EdGraph_Grammar.h"
#include "Frameworks/GraphGrammar/RuleGraph/Nodes/EdGraphNode_GrammarNode.h"

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"

struct FLayoutVerletNode {
    bool bPinned = false;
    TWeakObjectPtr<UEdGraphNode> Node;
    FVector2D Location;
    FVector2D PreviousLocation;
};

typedef TSharedPtr<FLayoutVerletNode> FLayoutVerletNodePtr;

struct FLayoutVerletLink {
    FLayoutVerletNodePtr NodeA;
    FLayoutVerletNodePtr NodeB;
    float Strength = 1.0f;
};

typedef TSharedPtr<FLayoutVerletLink> FLayoutVerletLinkPtr;

void FVerletGraphLayout::PerformLayout(UEdGraph_Grammar* Graph) {
    StartTime = FDateTime::Now();

    // Create the verlet graph
    {
        // Create the verlet nodes
        TMap<UEdGraphNode*, FLayoutVerletNodePtr> NodeToVerletMap;
        for (UEdGraphNode* Node : Graph->Nodes) {
            UEdGraphNode_GrammarNode* TaskNode = Cast<UEdGraphNode_GrammarNode>(Node);
            if (TaskNode) {
                FLayoutVerletNodePtr VerletNode = MakeShareable(new FLayoutVerletNode);
                VerletNode->Node = Node;
                VerletNode->Location = FVector2D(Node->NodePosX, Node->NodePosY);
                VerletNode->PreviousLocation = VerletNode->Location;
                VerletNodes.Add(VerletNode);

                NodeToVerletMap.Add(Node, VerletNode);
            }
        }
        if (VerletNodes.Num() > 0) {
            VerletNodes[0]->bPinned = true;
            VerletNodes[0]->Location = FVector2D::ZeroVector;
            VerletNodes[0]->PreviousLocation = FVector2D::ZeroVector;
        }

        // Create the link constraints
        for (UEdGraphNode* Node : Graph->Nodes) {
            UEdGraphNode_GrammarNode* TaskNode = Cast<UEdGraphNode_GrammarNode>(Node);
            if (TaskNode) {
                for (UEdGraphPin* OutgoingPin : TaskNode->GetOutputPin()->LinkedTo) {
                    UEdGraphNode_GrammarNode* OutgoingTaskNode = Cast<UEdGraphNode_GrammarNode>(
                        OutgoingPin->GetOwningNode());
                    if (OutgoingTaskNode) {
                        FLayoutVerletLinkPtr Constraint = MakeShareable(new FLayoutVerletLink);
                        Constraint->NodeA = NodeToVerletMap[Node];
                        Constraint->NodeB = NodeToVerletMap[OutgoingTaskNode];
                        Constraint->Strength = Config.LinkStrength;
                        VerletLinks.Add(Constraint);
                    }
                }
            }
        }
    }

    // Perform verlet integration
    for (int i = 0; i < Config.NumIterations * Config.NumConstraintIterations; i++) {
        SolveVerlet();
        if ((FDateTime::Now() - StartTime).GetSeconds() > 0.1f) {
            break;
        }
    }

    // Update the position of the nodes
    for (FLayoutVerletNodePtr VerletNode : VerletNodes) {
        VerletNode->Node->NodePosX = FMath::RoundToInt(VerletNode->Location.X);
        VerletNode->Node->NodePosY = FMath::RoundToInt(VerletNode->Location.Y);
    }

    Graph->NotifyGraphChanged();
}

void FVerletGraphLayout::SolveVerletConstraint(TSharedPtr<FLayoutVerletNode> A, TSharedPtr<FLayoutVerletNode> B,
                                               float RestingDistance, float Strength) {
    FVector2D Direction = A->Location - B->Location;
    float Distance = Direction.Size();
    if (Distance < 1e-3f) {
        Distance = FMath::Max(Distance, 1.0f);
        Direction = FMath::RandPointInCircle(1.0f);
    }

    float Diff = (Config.LinkRestingDistance - Distance) / Distance;
    FVector2D Offset = Direction * 0.5f * Diff * Strength;

    if (!A->bPinned) A->Location += Offset;
    if (!B->bPinned) B->Location -= Offset;
}

void FVerletGraphLayout::SolveVerlet() {
    // Solve the link constraints
    for (FLayoutVerletLinkPtr Link : VerletLinks) {
        SolveVerletConstraint(Link->NodeA, Link->NodeB, Config.LinkRestingDistance, Link->Strength);
    }

    // Solve node collision constraint
    {
        float MinDist = Config.NodeRadius * 2;
        float MinDistSquared = FMath::Square(MinDist);
        for (int i = 0; i < VerletNodes.Num(); i++) {
            for (int j = i + 1; j < VerletNodes.Num(); j++) {
                FLayoutVerletNodePtr NodeA = VerletNodes[i];
                FLayoutVerletNodePtr NodeB = VerletNodes[j];
                FVector2D Direction = NodeA->Location - NodeB->Location;
                float DistanceSq = Direction.SizeSquared();
                if (DistanceSq < MinDistSquared) {
                    // Nodes are too close. Push them away
                    float Distance = FMath::Sqrt(DistanceSq);
                    if (Distance < 1e-3f) {
                        Distance = 1.0f;
                        Direction = FMath::RandPointInCircle(1.0f);
                    }

                    float Diff = (MinDist - Distance) / Distance;
                    FVector2D Offset = Direction * 0.5f * Diff;
                    if (!NodeA->bPinned) NodeA->Location += Offset;
                    if (!NodeB->bPinned) NodeB->Location -= Offset;
                }
            }
        }
    }

    // Verlet integration
    {
        for (FLayoutVerletNodePtr Node : VerletNodes) {
            FVector2D Velocity = Node->Location - Node->PreviousLocation;
            Node->PreviousLocation = Node->Location;

            if (!Node->bPinned) Node->Location += Velocity + Config.Gravity;
        }
    }
}

