//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Utils/EditorService/DungeonNonEditorFallbackService.h"

#include "Engine/LevelStreaming.h"
#include "Engine/World.h"

AActor* FDungeonNonEditorFallbackService::CreateGroupActor(UWorld* World, const TArray<AActor*>& MergedActorList,
                                                           const FTransform& Transform) {
    // Not supported (and not needed) during runtime
    return nullptr;
}

UObject* FDungeonNonEditorFallbackService::CloneAsset(UObject* SourceAsset, const FString& FolderPath) {
    return SourceAsset;
}

void FDungeonNonEditorFallbackService::SetActorEnabled(AActor* Actor, bool bEnabled) {
    bool bHidden = !bEnabled;
    Actor->SetActorHiddenInGame(bHidden);
    Actor->SetActorTickEnabled(bEnabled);
    Actor->SetActorEnableCollision(bEnabled);
}

ULevelStreaming* FDungeonNonEditorFallbackService::AddLevelToWorld(UWorld* InWorld, const TCHAR* LevelPackageName,
                                                                   TSubclassOf<ULevelStreaming> LevelStreamingClass,
                                                                   const FTransform& LevelTransform) {
    return nullptr;
}

