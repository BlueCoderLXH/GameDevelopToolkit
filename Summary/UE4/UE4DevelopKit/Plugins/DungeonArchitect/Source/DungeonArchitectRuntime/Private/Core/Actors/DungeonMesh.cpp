//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Actors/DungeonMesh.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/UObjectBaseUtility.h"

UDungeonMesh::UDungeonMesh(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
    Template = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, "Template");
    Template->SetStaticMesh(StaticMesh);
    Template->SetMobility(EComponentMobility::Static);
    Template->SetCanEverAffectNavigation(true);
    Template->bCastStaticShadow = true;

}

void UDungeonMesh::CalculateHashCode() {
    FString HashData;
    if (StaticMesh) {
        HashData += StaticMesh->GetFullName();
    }

    for (const FMaterialOverride& MaterialOverride : MaterialOverrides) {
        HashData += "|";
        HashData += FString::FromInt(MaterialOverride.index);
        if (MaterialOverride.Material) {
            HashData += "|";
            HashData += MaterialOverride.Material->GetFullName();
        }
    }

    HashCode = GetTypeHash(HashData);
}

