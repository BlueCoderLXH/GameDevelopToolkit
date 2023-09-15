//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/ThemeEngine/SceneProviders/DungeonSceneProvider.h"

#include "Core/Actors/DungeonActorTemplate.h"
#include "Core/Dungeon.h"
#include "Core/Utils/DungeonModelHelper.h"
#include "Core/Utils/Profiler.h"

DEFINE_LOG_CATEGORY(DungeonSceneProvider);


FDungeonSceneProvider::FDungeonSceneProvider(ADungeon* InDungeon)
    : Dungeon(InDungeon)
    , BuildPriorityLocation(FVector::ZeroVector)
    , CurrentGTCommandExecutionIndex(0)
{
    LevelOverride = Dungeon ? Dungeon->GetLevel() : nullptr;
}

void FDungeonSceneProvider::OnDungeonBuildStart() {
}

void FDungeonSceneProvider::AddStaticMesh(UDungeonMesh* Mesh, const FDungeonSceneProviderContext& Context) {
    SceneProviderCommand_CreateMesh Command(Dungeon, LevelOverride, Context, Mesh);
    Command.Execute(GetDungeonWorld());
}

void FDungeonSceneProvider::AddLight(UPointLightComponent* LightTemplate, const FDungeonSceneProviderContext& Context) {
    SceneProviderCommand_AddLight Command(Dungeon, LevelOverride, Context, LightTemplate);
    Command.Execute(GetDungeonWorld());
}

void FDungeonSceneProvider::AddParticleSystem(UParticleSystem* ParticleTemplate, const FDungeonSceneProviderContext& Context) {
    SceneProviderCommand_AddParticleSystem Command(Dungeon, LevelOverride, Context, ParticleTemplate);
    Command.Execute(GetDungeonWorld());
}

void FDungeonSceneProvider::AddActorFromTemplate(UClass* ClassTemplate, const FDungeonSceneProviderContext& Context) {
    SceneProviderCommand_AddActor Command(Dungeon, LevelOverride, Context, ClassTemplate);
    Command.Execute(GetDungeonWorld());
}

void FDungeonSceneProvider::AddClonedActor(UDungeonActorTemplate* ActorTemplate, const FDungeonSceneProviderContext& Context) {
    SceneProviderCommand_CloneActor Command(Dungeon, LevelOverride, Context, ActorTemplate);
    Command.Execute(GetDungeonWorld());
}

void FDungeonSceneProvider::AddGroupActor(const TArray<FName>& ActorNodeIds,
                                          const FDungeonSceneProviderContext& Context) {
    SceneProviderCommand_CreateGroupActor Command(Dungeon, LevelOverride, Context, ActorNodeIds);
    Command.Execute(GetDungeonWorld());
}

void FDungeonSceneProvider::ProcessUnsupportedObject(UObject* Object, const FDungeonSceneProviderContext& Context) {
}

void FDungeonSceneProvider::ExecuteCustomCommand(TSharedPtr<FSceneProviderCommand> SceneCommand) {
    SceneCommand->Execute(GetDungeonWorld());
}

void FDungeonSceneProvider::AddReferencedObjects(FReferenceCollector& Collector) {
    if (Dungeon) {
        Collector.AddReferencedObject(Dungeon);
    }
    
    if (LevelOverride) {
        Collector.AddReferencedObject(LevelOverride);
    }
}

void FDungeonSceneProvider::RunGameThreadCommands(float MaxBuildTimePerFrameMs) {
    const double MAX_FRAME_TIME = MaxBuildTimePerFrameMs;

    FDateTime frameStartTime = FDateTime::Now();
    while (CurrentGTCommandExecutionIndex < GameThreadCommands.Num()) {
        TSharedPtr<FSceneProviderCommand> Command = GameThreadCommands[CurrentGTCommandExecutionIndex];
        if (Command->ShouldStallExecution()) {
            break;
        }

        if (Command->ShouldWaitTillEndOfFrame()) {
            // We need to wait for a tick so we process it in the next frame
            Command->SetShouldWaitTillEndOfFrame(false);
            break;
        }
        Command->Execute(GetDungeonWorld());
        CurrentGTCommandExecutionIndex++;

        double elapsedMilli = (FDateTime::Now() - frameStartTime).GetTotalMilliseconds();
        if (MAX_FRAME_TIME > 0 && elapsedMilli >= MAX_FRAME_TIME) {
            break;
        }
    }

    if (CurrentGTCommandExecutionIndex >= GameThreadCommands.Num()) {
        GameThreadCommands.Reset();
    }
}

void FDungeonSceneProvider::ApplyExecutionWeights() {
    for (TSharedPtr<FSceneProviderCommand> Command : GameThreadCommands) {
        Command->UpdateExecutionPriority(BuildPriorityLocation);
    }
}


bool FDungeonSceneProvider::IsRunningGameThreadCommands() const {
    return CurrentGTCommandExecutionIndex < GameThreadCommands.Num();
}

void FDungeonSceneProvider::SetBuildPriorityLocation(const FVector& Location) {
    this->BuildPriorityLocation = Location;
}

UWorld* FDefaultDungeonSceneProvider::GetDungeonWorld() {
    return World.Get();
}

void SceneProviderCommand_DestroyActor::ExecuteImpl(UWorld* World) {
    if (Actor && Actor->IsValidLowLevel()) {
        Actor->Destroy();
        Actor = nullptr;
    }
}

