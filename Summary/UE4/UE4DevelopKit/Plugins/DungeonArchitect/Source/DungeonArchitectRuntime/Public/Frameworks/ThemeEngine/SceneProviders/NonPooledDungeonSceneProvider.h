//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/ThemeEngine/SceneProviders/DungeonSceneProvider.h"

DECLARE_LOG_CATEGORY_EXTERN(NonPooledDungeonSceneProvider, Log, All);


/** Pools the actors in the scene and reuses them if possible, while rebuilding */
class DUNGEONARCHITECTRUNTIME_API FNonPooledDungeonSceneProvider : public FDungeonSceneProvider {
public:
    FNonPooledDungeonSceneProvider(ADungeon* pDungeon, UWorld* pWorld) : FDungeonSceneProvider(pDungeon),
                                                                         World(pWorld) {
    }

    virtual ~FNonPooledDungeonSceneProvider() {
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

    virtual UWorld* GetDungeonWorld() override;

protected:
    UWorld* World;
};

