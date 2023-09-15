//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonMesh.h"


UEdGraphNode_DungeonMesh::UEdGraphNode_DungeonMesh(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
    AdvancedOptions = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, "Template");
    AdvancedOptions->SetMobility(EComponentMobility::Static);
    AdvancedOptions->SetCanEverAffectNavigation(true);
    AdvancedOptions->bCastStaticShadow = true;
}


UObject* UEdGraphNode_DungeonMesh::GetNodeAssetObject(UObject* Outer) {

    UDungeonMesh* AssetObject = NewObject<UDungeonMesh>(Outer);
    AssetObject->StaticMesh = Mesh;

    UStaticMeshComponent* AdvancedOptionsCopy = NewObject<UStaticMeshComponent>(
        Outer, NAME_None, RF_NoFlags, AdvancedOptions);
    AssetObject->Template = AdvancedOptionsCopy;

    AssetObject->MaterialOverrides = MaterialOverrides;
    AssetObject->CalculateHashCode();
    return AssetObject;
}

UObject* UEdGraphNode_DungeonMesh::GetThumbnailAssetObject() {
    return Mesh;
}

void UEdGraphNode_DungeonMesh::PostEditChangeProperty(struct FPropertyChangedEvent& e) {
    UEdGraphNode_DungeonActorBase::PostEditChangeProperty(e);

    if (!e.Property) return;

    FName PropertyName = e.Property->GetFName();
    if (PropertyName == GET_MEMBER_NAME_CHECKED(UEdGraphNode_DungeonMesh, Mesh)) {
        AdvancedOptions->SetStaticMesh(Mesh);
    }
}

