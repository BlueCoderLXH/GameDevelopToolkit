//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Actors/DungeonMesh.h"
#include "Frameworks/ThemeEngine/Rules/DungeonSpawnLogic.h"
#include "Frameworks/ThemeEngine/SceneProviders/DungeonSceneProviderContext.h"

#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "Particles/Emitter.h"
#include "UObject/GCObject.h"

class ADungeon;
class UDungeonSpawnLogic;
class UDungeonActorTemplate;

DECLARE_DELEGATE_OneParam(FSceneProviderCommandActorEvent, AActor*);

class DUNGEONARCHITECTRUNTIME_API FSceneProviderCommand : public FGCObject {
public:
    FSceneProviderCommand(ADungeon* InDungeon, ULevel* InLevelOverride, const FDungeonSceneProviderContext& InContext) :
        ExecutionPriority(MAX_flt), Dungeon(InDungeon), LevelOverride(InLevelOverride), Context(InContext), bWaitForFrameUpdate(false)
    {
    }

    virtual ~FSceneProviderCommand() {
    }

    virtual void Execute(UWorld* World);

    static bool WeightSortPredicate(TSharedPtr<FSceneProviderCommand> cmd1, TSharedPtr<FSceneProviderCommand> cmd2) {
        return (cmd1->ExecutionPriority < cmd2->ExecutionPriority);
    }

    float ExecutionPriority;

    virtual void UpdateExecutionPriority(const FVector& BuildPosition) {
    }

    FORCEINLINE FName CreateNodeTagFromId(const FName& NodeId) {
        return *FString("NODE-").Append(NodeId.ToString());
    }

    virtual bool ShouldStallExecution() const { return false; }
    bool ShouldWaitTillEndOfFrame() const { return bWaitForFrameUpdate; }
    void SetShouldWaitTillEndOfFrame(bool InWaitTillEndOfFrame) { bWaitForFrameUpdate = InWaitTillEndOfFrame; }

    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

    static FName TagComplexActor; // If this tag is preset, we do not reuse this object, instead destroy and recreate it

    virtual void PostInitializeActor(AActor* Actor);

    // If tagged as a complex object, it will not reuse the object, instead destroy and recreate it
    static void TagAsComplexObject(AActor* Actor);
    static void MoveToFolder(ADungeon* Dungeon, AActor* ActorToMove);

    ADungeon* GetDungeon() const { return Dungeon; }
    FDungeonSceneProviderContext GetContext() const { return Context; }
    FSceneProviderCommandActorEvent& GetOnActorSpawned() { return OnActorSpawned; }
    
protected:
    virtual void ExecuteImpl(UWorld* World) = 0;

    void AddNodeTag(AActor* Actor, const FName& NodeId, bool bApplyPrefix = true);
    void MoveToFolder(AActor* ActorToMove);


    void UpdateExecutionPriorityByDistance(const FVector& BuildPosition, const FTransform& CommandTransform) {
        FVector Location = CommandTransform.GetLocation();
        ExecutionPriority = (Location - BuildPosition).SizeSquared();
    }

    template <typename T>
    T* AddActor(UWorld* World, ULevel* Level, const FTransform& transform, const FName& NodeId) {
        FActorSpawnParameters SpawnParams;
        SpawnParams.OverrideLevel = Level;
        T* Actor = World->SpawnActor<T>(T::StaticClass(), SpawnParams);
        Actor->SetActorTransform(transform);
        Actor->Tags.Add(FName("Dungeon"));
        AddNodeTag(Actor, NodeId);
        MoveToFolder(Actor);
        return Actor;
    }

    ADungeon* Dungeon = nullptr;
    ULevel* LevelOverride = nullptr;
    FDungeonSceneProviderContext Context;
    bool bWaitForFrameUpdate;
    FSceneProviderCommandActorEvent OnActorSpawned;
    
private:
    static void ExecuteSpawnLogics(AActor* SpawnedActor, ADungeon* InDungeon,
                                   const TArray<UDungeonSpawnLogic*>& SpawnLogics);

};

class DUNGEONARCHITECTRUNTIME_API SceneProviderCommand_CreateMesh : public FSceneProviderCommand {
public:
    SceneProviderCommand_CreateMesh(ADungeon* InDungeon, ULevel* InLevelOverride, const FDungeonSceneProviderContext& InContext, UDungeonMesh* InMesh)
        : FSceneProviderCommand(InDungeon, InLevelOverride, InContext), Mesh(InMesh) {
    }

    static void SetMeshComponentAttributes(UStaticMeshComponent* StaticMeshComponent,
                                           UStaticMeshComponent* StaticMeshTemplate);

    virtual void UpdateExecutionPriority(const FVector& BuildPosition) override {
        UpdateExecutionPriorityByDistance(BuildPosition, Context.transform);
    }

    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

protected:
    virtual void ExecuteImpl(UWorld* World) override;

private:
    UDungeonMesh* Mesh;
};

