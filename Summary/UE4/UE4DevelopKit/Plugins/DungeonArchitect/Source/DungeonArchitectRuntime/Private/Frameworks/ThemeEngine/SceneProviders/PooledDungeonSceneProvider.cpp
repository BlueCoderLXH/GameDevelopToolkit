//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/ThemeEngine/SceneProviders/PooledDungeonSceneProvider.h"

#include "Core/Actors/DungeonActorTemplate.h"
#include "Core/Dungeon.h"
#include "Core/Utils/DungeonModelHelper.h"
#include "Frameworks/ThemeEngine/SceneProviders/SceneProviderCommand.h"

#include "EngineUtils.h"

DEFINE_LOG_CATEGORY(PooledDungeonSceneProvider);

void FPooledDungeonSceneProvider::OnDungeonBuildStart() {
    FDungeonSceneProvider::OnDungeonBuildStart();

    NodeActorPool.Reset();
    const FName DungeonTag = UDungeonModelHelper::GetDungeonIdTag(Dungeon);

    // Collect all actors that have a "Dungeon" tag
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr) {
        AActor* Actor = *ActorItr;

        FName NodeId;
        bool bFoundDungeonTag = UDungeonModelHelper::GetNodeId(DungeonTag, Actor, NodeId);
        if (!bFoundDungeonTag) {
            // This actor does not belong to the dungeon
            continue;
        }

        if (Actor->Tags.Contains(FSceneProviderCommand::TagComplexActor)) {
            // We don't want to reuse a complex actor.  Instead we will create new ones and these old ones
            TSharedPtr<FSceneProviderCommand> Command = MakeShareable(new SceneProviderCommand_DestroyActor(Actor));
            Command->ExecutionPriority = MAX_flt;
            GameThreadCommands.Add(Command);
            continue;
        }

        TArray<TWeakObjectPtr<AActor>>& ActorList = NodeActorPool.FindOrAdd(NodeId);
        ActorList.Add(Actor);
    }
}

void FPooledDungeonSceneProvider::OnDungeonBuildStop() {
    // Destroy every unused object in the actor pool, since the dungeon building has ended
    TArray<TArray<TWeakObjectPtr<AActor>>> ActorValues;
    NodeActorPool.GenerateValueArray(ActorValues);
    for (TArray<TWeakObjectPtr<AActor>> ActorArray : ActorValues) {
        for (TWeakObjectPtr<AActor> Actor : ActorArray) {
            if (Actor.IsValid()) {
                TSharedPtr<FSceneProviderCommand> Command = MakeShareable(new SceneProviderCommand_DestroyActor(Actor.Get()));
                GameThreadCommands.Add(Command);
            }
        }
    }

    NodeActorPool.Reset();

    // Sort the game commands based on priority
    ApplyExecutionWeights();
    GameThreadCommands.Sort(FSceneProviderCommand::WeightSortPredicate);
}

AActor* FPooledDungeonSceneProvider::ReuseFromPool(const FName& NodeId, const FTransform& InTransform) {
    // Check if we have a free actor of this type in the pool
    if (NodeActorPool.Contains(NodeId)) {
        TArray<TWeakObjectPtr<AActor>>& ActorList = NodeActorPool[NodeId];
        if (ActorList.Num() > 0) {
            // A free actor is available in the pool. 
            // Try to find an actor that shares the same transform
            for (int i = 0; i < ActorList.Num(); i++) {
                TWeakObjectPtr<AActor> Actor = ActorList[i];
                if (Actor.IsValid() && Actor->GetTransform().Equals(InTransform)) {
                    ActorList.RemoveAtSwap(i);
                    return Actor.Get();
                }
            }

            // We did not find a free actor that has the requested transform
            // Grab the first free actor and remove it from the pool
            for (int i = 0; i < ActorList.Num(); i++) {
                TWeakObjectPtr<AActor> Actor = ActorList[i];
                if (Actor.IsValid()) {
                    ActorList.RemoveAtSwap(i);
                    return Actor.Get();
                }
            }
        }
    }

    // No free objects of this type in the pool
    return nullptr;
}

void FPooledDungeonSceneProvider::AddStaticMesh(UDungeonMesh* Mesh, const FDungeonSceneProviderContext& Context) {
    if (Mesh->StaticMesh == nullptr) return;


    TSharedPtr<FSceneProviderCommand> Command;

    // Check if we have a free actor of this type in the pool. If so use it.
    if (AActor* Actor = ReuseFromPool(Context.NodeId, Context.transform)) {
        Command = MakeShareable(new SceneProviderCommand_ReuseStaticMesh(Dungeon, Context, Actor, Mesh));
    }
    else {
        // No free actor exists.  Create a new actor later in the game thread
        Command = MakeShareable(new SceneProviderCommand_CreateMesh(Dungeon, LevelOverride, Context, Mesh));
    }
    GameThreadCommands.Add(Command);
}

