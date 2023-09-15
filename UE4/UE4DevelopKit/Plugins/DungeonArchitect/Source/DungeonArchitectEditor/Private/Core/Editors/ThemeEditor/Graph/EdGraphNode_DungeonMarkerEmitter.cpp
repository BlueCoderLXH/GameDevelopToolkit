//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonMarkerEmitter.h"

#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonMarker.h"
#include "Core/Editors/ThemeEditor/Graph/EdGraph_DungeonProp.h"

#define LOCTEXT_NAMESPACE "EdGraphNode_DungeonMesh"

UEdGraphNode_DungeonMarkerEmitter::UEdGraphNode_DungeonMarkerEmitter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
}

void UEdGraphNode_DungeonMarkerEmitter::AllocateDefaultPins() {
    // Create the input pin
    CreatePin(EGPD_Input, FDungeonDataTypes::PinType_Marker, TEXT("In"));
}

FLinearColor UEdGraphNode_DungeonMarkerEmitter::GetNodeTitleColor() const {
    return FLinearColor(0.7f, 0.7f, 0.7f);
}

FText UEdGraphNode_DungeonMarkerEmitter::GetTooltipText() const {
    return LOCTEXT("AnimSlotNode_Tooltip", "Plays animation from code using AnimMontage");
}

FText UEdGraphNode_DungeonMarkerEmitter::GetNodeTitle(ENodeTitleType::Type TitleType) const {
    FString MarkerName = ParentMarker ? ParentMarker->MarkerName : "[INVALID]";
    return FText::FromString(MarkerName);
}


void UEdGraphNode_DungeonMarkerEmitter::AutowireNewNode(UEdGraphPin* FromPin) {
    if (!FromPin) {
        return;
    }

    UEdGraphPin* OutputPin = FromPin;
    UEdGraphPin* InputPin = GetInputPin();

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

