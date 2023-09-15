//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Actors/DungeonInstancedMeshActor.h"

#include "Core/Actors/DungeonMesh.h"

ADungeonInstancedMeshActor::ADungeonInstancedMeshActor(const FObjectInitializer& ObjectInitializer) : Super(
    ObjectInitializer) {
    RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, "SceneRoot");
    if (RootComponent) {
        RootComponent->SetMobility(EComponentMobility::Static);
    }
}

void ADungeonInstancedMeshActor::OnBuildStart() {
    //BuildCache();
    DestroyAllInstances();
}

void ADungeonInstancedMeshActor::OnBuildStop() {
    PurgeUsedInstances();
    InstancedComponentCache.Reset();

    MarkComponentsRenderStateDirty();
}

void ADungeonInstancedMeshActor::PurgeUsedInstances() {
    // Get the list of all the instanced mesh components
    TArray<UHierarchicalInstancedStaticMeshComponent*> Components;
    GetComponents<UHierarchicalInstancedStaticMeshComponent>(Components);
    for (UHierarchicalInstancedStaticMeshComponent* Component : Components) {
        if (Component->GetInstanceCount() == 0) {
            Component->DestroyComponent();
            BlueprintCreatedComponents.Remove(Component);
        }
    }
}

void ADungeonInstancedMeshActor::DestroyAllInstances() {
    InstancedComponentCache.Reset();

    TArray<UHierarchicalInstancedStaticMeshComponent*> InstancedComponentArray;
    GetComponents<UHierarchicalInstancedStaticMeshComponent>(InstancedComponentArray);
    for (UHierarchicalInstancedStaticMeshComponent* Component : InstancedComponentArray) {
        Component->DestroyComponent();
        BlueprintCreatedComponents.Remove(Component);
    }
}

UHierarchicalInstancedStaticMeshComponent* ADungeonInstancedMeshActor::GetInstancedComponent(UDungeonMesh* Mesh) {
    if (!Mesh) {
        return nullptr;
    }

    if (!Mesh->HashCode) {
        // Calculate the hash code if not available (for backward compatibility)
        // TODO: Check performance for null meshes since has code will be 0
        Mesh->CalculateHashCode();
    }

    uint32 Hash = Mesh->HashCode;

    // Add the collision trace channels hash
    {
        if (Mesh->Template) {
            const FCollisionResponseContainer& ResponseToChannel = Mesh->Template->GetCollisionResponseToChannels();

            uint32 ChannelHash = FCrc::MemCrc32(ResponseToChannel.EnumArray, sizeof(ResponseToChannel.EnumArray));
            Hash = HashCombine(Hash, ChannelHash);
        }
    }


    // Search the instance cache if we have a cached copy
    {
        UHierarchicalInstancedStaticMeshComponent** SearchResult = InstancedComponentCache.Find(Hash);
        if (SearchResult) {
            UHierarchicalInstancedStaticMeshComponent* Component = *SearchResult;
            return Component;
        }
    }

    UHierarchicalInstancedStaticMeshComponent* Component = NewObject<UHierarchicalInstancedStaticMeshComponent>(this);
    Component->SetStaticMesh(Mesh->StaticMesh);
    Component->SetMobility(EComponentMobility::Static);

    if (Mesh->Template != nullptr) {
        // Copy over the collision responses from the template
        const FCollisionResponseContainer& ResponseToChannel = Mesh->Template->GetCollisionResponseToChannels();
        Component->SetCollisionResponseToChannels(ResponseToChannel);

        // Set the collision profile
        FName CollisionProfile = Mesh->Template->GetCollisionProfileName();
        Component->SetCollisionProfileName(CollisionProfile);

        // Set collision enabled
        ECollisionEnabled::Type CollisionEnabled = Mesh->Template->GetCollisionEnabled();
        Component->SetCollisionEnabled(CollisionEnabled);
    }

    // Set the material overrides
    for (const FMaterialOverride& MaterialOverride : Mesh->MaterialOverrides) {
        Component->SetMaterial(MaterialOverride.index, MaterialOverride.Material);
    }

    Component->SetupAttachment(GetRootComponent());
    Component->RegisterComponent();

    BlueprintCreatedComponents.Add(Component);
    InstancedComponentCache.Add(Hash, Component);

    return Component;
}


void ADungeonInstancedMeshActor::AddMeshInstance(UDungeonMesh* Mesh, const FTransform& Transform) {
    if (!Mesh || !Mesh->StaticMesh) return;
    UHierarchicalInstancedStaticMeshComponent* Component = GetInstancedComponent(Mesh);
    Component->AddInstance(Transform);

    // Set the cull distance
    if (Mesh && Mesh->Template) {
        Component->InstanceStartCullDistance = Mesh->Template->MinDrawDistance;
        Component->InstanceEndCullDistance = Mesh->Template->LDMaxDrawDistance;
        Component->SetMobility(Mesh->Template->Mobility);

        //Component->MinDrawDistance = Mesh->Template->MinDrawDistance;
        //Component->LDMaxDrawDistance = Mesh->Template->LDMaxDrawDistance;
    }
}

