//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Utils/DungeonBPFunctionLibrary.h"

#include "Core/Dungeon.h"
#include "Core/Utils/DungeonModelHelper.h"
#include "Frameworks/ThemeEngine/SceneProviders/SceneProviderCommand.h"

#include "Engine/LevelStreamingDynamic.h"

DEFINE_LOG_CATEGORY_STATIC(LogDungeonBPLibrary, Log, All);

AActor* UDungeonBPFunctionLibrary::SpawnDungeonOwnedActor(ADungeon* Dungeon, TSubclassOf<AActor> ActorClass,
                                                          const FTransform& Transform) {
    if (!Dungeon) {
        UE_LOG(LogDungeonBPLibrary, Error, TEXT("Invalid Dungeon reference passed to SpawnDungeonActor"));
        return nullptr;
    }

    UWorld* World = Dungeon->GetWorld();
    AActor* Actor = World->SpawnActor<AActor>(ActorClass, Transform);
    if (Actor) {
        if (World->WorldType == EWorldType::Editor) {
            Actor->RerunConstructionScripts();
        }
        const FName DungeonIdTag = UDungeonModelHelper::GetDungeonIdTag(Dungeon);
        Actor->Tags.Add(DungeonIdTag);

        FSceneProviderCommand::MoveToFolder(Dungeon, Actor);
        FSceneProviderCommand::TagAsComplexObject(Actor);
    }

    return Actor;
}

ULevelStreamingDynamic* UDungeonBPFunctionLibrary::StreamDungeonModuleLevel(UObject* WorldContextObject, TSoftObjectPtr<UWorld> Level, int32 InstanceId, const FVector& Location,
        const FRotator& Rotation, bool& bOutSuccess) {
    UPackage* LevelPackage = nullptr;
    ULevelStreamingDynamic* StreamingLevel = FLevelStreamLoader::LoadLevelInstanceBySoftObjectPtr(
        WorldContextObject, Level.GetLongPackageName(), InstanceId, Location, Rotation, bOutSuccess, LevelPackage);

    StreamingLevel->SetShouldBeLoaded(true);
    StreamingLevel->SetShouldBeVisible(true);
    StreamingLevel->bShouldBlockOnLoad = true;
    StreamingLevel->bInitiallyLoaded = true;
    StreamingLevel->bInitiallyVisible = true;
    return StreamingLevel;
}

bool UDungeonBPFunctionLibrary::ActorBelongsToDungeon(ADungeon* Dungeon, AActor* ActorToCheck) {
    if (!Dungeon || !ActorToCheck) return false;
    const FName DungeonIdTag = UDungeonModelHelper::GetDungeonIdTag(Dungeon);
    return ActorToCheck->Tags.Contains(DungeonIdTag);
}

bool UDungeonBPFunctionLibrary::DungeonObjectHasAuthority(UObject* Object) {
    AActor* Actor = nullptr;
    while (Object) {
        Actor = Cast<AActor>(Object);
        if (Actor) break;
        Object = Object->GetOuter();
    }
    
    if (!Actor) return false;
    return Actor->HasAuthority();
}

