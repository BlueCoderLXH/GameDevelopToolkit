//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/ThemeEditor/Graph/EdGraph_DungeonProp.h"

#include "Core/DungeonBuilder.h"
#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonActorTemplate.h"
#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonMarker.h"
#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonMarkerEmitter.h"
#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonMesh.h"
#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonParticleSystem.h"
#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonPointLight.h"
#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonSpotLight.h"
#include "Core/Editors/ThemeEditor/Graph/EdGraphSchema_DungeonProp.h"
#include "Frameworks/ThemeEngine/DungeonThemeAsset.h"
#include "Frameworks/ThemeEngine/Rules/DungeonSelectorLogic.h"
#include "Frameworks/ThemeEngine/Rules/DungeonTransformLogic.h"

#include "ActorFactories/ActorFactory.h"
#include "AssetSelection.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Kismet2/BlueprintEditorUtils.h"

const int32 UEdGraph_DungeonProp::SNAP_GRID = 16;

#define LOCTEXT_NAMESPACE "DungeonPropGraph"

const FName FDungeonDataTypes::PinType_Mesh = "mesh";
const FName FDungeonDataTypes::PinType_Marker = "marker";

UEdGraph_DungeonProp::UEdGraph_DungeonProp(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
    Schema = UEdGraphSchema_DungeonProp::StaticClass();
}

template <typename T>
void GetChildNodes(UEdGraphNode_DungeonBase* ParentNode, TArray<T*>& OutChildren) {
    for (UEdGraphPin* ChildPin : ParentNode->GetOutputPin()->LinkedTo) {
        if (ChildPin) {
            if (T* DesiredNode = Cast<T>(ChildPin->GetOwningNode())) {
                OutChildren.Add(DesiredNode);
            }
        }
    }
}

struct ExecutionSortComparer {
    bool operator()(const UEdGraphNode_DungeonActorBase& A, const UEdGraphNode_DungeonActorBase& B) const {
        return A.ExecutionOrder < B.ExecutionOrder;
    }
};

template <typename T>
void CloneUObjectArray(UObject* Outer, const TArray<T*>& SourceList, TArray<T*>& DestList) {
    DestList.Reset();
    for (T* Source : SourceList) {
        if (!Source) continue;
        T* Clone = NewObject<T>(Outer, Source->GetClass(), NAME_None, RF_NoFlags, Source);
        DestList.Add(Clone);
    }
}

void UEdGraph_DungeonProp::RebuildGraph(UObject* Owner, TArray<FPropTypeData>& OutProps, TArray<FDungeonGraphBuildError>& OutErrors) const {
    OutProps.Reset();
    // TODO: Check for cycles

    TArray<UEdGraphNode_DungeonMarker*> MarkerNodes;
    GetNodesOfClass<UEdGraphNode_DungeonMarker>(MarkerNodes);
    for (UEdGraphNode_DungeonMarker* MarkerNode : MarkerNodes) {
        TArray<UEdGraphNode_DungeonActorBase*> ActorNodes;
        GetChildNodes<UEdGraphNode_DungeonActorBase>(MarkerNode, ActorNodes);
        ActorNodes.Sort(ExecutionSortComparer());
        for (UEdGraphNode_DungeonActorBase* ActorNode : ActorNodes) {
            FPropTypeData Prop;
            Prop.NodeId = FName(*ActorNode->NodeGuid.ToString());
            Prop.AttachToSocket = MarkerNode->MarkerName;
            Prop.AssetObject = ActorNode->GetNodeAssetObject(Owner);
            Prop.Probability = ActorNode->Probability;
            Prop.Offset = ActorNode->Offset;
            Prop.bUseSpatialConstraint = ActorNode->bUseSpatialConstraint;
            Prop.ConsumeOnAttach = ActorNode->ConsumeOnAttach;

            if (ActorNode->SpatialConstraint) {
                UDungeonSpatialConstraint* Template = ActorNode->SpatialConstraint;
                Prop.SpatialConstraint = NewObject<UDungeonSpatialConstraint>(
                    Owner, Template->GetClass(), NAME_None, RF_NoFlags, Template);
            }
            else {
                Prop.SpatialConstraint = nullptr;
            }

            // Clone the selection logic instances
            CloneUObjectArray<UDungeonSelectorLogic>(Owner, ActorNode->SelectionLogics, Prop.SelectionLogics);
            Prop.bUseSelectionLogic = ActorNode->bUseSelectionLogic;
            Prop.bLogicOverridesAffinity = ActorNode->bLogicOverridesAffinity;

            // Clone the transform logic instance
            CloneUObjectArray<UDungeonTransformLogic>(Owner, ActorNode->TransformLogics, Prop.TransformLogics);
            Prop.bUseTransformLogic = ActorNode->bUseTransformLogic;

            // Clone the spawn logic instance
            CloneUObjectArray<UDungeonSpawnLogic>(Owner, ActorNode->SpawnLogics, Prop.SpawnLogics);
            Prop.bUseSpawnLogic = ActorNode->bUseSpawnLogic;

            // Insert Child Marker emitters
            TArray<UEdGraphNode_DungeonMarkerEmitter*> EmitterNodes;
            GetChildNodes<UEdGraphNode_DungeonMarkerEmitter>(ActorNode, EmitterNodes);
            for (UEdGraphNode_DungeonMarkerEmitter* EmitterNode : EmitterNodes) {
                //if (!EmitterNode || !EmitterNode->IsValidLowLevel()) continue;
                if (EmitterNode && EmitterNode->ParentMarker) {
                    FPropChildSocketData ChildSocket;
                    ChildSocket.SocketType = EmitterNode->ParentMarker->MarkerName;
                    ChildSocket.Offset = EmitterNode->Offset;
                    Prop.ChildSockets.Add(ChildSocket);
                }
            }
            OutProps.Add(Prop);
        }
    }
}

