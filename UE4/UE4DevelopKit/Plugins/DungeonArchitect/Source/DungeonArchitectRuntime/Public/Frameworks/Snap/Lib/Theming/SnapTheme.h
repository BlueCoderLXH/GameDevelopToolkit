//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/ThemeEngine/SceneProviders/DungeonSceneProvider.h"

class DUNGEONARCHITECTRUNTIME_API FSnapThemeSceneProvider : public FDungeonSceneProvider
{
public:
    FSnapThemeSceneProvider(UWorld* InWorld) : World(InWorld) {}
    virtual UWorld* GetDungeonWorld() override { return World.Get(); }
  
    virtual void AddStaticMesh(UDungeonMesh* Mesh, const FDungeonSceneProviderContext& Context) override;
    virtual void AddLight(UPointLightComponent* LightTemplate, const FDungeonSceneProviderContext& Context) override;
    virtual void AddParticleSystem(UParticleSystem* ParticleTemplate, const FDungeonSceneProviderContext& Context) override;
    virtual void AddActorFromTemplate(UClass* ClassTemplate, const FDungeonSceneProviderContext& Context) override;
    virtual void AddClonedActor(UDungeonActorTemplate* ActorTemplate, const FDungeonSceneProviderContext& Context) override;
    virtual void ExecuteCustomCommand(TSharedPtr<FSceneProviderCommand> SceneCommand) override;
    TArray<TWeakObjectPtr<AActor>> GetSpawnedActors() const { return SpawnedActors; }
    
private:
    void OnActorSpawned(AActor* InActor);
    
private:
    TWeakObjectPtr<UWorld> World;
    TArray<TWeakObjectPtr<AActor>> SpawnedActors;
};