class DUNGEONARCHITECTRUNTIME_API SceneProviderCommand_AddLight : public FSceneProviderCommand {
public:
    SceneProviderCommand_AddLight(ADungeon* InDungeon, ULevel* InLevelOverride, const FDungeonSceneProviderContext& InContext,
                                  UPointLightComponent* pLightTemplate)
        : FSceneProviderCommand(InDungeon, InLevelOverride, InContext), LightTemplate(pLightTemplate) {
    }

    static void SetSpotLightAttributes(USpotLightComponent* SpotLightComponent, USpotLightComponent* SpotLightTemplate);
    static void SetPointLightAttributes(UPointLightComponent* PointLightComponent, UPointLightComponent* LightTemplate);

    virtual void UpdateExecutionPriority(const FVector& BuildPosition) override {
        UpdateExecutionPriorityByDistance(BuildPosition, Context.transform);
    }

    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

protected:
    virtual void ExecuteImpl(UWorld* World) override;

private:
    UPointLightComponent* LightTemplate;
};

class DUNGEONARCHITECTRUNTIME_API SceneProviderCommand_AddParticleSystem : public FSceneProviderCommand {
public:
    SceneProviderCommand_AddParticleSystem(ADungeon* InDungeon, ULevel* InLevelOverride, const FDungeonSceneProviderContext& InContext,
                                           UParticleSystem* pParticleTemplate)
        : FSceneProviderCommand(InDungeon, InLevelOverride, InContext), ParticleTemplate(pParticleTemplate) {
    }

    virtual void UpdateExecutionPriority(const FVector& BuildPosition) override {
        UpdateExecutionPriorityByDistance(BuildPosition, Context.transform);
    }

    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

protected:
    virtual void ExecuteImpl(UWorld* World) override;

private:
    UParticleSystem* ParticleTemplate;
};

class DUNGEONARCHITECTRUNTIME_API SceneProviderCommand_AddActor : public FSceneProviderCommand {
public:
    SceneProviderCommand_AddActor(ADungeon* InDungeon, ULevel* InLevelOverride, const FDungeonSceneProviderContext& InContext,
                                  UClass* InClassTemplate)
        : FSceneProviderCommand(InDungeon, InLevelOverride, InContext), ClassTemplate(InClassTemplate) {
    }

    virtual void UpdateExecutionPriority(const FVector& BuildPosition) override {
        UpdateExecutionPriorityByDistance(BuildPosition, Context.transform);
    }

    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

protected:
    virtual void ExecuteImpl(UWorld* World) override;

private:
    UClass* ClassTemplate;
};

class DUNGEONARCHITECTRUNTIME_API SceneProviderCommand_CreateGroupActor : public FSceneProviderCommand {
public:
    SceneProviderCommand_CreateGroupActor(ADungeon* InDungeon, ULevel* InLevelOverride, const FDungeonSceneProviderContext& InContext,
                                          const TArray<FName>& InActorNodeIds)
        : FSceneProviderCommand(InDungeon, InLevelOverride, InContext)
          , ActorNodeIds(InActorNodeIds) {
    }

    virtual void UpdateExecutionPriority(const FVector& BuildPosition) override {
		ExecutionPriority = MAX_flt;
    }

protected:
    virtual void ExecuteImpl(UWorld* World) override;

private:
    TArray<FName> ActorNodeIds;
};


class DUNGEONARCHITECTRUNTIME_API SceneProviderCommand_CloneActor : public FSceneProviderCommand {
public:
    SceneProviderCommand_CloneActor(ADungeon* InDungeon, ULevel* InLevelOverride, const FDungeonSceneProviderContext& InContext,
                                    UDungeonActorTemplate* InActorTemplate)
        : FSceneProviderCommand(InDungeon, InLevelOverride, InContext)
          , ActorTemplate(InActorTemplate) {
    }

    virtual void UpdateExecutionPriority(const FVector& BuildPosition) override {
        UpdateExecutionPriorityByDistance(BuildPosition, Context.transform);
    }

    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

protected:
    virtual void ExecuteImpl(UWorld* World) override;

private:
    UDungeonActorTemplate* ActorTemplate;
};


class DUNGEONARCHITECTRUNTIME_API SceneProviderCommand_SetActorTransform : public FSceneProviderCommand {
public:
    SceneProviderCommand_SetActorTransform(const FDungeonSceneProviderContext& InContext, AActor* InActor) :
        FSceneProviderCommand(nullptr, nullptr, InContext), Actor(InActor) {
    }

    virtual void UpdateExecutionPriority(const FVector& BuildPosition) override {
        UpdateExecutionPriorityByDistance(BuildPosition, Context.transform);
    }

    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

protected:
    virtual void ExecuteImpl(UWorld* World) override;

private:
    AActor* Actor;
};

