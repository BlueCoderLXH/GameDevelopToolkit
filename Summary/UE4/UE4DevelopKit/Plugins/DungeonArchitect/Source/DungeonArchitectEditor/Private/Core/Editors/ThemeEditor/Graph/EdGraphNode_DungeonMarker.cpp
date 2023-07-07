//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonMarker.h"

#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonMarkerEmitter.h"
#include "Core/Editors/ThemeEditor/Graph/EdGraph_DungeonProp.h"

#define LOCTEXT_NAMESPACE "EdGraphNode_DungeonMesh"

UEdGraphNode_DungeonMarker::UEdGraphNode_DungeonMarker(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
    MarkerName = FString("Marker");
    bUserDefined = true;
    bBuilderEmittedMarker = false;
}

void UEdGraphNode_DungeonMarker::AllocateDefaultPins() {
    // Create the output pin
    CreatePin(EGPD_Output, FDungeonDataTypes::PinType_Mesh, TEXT("Out"));
}

FLinearColor UEdGraphNode_DungeonMarker::GetNodeTitleColor() const {
    return FLinearColor(0.7f, 0.7f, 0.7f);
}

FText UEdGraphNode_DungeonMarker::GetTooltipText() const {
    return LOCTEXT("AnimSlotNode_Tooltip", "Plays animation from code using AnimMontage");
}

FText UEdGraphNode_DungeonMarker::GetNodeTitle(ENodeTitleType::Type TitleType) const {
    return FText::FromString(MarkerName);
}


void UEdGraphNode_DungeonMarker::DestroyNode() {
    // Remove this node's references from marker emitter nodes that point to this
    TArray<UEdGraphNode_DungeonMarkerEmitter*> EmitterNodes;
    GetGraph()->GetNodesOfClass<UEdGraphNode_DungeonMarkerEmitter>(EmitterNodes);
    for (UEdGraphNode_DungeonMarkerEmitter* EmitterNode : EmitterNodes) {
        if (EmitterNode->ParentMarker == this) {
            EmitterNode->ParentMarker = nullptr;
        }
    }

    UEdGraphNode_DungeonBase::DestroyNode();
}

void UEdGraphNode_DungeonMarker::PostEditChangeProperty(struct FPropertyChangedEvent& e) {
    UEdGraphNode_DungeonBase::PostEditChangeProperty(e);
    if (e.Property && e.Property->GetFName() == "MarkerName") {
        GetGraph()->NotifyGraphChanged();
    }
}

bool UEdGraphNode_DungeonMarker::CanEditChange(const FProperty* InProperty) const {
    if (InProperty->GetFName() == "MarkerName" && !bUserDefined) {
        return false;
    }

    return UEdGraphNode::CanEditChange(InProperty);
}

void UEdGraphNode_DungeonMarker::AutowireNewNode(UEdGraphPin* FromPin) {
    if (!FromPin) {
        return;
    }

    UEdGraphPin* OutputPin = GetOutputPin();
    UEdGraphPin* InputPin = FromPin;

    if (FromPin->PinType.PinCategory == FDungeonDataTypes::PinType_Marker) {
        // Make sure we have no loops with this connection
        const UEdGraphSchema* Schema = GetGraph()->GetSchema();
        const FPinConnectionResponse ConnectionValid = Schema->CanCreateConnection(OutputPin, InputPin);
        if (ConnectionValid.Response == CONNECT_RESPONSE_MAKE) {
            OutputPin->MakeLinkTo(InputPin);
        }
    }
}

#undef LOCTEXT_NAMESPACE

