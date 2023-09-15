//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "EdGraphSchema_Extensions.generated.h"

class DungeonSchemaExtensionProvider;

UCLASS()
class DUNGEONARCHITECTEDITOR_API UEdGraphSchema_Extensions : public UObject {
    GENERATED_BODY()

public:
    /** Returns the extension manager and creates it if missing */
    static UEdGraphSchema_Extensions& Get();

    void AddExtension(TSharedPtr<DungeonSchemaExtensionProvider> Extension);

    void CreateCustomActions(TArray<TSharedPtr<FEdGraphSchemaAction>>& OutActions, const UEdGraph* Graph,
                             UEdGraph* OwnerOfTemporaries, bool bShowNewMesh, bool bShowNewMarker,
                             bool bShowMarkerEmitters);

private:
    static class UEdGraphSchema_Extensions* SchemaExtensionSingleton;
    TArray<TSharedPtr<DungeonSchemaExtensionProvider>> Extensions;
};


class DUNGEONARCHITECTEDITOR_API DungeonSchemaExtensionProvider {
public:
    virtual ~DungeonSchemaExtensionProvider() {
    }

    virtual void CreateCustomActions(TArray<TSharedPtr<FEdGraphSchemaAction>>& OutActions, const UEdGraph* Graph,
                                     UEdGraph* OwnerOfTemporaries, bool bShowNewMesh, bool bShowNewMarker,
                                     bool bShowMarkerEmitters) = 0;
};