class DUNGEONARCHITECTRUNTIME_API SceneProviderCommand_DestroyActor : public FSceneProviderCommand {
public:
    SceneProviderCommand_DestroyActor(AActor* pActor) : FSceneProviderCommand(nullptr, nullptr, FDungeonSceneProviderContext()),
                                                        Actor(pActor) {
    }

    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

protected:
    virtual void ExecuteImpl(UWorld* World) override;

private:
    AActor* Actor;
};


class DUNGEONARCHITECTRUNTIME_API SceneProviderCommand_DestroyActorWithTag : public FSceneProviderCommand {
public:
    SceneProviderCommand_DestroyActorWithTag(const FName& InTag) : FSceneProviderCommand(nullptr, nullptr, FDungeonSceneProviderContext()),
                                                                   Tag(InTag) {
    }

    virtual void UpdateExecutionPriority(const FVector& BuildPosition) override {
        ExecutionPriority = MAX_flt;
    }

protected:
    virtual void ExecuteImpl(UWorld* World) override;

private:
    FName Tag;
};


class DUNGEONARCHITECTRUNTIME_API SceneProviderCommand_ReuseActor : public FSceneProviderCommand {
public:
    SceneProviderCommand_ReuseActor(ADungeon* InDungeon, const FDungeonSceneProviderContext& InContext,
                                    AActor* InActorToReuse)
        : FSceneProviderCommand(InDungeon, nullptr, InContext)
          , ActorToReuse(InActorToReuse) {
    }

    virtual void UpdateExecutionPriority(const FVector& BuildPosition) override {
        UpdateExecutionPriorityByDistance(BuildPosition, Context.transform);
    }

    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

protected:
    virtual void ExecuteImpl(UWorld* World) override;

protected:
    AActor* ActorToReuse;
    bool bRerunConstructionScripts = true;
};


class DUNGEONARCHITECTRUNTIME_API SceneProviderCommand_ReuseStaticMesh : public SceneProviderCommand_ReuseActor {
public:
    SceneProviderCommand_ReuseStaticMesh(ADungeon* InDungeon, const FDungeonSceneProviderContext& InContext,
                                         AActor* InActorToReuse, UDungeonMesh* InMesh)
        : SceneProviderCommand_ReuseActor(InDungeon, InContext, InActorToReuse)
          , Mesh(InMesh) {
    }

    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

protected:
    virtual void ExecuteImpl(UWorld* World) override;

private:
    UDungeonMesh* Mesh;
};

class DUNGEONARCHITECTRUNTIME_API SceneProviderCommand_ReuseLight : public SceneProviderCommand_ReuseActor {
public:
    SceneProviderCommand_ReuseLight(ADungeon* InDungeon, const FDungeonSceneProviderContext& InContext,
                                    AActor* InActorToReuse, UPointLightComponent* InLightTemplate)
        : SceneProviderCommand_ReuseActor(InDungeon, InContext, InActorToReuse)
          , LightTemplate(InLightTemplate) {
    }

    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

protected:
    virtual void ExecuteImpl(UWorld* World) override;

private:
    UPointLightComponent* LightTemplate;
};


class DUNGEONARCHITECTRUNTIME_API SceneProviderCommand_ReuseParticleSystem : public SceneProviderCommand_ReuseActor {
public:
    SceneProviderCommand_ReuseParticleSystem(ADungeon* InDungeon, const FDungeonSceneProviderContext& InContext,
                                             AActor* InActorToReuse, UParticleSystem* InParticleTemplate)
        : SceneProviderCommand_ReuseActor(InDungeon, InContext, InActorToReuse)
          , ParticleTemplate(InParticleTemplate) {
    }

    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

protected:
    virtual void ExecuteImpl(UWorld* World) override;

private:
    UParticleSystem* ParticleTemplate;
};

class DUNGEONARCHITECTRUNTIME_API SceneProviderCommand_ReuseActorTemplate : public SceneProviderCommand_ReuseActor {
public:
    SceneProviderCommand_ReuseActorTemplate(ADungeon* InDungeon, const FDungeonSceneProviderContext& InContext,
                                            AActor* InActorToReuse, UClass* InClassTemplate)
        : SceneProviderCommand_ReuseActor(InDungeon, InContext, InActorToReuse)
          , ClassTemplate(InClassTemplate) {
    }

    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

protected:
    virtual void ExecuteImpl(UWorld* World) override;

private:
    UClass* ClassTemplate;
};

class DUNGEONARCHITECTRUNTIME_API SceneProviderCommand_ReuseClonedActor : public SceneProviderCommand_ReuseActor {
public:
    SceneProviderCommand_ReuseClonedActor(ADungeon* InDungeon, const FDungeonSceneProviderContext& InContext,
                                          AActor* InActorToReuse, UDungeonActorTemplate* InActorTemplate)
        : SceneProviderCommand_ReuseActor(InDungeon, InContext, InActorToReuse)
          , ActorTemplate(InActorTemplate) {
    }

    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;

protected:
    virtual void ExecuteImpl(UWorld* World) override;

private:
    UDungeonActorTemplate* ActorTemplate;
};