void FPooledDungeonSceneProvider::AddLight(UPointLightComponent* LightTemplate,
                                           const FDungeonSceneProviderContext& Context) {
    // Check if we have a free actor of this type in the pool. If so use it.
    TSharedPtr<FSceneProviderCommand> Command;
    if (AActor* Actor = ReuseFromPool(Context.NodeId, Context.transform)) {
        Command = MakeShareable(new SceneProviderCommand_ReuseLight(Dungeon, Context, Actor, LightTemplate));
    }
    else {
        // No free actor exists.  Create a new actor later in the game thread
        Command = MakeShareable(new SceneProviderCommand_AddLight(Dungeon, LevelOverride, Context, LightTemplate));
    }
    GameThreadCommands.Add(Command);
}

void FPooledDungeonSceneProvider::AddParticleSystem(UParticleSystem* ParticleTemplate,
                                                    const FDungeonSceneProviderContext& Context) {
    // Check if we have a free actor of this type in the pool. If so use it.
    TSharedPtr<FSceneProviderCommand> Command;
    if (AActor* Actor = ReuseFromPool(Context.NodeId, Context.transform)) {
        Command = MakeShareable(
            new SceneProviderCommand_ReuseParticleSystem(Dungeon, Context, Actor, ParticleTemplate));
    }
    else {
        // No free actor exists.  Create a new actor later in the game thread
        Command = MakeShareable(new SceneProviderCommand_AddParticleSystem(Dungeon, LevelOverride, Context, ParticleTemplate));
    }
    GameThreadCommands.Add(Command);
}

void FPooledDungeonSceneProvider::AddActorFromTemplate(UClass* ClassTemplate,
                                                       const FDungeonSceneProviderContext& Context) {
    // Check if we have a free actor of this type in the pool. If so use it.
    TSharedPtr<FSceneProviderCommand> Command;
    if (AActor* Actor = ReuseFromPool(Context.NodeId, Context.transform)) {
        Command = MakeShareable(new SceneProviderCommand_ReuseActorTemplate(Dungeon, Context, Actor, ClassTemplate));
    }
    else {
        Command = MakeShareable(new SceneProviderCommand_AddActor(Dungeon, LevelOverride, Context, ClassTemplate));
    }
    GameThreadCommands.Add(Command);
}

void FPooledDungeonSceneProvider::AddClonedActor(UDungeonActorTemplate* ActorTemplate,
                                                 const FDungeonSceneProviderContext& Context) {
    // Check if we have a free actor of this type in the pool. If so use it.
    TSharedPtr<FSceneProviderCommand> Command;
    if (AActor* Actor = ReuseFromPool(Context.NodeId, Context.transform)) {
        Command = MakeShareable(new SceneProviderCommand_ReuseClonedActor(Dungeon, Context, Actor, ActorTemplate));
    }
    else {
        Command = MakeShareable(new SceneProviderCommand_CloneActor(Dungeon, LevelOverride, Context, ActorTemplate));
    }
    GameThreadCommands.Add(Command);
}

void FPooledDungeonSceneProvider::AddGroupActor(const TArray<FName>& ActorNodeIds,
                                                const FDungeonSceneProviderContext& Context) {
    // Check if we have a free actor of this type in the pool. If so use it.
    AActor* Actor = ReuseFromPool(Context.NodeId, Context.transform);
    if (!Actor) {
        TSharedPtr<FSceneProviderCommand> Command = MakeShareable(
            new SceneProviderCommand_CreateGroupActor(Dungeon, LevelOverride, Context, ActorNodeIds));
        GameThreadCommands.Add(Command);
    }
}

void FPooledDungeonSceneProvider::ExecuteCustomCommand(TSharedPtr<FSceneProviderCommand> SceneCommand) {
    GameThreadCommands.Add(SceneCommand);
}

void FPooledDungeonSceneProvider::AddReferencedObjects(FReferenceCollector& Collector) {
    FDungeonSceneProvider::AddReferencedObjects(Collector);

    Collector.AddReferencedObject(World);
}

UWorld* FPooledDungeonSceneProvider::GetDungeonWorld() {
    return World;
}

