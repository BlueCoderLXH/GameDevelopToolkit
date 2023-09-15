//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/ThemeEditor/Graph/EdGraphSchema_Extensions.h"


UEdGraphSchema_Extensions* UEdGraphSchema_Extensions::SchemaExtensionSingleton = nullptr;

UEdGraphSchema_Extensions& UEdGraphSchema_Extensions::Get() {
    // Create it if we need to
    if (SchemaExtensionSingleton == nullptr) {
        SchemaExtensionSingleton = NewObject<UEdGraphSchema_Extensions>();

        // Keep the singleton alive
        SchemaExtensionSingleton->AddToRoot();
    }

    return *SchemaExtensionSingleton;
}

void UEdGraphSchema_Extensions::AddExtension(TSharedPtr<DungeonSchemaExtensionProvider> Extension) {
    Extensions.Add(Extension);
}

void UEdGraphSchema_Extensions::CreateCustomActions(TArray<TSharedPtr<FEdGraphSchemaAction>>& OutActions,
                                                    const UEdGraph* Graph, UEdGraph* OwnerOfTemporaries,
                                                    bool bShowNewMesh, bool bShowNewMarker, bool bShowMarkerEmitters) {
    for (TSharedPtr<DungeonSchemaExtensionProvider> Extension : Extensions) {
        Extension->CreateCustomActions(OutActions, Graph, OwnerOfTemporaries, bShowNewMesh, bShowNewMarker,
                                       bShowMarkerEmitters);
    }
}

