//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/ThemeEngine/SceneProviders/DungeonSceneProvider.h"

DECLARE_LOG_CATEGORY_EXTERN(PooledDungeonSceneProvider, Log, All);


/** Pools the actors in the scene and reuses them if possible, while rebuilding */
class DUNGEONARCHITECTRUNTIME_API FPooledDungeonSceneProvider : public FDungeonSceneProvider {
public:
    FPooledDungeonSceneProvider(ADungeon* pDungeon, UWorld* pWorld) : FDungeonSceneProvider(pDungeon), World(pWorld) {
    }

    virtual ~FPooledDungeonSceneProvider() {
    }

    virtual void OnDungeonBuildStart() override;
    virtual void OnDungeonBuildStop() override;
    virtual void AddStaticMesh(UDungeonMesh* Mesh, const FDungeonSceneProviderContext& Context) override;
    virtual void AddLight(UPointLightComponent* LightTemplate, const FDungeonSceneProviderContext& Context) override;
    virtual void AddParticleSystem(UParticleSystem* ParticleTemplate, const FDungeonSceneProviderContext& Context) override;
    virtual void AddActorFromTemplate(UClass* ClassTemplate, const FDungeonSceneProviderContext& Context) override;
    virtual void AddClonedActor(UDungeonActorTemplate* ActorTemplate, const FDungeonSceneProviderContext& Context) override;
    virtual void AddGroupActor(const TArray<FName>& ActorNodeIds, const FDungeonSceneProviderContext& Context) override;
    virtual void ExecuteCustomCommand(TSharedPtr<FSceneProviderCommand> SceneCommand) override;

    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
    virtual UWorld* GetDungeonWorld() override;

protected:
    /** Reuses an object from pool if available. Returns true if an actor was found. If found, it enqueues a reuse command */
    AActor* ReuseFromPool(const FName& NodeId, const FTransform& InTransform);

protected:
    UWorld* World;
    TMap<FName, TArray<TWeakObjectPtr<AActor>>> NodeActorPool;
};

