//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "DungeonInstancedMeshActor.generated.h"

class UDungeonMesh;

/**
 *
 */
UCLASS()
class DUNGEONARCHITECTRUNTIME_API ADungeonInstancedMeshActor : public AActor {
    GENERATED_BODY()

public:
    ADungeonInstancedMeshActor(const FObjectInitializer& ObjectInitializer);

    void OnBuildStart();
    void OnBuildStop();

    /** 
     * Adds a static mesh to appropriate instanced mesh component.
     * A new instanced mesh component is created if none exists
     * This has to be called after BuildComponentCache is called
     */
    void AddMeshInstance(UDungeonMesh* Mesh, const FTransform& Transform);

private:
    void DestroyAllInstances();
    void PurgeUsedInstances();
    UHierarchicalInstancedStaticMeshComponent* GetInstancedComponent(UDungeonMesh* Mesh);

private:
    TMap<uint32, UHierarchicalInstancedStaticMeshComponent*> InstancedComponentCache;
};

