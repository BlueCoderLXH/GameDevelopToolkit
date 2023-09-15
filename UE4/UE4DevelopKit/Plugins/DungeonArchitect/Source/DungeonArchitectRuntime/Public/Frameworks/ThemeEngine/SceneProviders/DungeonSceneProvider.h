//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/ThemeEngine/SceneProviders/DungeonSceneProviderContext.h"
#include "Frameworks/ThemeEngine/SceneProviders/SceneProviderCommand.h"

#include "Components/PointLightComponent.h"

DECLARE_LOG_CATEGORY_EXTERN(DungeonSceneProvider, Log, All);

class UDungeonActorTemplate;

class DUNGEONARCHITECTRUNTIME_API FDungeonSceneProvider : public FGCObject {
public:
    FDungeonSceneProvider() : FDungeonSceneProvider(nullptr) {}
    FDungeonSceneProvider(ADungeon* InDungeon);

    virtual ~FDungeonSceneProvider() {
    }

    virtual void OnDungeonBuildStart();

    virtual void OnDungeonBuildStop() {
    }

    virtual void AddStaticMesh(UDungeonMesh* Mesh, const FDungeonSceneProviderContext& Context);
    virtual void AddLight(UPointLightComponent* LightTemplate, const FDungeonSceneProviderContext& Context);
    virtual void AddParticleSystem(UParticleSystem* ParticleTemplate, const FDungeonSceneProviderContext& Context);
    virtual void AddActorFromTemplate(UClass* ClassTemplate, const FDungeonSceneProviderContext& Context);
    virtual void AddClonedActor(UDungeonActorTemplate* ActorTemplate, const FDungeonSceneProviderContext& Context);
    virtual void AddGroupActor(const TArray<FName>& ActorNodeIds, const FDungeonSceneProviderContext& Context);
    virtual void ProcessUnsupportedObject(UObject* Object, const FDungeonSceneProviderContext& Context);
    virtual void ExecuteCustomCommand(TSharedPtr<FSceneProviderCommand> SceneCommand);

    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
    
    virtual UWorld* GetDungeonWorld() = 0;

    // Run all the queued commands. This will be called from the game thread. The commands might have been inserted from the background thread
    void RunGameThreadCommands(float MaxBuildTimePerFrameMs);
    bool IsRunningGameThreadCommands() const;
    void SetBuildPriorityLocation(const FVector& Location);

    void ApplyExecutionWeights();
    void SetLevelOverride(ULevel* InLevel) { LevelOverride = InLevel; }

protected:
    ADungeon* Dungeon = nullptr;
    ULevel* LevelOverride = nullptr;

    // The location to start building from and spreading out from here, while building asynchronously
    FVector BuildPriorityLocation;

    // Commands to be run in the game thread. Push a command here if we cannot perform a specific tasks in the background thread
    TArray<TSharedPtr<FSceneProviderCommand>> GameThreadCommands;

private:
    int32 CurrentGTCommandExecutionIndex;
};

class DUNGEONARCHITECTRUNTIME_API FDefaultDungeonSceneProvider : public FDungeonSceneProvider {
public:
    FDefaultDungeonSceneProvider(ADungeon* InDungeon, UWorld* InWorld) : FDungeonSceneProvider(InDungeon), World(InWorld) {
    }

    virtual UWorld* GetDungeonWorld() override;

private:
    TWeakObjectPtr<UWorld> World;
};

/** Ignores all requests to create objects on the scene */
class DUNGEONARCHITECTRUNTIME_API FNullDungeonSceneProvider : public FDefaultDungeonSceneProvider {
public:
    FNullDungeonSceneProvider(ADungeon* pDungeon, UWorld* World) : FDefaultDungeonSceneProvider(pDungeon, World) {
    }

    virtual void AddStaticMesh(UDungeonMesh* Mesh, const FDungeonSceneProviderContext& Context) override {
    }

    virtual void AddLight(UPointLightComponent* LightTemplate, const FDungeonSceneProviderContext& Context) override {
    }

    virtual void AddParticleSystem(UParticleSystem* ParticleTemplate, const FDungeonSceneProviderContext& Context) override {
    }

    virtual void AddActorFromTemplate(UClass* ClassTemplate, const FDungeonSceneProviderContext& Context) override {
    }
};

