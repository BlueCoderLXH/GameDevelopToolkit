//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonActorBase.h"

#include "Core/Editors/ThemeEditor/Graph/EdGraph_DungeonProp.h"

#include "EdGraph/EdGraphSchema.h"
#include "GraphEditAction.h"

#define LOCTEXT_NAMESPACE "EdGraphNode_DungeonActorBase"
DEFINE_LOG_CATEGORY(DungeonNodeLog);

UEdGraphNode_DungeonActorBase::UEdGraphNode_DungeonActorBase(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
    Probability = 1.0f;
    Affinity = 1.0f;
    ConsumeOnAttach = true;
    bLogicOverridesAffinity = true;
    bUseSpatialConstraint = false;
}

void UEdGraphNode_DungeonActorBase::AllocateDefaultPins() {
    UEdGraphPin* Inputs = CreatePin(EGPD_Input, FDungeonDataTypes::PinType_Mesh, TEXT("In"));
    UEdGraphPin* Outputs = CreatePin(EGPD_Output, FDungeonDataTypes::PinType_Marker, TEXT("Out"));
}

FLinearColor UEdGraphNode_DungeonActorBase::GetNodeTitleColor() const {
    return FLinearColor(0.7f, 0.7f, 0.7f);
}

FText UEdGraphNode_DungeonActorBase::GetTooltipText() const {
    return LOCTEXT("AnimSlotNode_Tooltip", "Plays animation from code using AnimMontage");
}

FText UEdGraphNode_DungeonActorBase::GetNodeTitle(ENodeTitleType::Type TitleType) const {
    return FText::FromString("Mesh");
}

void UEdGraphNode_DungeonActorBase::PostEditChangeProperty(struct FPropertyChangedEvent& e) {
    Super::PostEditChangeProperty(e);
    if (!e.Property) return;

    FString PropertyName;
    e.Property->GetName(PropertyName);

    UE_LOG(DungeonNodeLog, Log, TEXT("Property Changed: %s"), *PropertyName);

    // List of properties that do no require a complete node object cleanup from the scene
    TSet<FString> NonCleanupProperties;
    NonCleanupProperties.Add("Offset");
    NonCleanupProperties.Add("Affinity");
    NonCleanupProperties.Add("ConsumeOnAttach");
    NonCleanupProperties.Add("bUseSelectionLogic");
    NonCleanupProperties.Add("SelectionLogicClass");
    NonCleanupProperties.Add("bUseTransformLogic");
    NonCleanupProperties.Add("TransformLogicClass");

    bool bRequiresCleanup = !NonCleanupProperties.Contains(PropertyName);
    if (bRequiresCleanup) {
        UEdGraph_DungeonProp* ThemeGraph = Cast<UEdGraph_DungeonProp>(GetGraph());
        if (ThemeGraph) {
            FEdGraphEditAction Action;
            Action.Action = GRAPHACTION_Default;
            Action.bUserInvoked = true;
            Action.Graph = ThemeGraph;
            Action.Nodes.Add(this);
            ThemeGraph->NotifyNodePropertyChanged(Action);
        }
    }
}

void UEdGraphNode_DungeonActorBase::AutowireNewNode(UEdGraphPin* FromPin) {
    if (!FromPin) {
        return;
    }

    UEdGraphPin* OutputPin = nullptr;
    UEdGraphPin* InputPin = nullptr;

    if (FromPin->PinType.PinCategory == FDungeonDataTypes::PinType_Mesh) {
        OutputPin = FromPin;
        InputPin = GetInputPin();
    }
    else if (FromPin->PinType.PinCategory == FDungeonDataTypes::PinType_Marker) {
        OutputPin = GetOutputPin();
        InputPin = FromPin;
    }

    const UEdGraphSchema* Schema = GetGraph()->GetSchema();
    const FPinConnectionResponse ConnectionValid = Schema->CanCreateConnection(OutputPin, InputPin);
    if (ConnectionValid.Response == CONNECT_RESPONSE_MAKE) {
        OutputPin->MakeLinkTo(InputPin);
    }
}

void UEdGraphNode_DungeonActorBase::NodeConnectionListChanged() {
    UEdGraphNode_DungeonBase::NodeConnectionListChanged();

    if (GetInputPin()->LinkedTo.Num() == 0) {
        ExecutionOrder = 1;
    }
}


UObject* UEdGraphNode_DungeonActorBase::GetThumbnailAssetObject() {
    return GetNodeAssetObject(GetTransientPackage());
}


#undef LOCTEXT_NAMESPACE

