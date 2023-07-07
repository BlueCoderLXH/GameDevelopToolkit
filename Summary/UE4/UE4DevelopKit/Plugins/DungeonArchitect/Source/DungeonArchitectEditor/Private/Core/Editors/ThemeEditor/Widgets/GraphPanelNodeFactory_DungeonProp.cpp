//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/ThemeEditor/Widgets/GraphPanelNodeFactory_DungeonProp.h"

#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonMarker.h"
#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonMarkerEmitter.h"
#include "Core/Editors/ThemeEditor/Widgets/SGraphNode_DungeonActor.h"
#include "Core/Editors/ThemeEditor/Widgets/SGraphNode_DungeonMarker.h"
#include "Core/Editors/ThemeEditor/Widgets/SGraphNode_DungeonMarkerEmitter.h"

FGraphPanelNodeFactory_DungeonProp::FGraphPanelNodeFactory_DungeonProp() {
}

TSharedPtr<class SGraphNode> FGraphPanelNodeFactory_DungeonProp::CreateNode(UEdGraphNode* Node) const {
    if (UEdGraphNode_DungeonActorBase* ActorNode = Cast<UEdGraphNode_DungeonActorBase>(Node)) {
        TSharedPtr<SGraphNode_DungeonActor> SNode = SNew(SGraphNode_DungeonActor, ActorNode);
        ActorNode->PropertyObserver = SNode;
        return SNode;
    }
    if (UEdGraphNode_DungeonMarker* MarkerNode = Cast<UEdGraphNode_DungeonMarker>(Node)) {
        TSharedPtr<SGraphNode_DungeonMarker> SNode = SNew(SGraphNode_DungeonMarker, MarkerNode);
        MarkerNode->PropertyObserver = SNode;
        return SNode;
    }
    if (UEdGraphNode_DungeonMarkerEmitter* MarkerEmitterNode = Cast<UEdGraphNode_DungeonMarkerEmitter>(Node)) {
        TSharedPtr<SGraphNode_DungeonMarkerEmitter> SNode = SNew(SGraphNode_DungeonMarkerEmitter, MarkerEmitterNode);
        MarkerEmitterNode->PropertyObserver = SNode;
        return SNode;
    }

    return nullptr;
}