void UEdGraph_DungeonProp::RecreateDefaultMarkerNodes(TSubclassOf<UDungeonBuilder> BuilderClass) {
    TArray<FString> DefaultMarkerNames;
    if (BuilderClass) {
        UDungeonBuilder* Builder = NewObject<UDungeonBuilder>(static_cast<UObject*>(GetTransientPackage()), BuilderClass);
        if (Builder) {
            Builder->GetDefaultMarkerNames(DefaultMarkerNames);
        }
    }
    RecreateDefaultMarkerNodes(DefaultMarkerNames);
}

void UEdGraph_DungeonProp::RecreateDefaultMarkerNodes(const TArray<FString> InMarkerNames) {
    // Remove all the unused builder marker nodes 
    {
        TSet<UEdGraphNode_DungeonMarker*> ReferencedNodes;

        // First cache all the marker emitter nodes' references
        TArray<UEdGraphNode_DungeonMarkerEmitter*> EmitterNodes;
        GetNodesOfClass<UEdGraphNode_DungeonMarkerEmitter>(EmitterNodes);
        for (UEdGraphNode_DungeonMarkerEmitter* EmitterNode : EmitterNodes) {
            if (EmitterNode && EmitterNode->ParentMarker) {
                ReferencedNodes.Add(EmitterNode->ParentMarker);
            }
        }

        // Now grab all the marker nodes, and remove the builder created nodes that are not referenced by emitters
        TArray<UEdGraphNode_DungeonMarker*> MarkerNodes;
        GetNodesOfClass<UEdGraphNode_DungeonMarker>(MarkerNodes);
        for (UEdGraphNode_DungeonMarker* MarkerNode : MarkerNodes) {
            if (MarkerNode->bBuilderEmittedMarker && !ReferencedNodes.Contains(MarkerNode)) {
                if (MarkerNode->GetOutputPin()->LinkedTo.Num() > 0) {
                    // This node has children
                    continue;
                }

                // Remove this node from the graph
                // Break all node links first so that we don't update the material before deleting
                MarkerNode->BreakAllNodeLinks();
                FBlueprintEditorUtils::RemoveNode(nullptr, MarkerNode, true);
            }
        }
    }

    TSet<FString> ExistingMarkerNames;
    {
        TArray<UEdGraphNode_DungeonMarker*> MarkerNodes;
        GetNodesOfClass<UEdGraphNode_DungeonMarker>(MarkerNodes);
        for (UEdGraphNode_DungeonMarker* MarkerNode : MarkerNodes) {
            ExistingMarkerNames.Add(MarkerNode->MarkerName);
        }
    }


    int32 NumItemsPerRow = 5;
    int32 XDelta = 200;
    int32 YDelta = 400;

    int PositionIndex = 0;

    for (int i = 0; i < InMarkerNames.Num(); i++) {
        FString MarkerName = InMarkerNames[i];
        if (ExistingMarkerNames.Contains(MarkerName)) {
            continue;
        }
        ExistingMarkerNames.Add(MarkerName);

        UEdGraphNode_DungeonMarker* MarkerNode = NewObject<UEdGraphNode_DungeonMarker>(this);
        MarkerNode->bUserDefined = false;
        MarkerNode->bBuilderEmittedMarker = true;
        MarkerNode->Rename(nullptr, this, REN_NonTransactional);
        MarkerNode->MarkerName = MarkerName;
        this->AddNode(MarkerNode, true, false);

        MarkerNode->CreateNewGuid();
        MarkerNode->PostPlacedNewNode();
        MarkerNode->AllocateDefaultPins();
        MarkerNode->NodePosY = -1000;

        bool bOccupied = false;
        int32 X, Y;
        do {
            X = (PositionIndex % NumItemsPerRow) * XDelta;
            Y = (PositionIndex / NumItemsPerRow) * YDelta;
            PositionIndex++;
        }
        while (!ContainsFreeSpace(X, Y, 100));

        MarkerNode->NodePosX = X;
        MarkerNode->NodePosY = Y;
        MarkerNode->SnapToGrid(SNAP_GRID);
    }

}

