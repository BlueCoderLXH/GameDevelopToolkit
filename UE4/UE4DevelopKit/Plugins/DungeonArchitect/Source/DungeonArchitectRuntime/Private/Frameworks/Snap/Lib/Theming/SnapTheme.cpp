//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Snap/Lib/Theming/SnapTheme.h"


void FSnapThemeSceneProvider::AddStaticMesh(UDungeonMesh* Mesh, const FDungeonSceneProviderContext& Context) {
    SceneProviderCommand_CreateMesh Command(Dungeon, LevelOverride, Context, Mesh);
    Command.GetOnActorSpawned().BindRaw(this, &FSnapThemeSceneProvider::OnActorSpawned);
    Command.Execute(GetDungeonWorld());
}

void FSnapThemeSceneProvider::AddLight(UPointLightComponent* LightTemplate, const FDungeonSceneProviderContext& Context) {
    SceneProviderCommand_AddLight Command(Dungeon, LevelOverride, Context, LightTemplate);
    Command.GetOnActorSpawned().BindRaw(this, &FSnapThemeSceneProvider::OnActorSpawned);
    Command.Execute(GetDungeonWorld());
}

void FSnapThemeSceneProvider::AddParticleSystem(UParticleSystem* ParticleTemplate, const FDungeonSceneProviderContext& Context) {
    SceneProviderCommand_AddParticleSystem Command(Dungeon, LevelOverride, Context, ParticleTemplate);
    Command.GetOnActorSpawned().BindRaw(this, &FSnapThemeSceneProvider::OnActorSpawned);
    Command.Execute(GetDungeonWorld());
}

void FSnapThemeSceneProvider::AddActorFromTemplate(UClass* ClassTemplate, const FDungeonSceneProviderContext& Context) {
    SceneProviderCommand_AddActor Command(Dungeon, LevelOverride, Context, ClassTemplate);
    Command.GetOnActorSpawned().BindRaw(this, &FSnapThemeSceneProvider::OnActorSpawned);
    Command.Execute(GetDungeonWorld());
}

void FSnapThemeSceneProvider::AddClonedActor(UDungeonActorTemplate* ActorTemplate, const FDungeonSceneProviderContext& Context) {
    SceneProviderCommand_CloneActor Command(Dungeon, LevelOverride, Context, ActorTemplate);
    Command.GetOnActorSpawned().BindRaw(this, &FSnapThemeSceneProvider::OnActorSpawned);
    Command.Execute(GetDungeonWorld());
}

void FSnapThemeSceneProvider::ExecuteCustomCommand(TSharedPtr<FSceneProviderCommand> SceneCommand) {
    SceneCommand->GetOnActorSpawned().BindRaw(this, &FSnapThemeSceneProvider::OnActorSpawned);
    SceneCommand->Execute(GetDungeonWorld());
}

void FSnapThemeSceneProvider::OnActorSpawned(AActor* InActor) {
    SpawnedActors.Add(InActor);
}

