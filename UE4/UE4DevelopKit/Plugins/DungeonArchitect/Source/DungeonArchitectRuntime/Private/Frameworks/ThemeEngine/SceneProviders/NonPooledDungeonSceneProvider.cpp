//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/ThemeEngine/SceneProviders/NonPooledDungeonSceneProvider.h"

#include "Core/Dungeon.h"
#include "Core/Utils/DungeonModelHelper.h"

DEFINE_LOG_CATEGORY(NonPooledDungeonSceneProvider);

void FNonPooledDungeonSceneProvider::OnDungeonBuildStart() {
    FDungeonSceneProvider::OnDungeonBuildStart();

}

void FNonPooledDungeonSceneProvider::OnDungeonBuildStop() {
    // Sort the game commands based on priority
    ApplyExecutionWeights();
    GameThreadCommands.Sort(FSceneProviderCommand::WeightSortPredicate);
}

void FNonPooledDungeonSceneProvider::AddStaticMesh(UDungeonMesh* Mesh, const FDungeonSceneProviderContext& Context) {
    if (Mesh->StaticMesh == nullptr) return;

    TSharedPtr<FSceneProviderCommand> Command = MakeShareable(
        new SceneProviderCommand_CreateMesh(Dungeon, LevelOverride, Context, Mesh));
    GameThreadCommands.Add(Command);
}

void FNonPooledDungeonSceneProvider::AddLight(UPointLightComponent* LightTemplate,
                                              const FDungeonSceneProviderContext& Context) {
    // No free actor exists.  Create a new actor later in the game thread
    TSharedPtr<FSceneProviderCommand> Command = MakeShareable(
        new SceneProviderCommand_AddLight(Dungeon, LevelOverride, Context, LightTemplate));
    GameThreadCommands.Add(Command);
}

void FNonPooledDungeonSceneProvider::AddParticleSystem(UParticleSystem* ParticleTemplate,
                                                       const FDungeonSceneProviderContext& Context) {
    // No free actor exists.  Create a new actor later in the game thread
    TSharedPtr<FSceneProviderCommand> Command = MakeShareable(
        new SceneProviderCommand_AddParticleSystem(Dungeon, LevelOverride, Context, ParticleTemplate));
    GameThreadCommands.Add(Command);
}

void FNonPooledDungeonSceneProvider::AddActorFromTemplate(UClass* ClassTemplate,
                                                          const FDungeonSceneProviderContext& Context) {
    TSharedPtr<FSceneProviderCommand> Command = MakeShareable(
        new SceneProviderCommand_AddActor(Dungeon, LevelOverride, Context, ClassTemplate));
    GameThreadCommands.Add(Command);
}

void FNonPooledDungeonSceneProvider::AddClonedActor(UDungeonActorTemplate* ActorTemplate,
                                                    const FDungeonSceneProviderContext& Context) {
    TSharedPtr<FSceneProviderCommand> Command = MakeShareable(
        new SceneProviderCommand_CloneActor(Dungeon, LevelOverride, Context, ActorTemplate));
    GameThreadCommands.Add(Command);
}

void FNonPooledDungeonSceneProvider::AddGroupActor(const TArray<FName>& ActorNodeIds,
                                                   const FDungeonSceneProviderContext& Context) {
    TSharedPtr<FSceneProviderCommand> Command = MakeShareable(
        new SceneProviderCommand_CreateGroupActor(Dungeon, LevelOverride, Context, ActorNodeIds));
    GameThreadCommands.Add(Command);
}

void FNonPooledDungeonSceneProvider::ExecuteCustomCommand(TSharedPtr<FSceneProviderCommand> SceneCommand) {
    GameThreadCommands.Add(SceneCommand);
}

UWorld* FNonPooledDungeonSceneProvider::GetDungeonWorld() {
    return World;
}