float GetDistanceSq(int32 X0, int32 Y0, int32 X1, int32 Y1) {
    return (X0 - X1) * (X0 - X1) + (Y0 - Y1) * (Y0 - Y1);
}

bool UEdGraph_DungeonProp::ContainsFreeSpace(int32 X, int32 Y, float Distance) {
    TArray<UEdGraphNode_DungeonBase*> DungeonNodes;
    GetNodesOfClass<UEdGraphNode_DungeonBase>(DungeonNodes);

    for (UEdGraphNode_DungeonBase* Node : DungeonNodes) {
        float DistSq = GetDistanceSq(X, Y, Node->NodePosX, Node->NodePosY);
        if (DistSq < Distance * Distance) {
            return false;
        }
    }
    return true;
}

bool UEdGraph_DungeonProp::IsAssetAcceptableForDrop(const UObject* AssetObject) const {
    if (!AssetObject) return false;

    UClass* AssetClass = AssetObject->GetClass();

    bool bValidClass = AssetClass->IsChildOf<UStaticMesh>()
        || AssetClass->IsChildOf<UParticleSystem>()
        || AssetClass->IsChildOf<UClass>()
        || AssetClass->IsChildOf<AActor>()
        || AssetClass->IsChildOf<UBlueprint>();

    return bValidClass;
}

UEdGraphNode_DungeonBase* UEdGraph_DungeonProp::CreateNewNode(UObject* AssetObject, const FVector2D& Location) {
    UEdGraphNode_DungeonBase* Node = nullptr;

    UClass* AssetClass = AssetObject->GetClass();

    if (AssetClass->IsChildOf<UStaticMesh>()) {
        UEdGraphNode_DungeonMesh* MeshNode = CreateNewNode<UEdGraphNode_DungeonMesh>(Location);
        MeshNode->Mesh = Cast<UStaticMesh>(AssetObject);

        Node = MeshNode;
    }
    else if (AssetClass->IsChildOf<UParticleSystem>()) {
        UEdGraphNode_DungeonParticleSystem* ParticleNode = CreateNewNode<UEdGraphNode_DungeonParticleSystem>(Location);
        ParticleNode->ParticleSystem = Cast<UParticleSystem>(AssetObject);

        Node = ParticleNode;
    }
    else if (AssetClass->IsChildOf<UBlueprint>()) {
        UEdGraphNode_DungeonActorTemplate* ActorNode = CreateNewNode<UEdGraphNode_DungeonActorTemplate>(Location);
        UBlueprint* Blueprint = Cast<UBlueprint>(AssetObject);
        if (Blueprint) {
            ActorNode->SetTemplateClass(Cast<UClass>(Blueprint->GeneratedClass));
        }

        Node = ActorNode;
    }
    else if (AssetClass->IsChildOf<AActor>()) {
        UEdGraphNode_DungeonActorTemplate* ActorNode = CreateNewNode<UEdGraphNode_DungeonActorTemplate>(Location);
        ActorNode->SetTemplateClass(AssetClass);

        Node = ActorNode;
    }
    else if (AssetClass->IsChildOf<UClass>()) {
        UClass* ActorClass = Cast<UClass>(AssetObject);
        if (ActorClass->IsChildOf(ASpotLight::StaticClass())) {
            Node = CreateNewNode<UEdGraphNode_DungeonSpotLight>(Location);
        }
        else if (ActorClass->IsChildOf(APointLight::StaticClass())) {
            Node = CreateNewNode<UEdGraphNode_DungeonPointLight>(Location);
        }
        else {
            UEdGraphNode_DungeonActorTemplate* ActorNode = CreateNewNode<UEdGraphNode_DungeonActorTemplate>(Location);
            ActorNode->SetTemplateClass(ActorClass);
            Node = ActorNode;
        }
    }
    else {
        // Try to create an actor from the asset factory
        FAssetData AssetData(AssetObject);
        UActorFactory* ActorFactory = FActorFactoryAssetProxy::GetFactoryForAsset(AssetData);
        if (ActorFactory) {
            // Create an actor node
            UEdGraphNode_DungeonActorTemplate* ActorNode = CreateNewNode<UEdGraphNode_DungeonActorTemplate>(Location);
            ActorNode->SetTemplateFromAsset(AssetObject, ActorFactory);

            Node = ActorNode;
        }
    }

    return Node;
}

FDelegateHandle UEdGraph_DungeonProp::AddOnNodePropertyChangedHandler(const FOnGraphChanged::FDelegate& InHandler) {
    return OnNodePropertyChanged.Add(InHandler);
}

void UEdGraph_DungeonProp::RemoveOnNodePropertyChangedHandler(FDelegateHandle Handle) {
    OnNodePropertyChanged.Remove(Handle);
}

void UEdGraph_DungeonProp::NotifyNodePropertyChanged(const FEdGraphEditAction& InAction) {
    OnNodePropertyChanged.Broadcast(InAction);
}

#undef LOCTEXT_NAMESPACE

