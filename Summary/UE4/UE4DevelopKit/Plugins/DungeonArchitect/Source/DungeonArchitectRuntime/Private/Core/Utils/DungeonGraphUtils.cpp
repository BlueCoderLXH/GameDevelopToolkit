//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Utils/DungeonGraphUtils.h"

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphPin.h"

UEdGraphNode* FDungeonSchemaAction_NewNode::PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin,
                                                          const FVector2D Location, bool bSelectNewNode/* = true*/) {
    UEdGraphNode* ResultNode = nullptr;

#if WITH_EDITOR
    static const int32 SNAP_GRID = 16;
    const int32 NodeDistance = 60;

    // If there is a template, we actually use it
    if (NodeTemplate != nullptr) {
        NodeTemplate->SetFlags(RF_Transactional);

        // set outer to be the graph so it doesn't go away
        NodeTemplate->Rename(nullptr, ParentGraph, REN_NonTransactional);
        ParentGraph->AddNode(NodeTemplate, true, bSelectNewNode);

        NodeTemplate->CreateNewGuid();
        NodeTemplate->PostPlacedNewNode();
        NodeTemplate->AllocateDefaultPins();
        NodeTemplate->AutowireNewNode(FromPin);


        // For input pins, new node will generally overlap node being dragged off
        // Work out if we want to visually push away from connected node
        int32 XLocation = Location.X;
        if (FromPin && FromPin->Direction == EGPD_Input) {
            UEdGraphNode* PinNode = FromPin->GetOwningNode();
            const float XDelta = FMath::Abs(PinNode->NodePosX - Location.X);

            if (XDelta < NodeDistance) {
                // Set location to edge of current node minus the max move distance
                // to force node to push off from connect node enough to give selection handle
                XLocation = PinNode->NodePosX - NodeDistance;
            }
        }

        NodeTemplate->NodePosX = XLocation;
        NodeTemplate->NodePosY = Location.Y;
        NodeTemplate->SnapToGrid(SNAP_GRID);

        ResultNode = NodeTemplate;
    }

#endif // WITH_EDITOR
    return ResultNode;
}

UEdGraphNode* FDungeonSchemaAction_NewNode::PerformAction(class UEdGraph* ParentGraph, TArray<UEdGraphPin*>& FromPins,
                                                          const FVector2D Location, bool bSelectNewNode/* = true*/) {
    UEdGraphNode* ResultNode = nullptr;

#if WITH_EDITOR

    if (FromPins.Num() > 0) {
        ResultNode = PerformAction(ParentGraph, FromPins[0], Location, bSelectNewNode);

        // Try autowiring the rest of the pins
        for (int32 Index = 1; Index < FromPins.Num(); ++Index) {
            ResultNode->AutowireNewNode(FromPins[Index]);
        }
    }
    else {
        ResultNode = PerformAction(ParentGraph, nullptr, Location, bSelectNewNode);
    }

#endif // WITH_EDITOR
    return ResultNode;
}

void FDungeonSchemaAction_NewNode::AddReferencedObjects(FReferenceCollector& Collector) {
    FEdGraphSchemaAction::AddReferencedObjects(Collector);

    // These don't get saved to disk, but we want to make sure the objects don't get GC'd while the action array is around
    Collector.AddReferencedObject(NodeTemplate);
}

TSharedPtr<FDungeonSchemaAction_NewNode> FDungeonSchemaUtils::AddNewNodeAction(
    TArray<TSharedPtr<FEdGraphSchemaAction>>& OutActions, const FText& Category, const FText& MenuDesc,
    const FText& Tooltip, int32 Priority) {
    TSharedPtr<FDungeonSchemaAction_NewNode> NewAction = MakeShared<FDungeonSchemaAction_NewNode>(
        Category, MenuDesc, Tooltip, 0);

    OutActions.Add(NewAction);
    return NewAction;
}

