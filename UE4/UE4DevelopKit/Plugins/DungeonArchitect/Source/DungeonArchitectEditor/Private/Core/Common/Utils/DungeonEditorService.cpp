//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Common/Utils/DungeonEditorService.h"

#include "Core/Common/Utils/DungeonEditorUtils.h"

#include "Components/BrushComponent.h"
#include "Editor.h"
#include "Editor/GroupActor.h"
#include "Editor/UnrealEdEngine.h"
#include "EditorBuildUtils.h"
#include "EditorLevelUtils.h"
#include "EditorModeManager.h"
#include "EditorModes.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Engine/WorldComposition.h"
#include "FileHelpers.h"
#include "HAL/PlatformApplicationMisc.h"
#include "InstancedFoliage.h"
#include "InstancedFoliageActor.h"
#include "LevelEditorViewport.h"
#include "LevelUtils.h"
#include "Misc/ScopedSlowTask.h"
#include "ObjectTools.h"
#include "PackageTools.h"
#include "UnrealEdGlobals.h"

DEFINE_LOG_CATEGORY(LogEditorService)

#define LOCTEXT_NAMESPACE "DungeonEditorService"

AActor* FDungeonEditorService::CreateGroupActor(UWorld* World, const TArray<AActor*>& MergedActorList,
                                                const FTransform& Transform) {
    FActorSpawnParameters SpawnParams;
    AGroupActor* GroupActor = World->SpawnActor<AGroupActor>(Transform.GetLocation(), FRotator(Transform.GetRotation()),
                                                             SpawnParams);
    //GroupActor->SetActorTransform(Transform);
    for (AActor* Actor : MergedActorList) {
        GroupActor->Add(*Actor);
    }
    return GroupActor;
}

void CopyFoliageToLevel(ULevel* SourceLevel, ULevel* TargetLevel, const FTransform& LevelTransform,
                        TArray<FGuid>& OutFoliageList) {
    AInstancedFoliageActor* SourceIFA = AInstancedFoliageActor::GetInstancedFoliageActorForLevel(
        SourceLevel, /*bCreateIfNone*/ false);
    if (!SourceIFA) return;

    //SourceIFA->MoveSelectedInstancesToLevel(PersistentLevel);

    AInstancedFoliageActor* TargetIFA = AInstancedFoliageActor::GetInstancedFoliageActorForLevel(
        TargetLevel, /*bCreateIfNone*/ true);

    SourceIFA->Modify();
    TargetIFA->Modify();

    // Do move
    for (auto& MeshPair : SourceIFA->FoliageInfos) {
        UFoliageType* FoliageType = MeshPair.Key;
        TUniqueObj<FFoliageInfo>& MeshInfo = MeshPair.Value;
        FFoliageInfo* TargetMeshInfo = nullptr;
        UFoliageType* TargetFoliageType = TargetIFA->AddFoliageType(FoliageType, &TargetMeshInfo);
        TArray<int32> SourceInstancesToRemove;
        for (int i = 0; i < MeshInfo->Instances.Num(); i++) {
            FFoliageInstance Instance = MeshInfo->Instances[i];
            FTransform TargetTransform;
            FTransform SourceTransform = Instance.GetInstanceWorldTransform();
            FTransform::Multiply(&TargetTransform, &SourceTransform, &LevelTransform);
            FGuid FoliageId = FGuid::NewGuid();
            OutFoliageList.Add(FoliageId);
            Instance.ProceduralGuid = FoliageId;
            Instance.Location = TargetTransform.GetLocation();
            Instance.Rotation = TargetTransform.GetRotation().Rotator();
            Instance.DrawScale3D = TargetTransform.GetScale3D();

            bool bRebuildFoliageTree = (i == MeshInfo->Instances.Num() - 1);
            TargetMeshInfo->AddInstance(TargetIFA, TargetFoliageType, Instance);
            SourceInstancesToRemove.Add(i);
        }
        bool bRebuildFoliageTree = true;
        MeshInfo->RemoveInstances(SourceIFA, SourceInstancesToRemove, bRebuildFoliageTree);
    }

}

void UnloadLevel(UWorld* ThisWorld, ULevel* Level, FName LevelPackageName) {
    if (GLevelEditorModeTools().IsModeActive(FBuiltinEditorModes::EM_Landscape)) {
        GLevelEditorModeTools().ActivateDefaultMode();
    }

    //BroadcastPreLevelsUnloaded();

    if (Level != nullptr) {
        // Unselect all actors before removing the level
        // This avoids crashing in areas that rely on getting a selected actors level. The level will be invalid after its removed.
        for (auto ActorIt = Level->Actors.CreateIterator(); ActorIt; ++ActorIt) {
            GEditor->SelectActor((*ActorIt), /*bInSelected=*/ false, /*bSelectEvenIfHidden=*/ false);
        }

        // In case we have created temporary streaming level object for this sub-level - remove it before unloading sub-level
        {
            //FName LevelPackageName = LevelModel->GetLongPackageName();
            auto Predicate = [&](ULevelStreaming* StreamingLevel) {
                return (StreamingLevel->GetWorldAssetPackageFName() == LevelPackageName && StreamingLevel->HasAnyFlags(
                    RF_Transient));
            };

            int32 Index = ThisWorld->GetStreamingLevels().IndexOfByPredicate(Predicate);
            if (Index != INDEX_NONE) {
                ThisWorld->GetStreamingLevels()[Index]->MarkPendingKill();
                ThisWorld->RemoveStreamingLevelAt(Index);
            }
        }

        if (Level->bIsVisible) {
            Level->OwningWorld->RemoveFromWorld(Level);
            check(Level->bIsVisible == false);
        }

        // Unload sub-level
        {
            UWorld* CurrentWorld = ThisWorld;
            EditorLevelUtils::RemoveLevelFromWorld(Level);
        }
    }


    //BroadcastPostLevelsUnloaded();

    GEditor->ResetTransaction(LOCTEXT("RemoveLevelTransReset", "Removing Levels from World"));

    // Collect garbage to clear out the destroyed level
    CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);

}

UObject* FDungeonEditorService::CloneAsset(UObject* SourceAsset, const FString& FolderPath) {
    FString ClonedAssetName = SourceAsset->GetName() + "_" + FGuid::NewGuid().ToString();
    FString ClonedAssetPackageName = FolderPath + "/" + ClonedAssetName;
    FAssetPackageInfo ClonedAssetInfo = FDungeonAssetUtils::DuplicateAsset(
        SourceAsset, ClonedAssetPackageName, ClonedAssetName);
    return ClonedAssetInfo.Asset;
}

void __PrivateMovesActorsToLevel(TArray<AActor*>& ActorsToMove, ULevel* DestLevel, int32& OutNumMovedActors) {
    // Backup the current contents of the clipboard string as we'll be using cut/paste features to move actors
    // between levels and this will trample over the clipboard data.
    FString OriginalClipboardContent;
    FPlatformApplicationMisc::ClipboardPaste(OriginalClipboardContent);

    check(DestLevel != NULL);

    // Cache a the destination world
    UWorld* World = DestLevel->OwningWorld;

    // Deselect all actors
    GEditor->Exec(World, TEXT("ACTOR SELECT NONE"));

    for (TArray<AActor*>::TIterator CurActorIt(ActorsToMove); CurActorIt; ++CurActorIt) {
        AActor* CurActor = *CurActorIt;
        if (!CurActor) continue;

        ULevelStreaming* ActorPrevLevel = FLevelUtils::FindStreamingLevel(CurActor->GetLevel());
        const FString& PrevLevelName = ActorPrevLevel != nullptr
                                           ? ActorPrevLevel->GetWorldAssetPackageName()
                                           : CurActor->GetLevel()->GetName();

        // Select this actor
        GEditor->SelectActor(CurActor, true, false, true);
        // Cache the world the actor is in, or if we already did ensure its the same world.
        check(World == CurActor->GetWorld());
    }

    OutNumMovedActors = 0;
    if (GEditor->GetSelectedActorCount() > 0) {
        // @todo: Perf: Not sure if this is needed here.
        GEditor->NoteSelectionChange();

        // Move the actors!
        UEditorLevelUtils::MoveSelectedActorsToLevel(DestLevel);

        // The moved (pasted) actors will now be selected
        OutNumMovedActors += GEditor->GetSelectedActorCount();
    }

    // Restore the original clipboard contents
    FPlatformApplicationMisc::ClipboardCopy(*OriginalClipboardContent);
}

void FDungeonEditorService::MoveActorsAcrossLevels(ULevel* SourceLevel, ULevel* TargetLevel,
                                                   const FTransform& LevelTransform, TArray<FGuid>& OutFoliageList) {

    int32 NumMovedActors = 0;
    //__PrivateMovesActorsToLevel(SourceLevel->Actors, TargetLevel, NumMovedActors);
    CopyFoliageToLevel(SourceLevel, TargetLevel, LevelTransform, OutFoliageList);

    /*
    TArray<AActor*> OriginalSelection;
    for (TActorIterator<AActor> ActorIt(TargetLevel->GetWorld()); ActorIt; ++ActorIt) {
        AActor* Actor = *ActorIt;
        if (Actor->IsSelected()) {
            OriginalSelection.Add(Actor);
        }
    }

    for (TActorIterator<AActor> ActorIt(TargetLevel->GetWorld()); ActorIt; ++ActorIt) {
        AActor* Actor = *ActorIt;
        GEditor->SelectActor(Actor, false, false);
    }

    for (AActor* Actor : SourceLevel->Actors) {
        GEditor->SelectActor(Actor, true, false, true, false);
    }


    GEditor->MoveSelectedActorsToLevel(TargetLevel);
    CopyFoliageToLevel(SourceLevel, TargetLevel, LevelTransform, OutFoliageList);

    for (AActor* Actor : OriginalSelection) {
        GEditor->SelectActor(Actor, true, true);
    }
    */

}


void FDungeonEditorService::RefreshEditorViews() {
    // refresh editor windows
    FEditorDelegates::RefreshAllBrowsers.Broadcast();

    // Update volume actor visibility for each viewport since we loaded a level which could potentially contain volumes
    GUnrealEd->UpdateVolumeActorVisibility(nullptr);

}

void FDungeonEditorService::SetActorEnabled(AActor* Actor, bool bEnabled) {
    bool bHidden = !bEnabled;
    Actor->bHiddenEd = bHidden;
    Actor->bHiddenEdLayer = bHidden;
    Actor->bHiddenEdLevel = bHidden;
    Actor->SetActorHiddenInGame(bHidden);
    Actor->SetIsTemporarilyHiddenInEditor(bHidden);
    Actor->SetActorTickEnabled(bEnabled);
    Actor->SetActorEnableCollision(bEnabled);
}

void FDungeonEditorService::DeleteAssets(const TArray<UObject*>& InObjectsToDelete, bool bShowConfirmation) {
    int32 NumObjectsDeleted = ObjectTools::DeleteObjects(InObjectsToDelete, bShowConfirmation);
    UE_LOG(LogEditorService, Log, TEXT("%d objects deleted"), NumObjectsDeleted);
}

void FDungeonEditorService::ForceDeleteAssets(const TArray<UObject*>& InObjectsToDelete, bool bShowConfirmation) {
    int32 NumObjectsDeleted = ObjectTools::ForceDeleteObjects(InObjectsToDelete, bShowConfirmation);
    UE_LOG(LogEditorService, Log, TEXT("%d objects force deleted"), NumObjectsDeleted);
}

void FDungeonEditorService::DestroyFoliage(ULevel* Level, const TArray<FGuid>& InFoliageList) {
    if (!Level) return;
    AInstancedFoliageActor* IFA = AInstancedFoliageActor::GetInstancedFoliageActorForLevel(
        Level, /*bCreateIfNone*/ false);
    if (!IFA) return;
    IFA->Modify();

    for (auto& MeshPair : IFA->FoliageInfos) {
        TUniqueObj<FFoliageInfo>& MeshInfo = MeshPair.Value;
        TArray<int32> SourceInstancesToRemove;
        for (int i = 0; i < MeshInfo->Instances.Num(); i++) {
            const FFoliageInstance& FoliageInstance = MeshInfo->Instances[i];
            if (InFoliageList.Contains(FoliageInstance.ProceduralGuid)) {
                SourceInstancesToRemove.Add(i);
            }
        }
        MeshInfo->RemoveInstances(IFA, SourceInstancesToRemove, true);
    }
}

void FDungeonEditorService::BuildMapGeometry(UWorld* World) {
    FEditorBuildUtils::EditorBuild(World, FBuildOptions::BuildVisibleGeometry);
}

void FDungeonEditorService::SaveDirtyPackages() {
    UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);
}

bool FDungeonEditorService::UnloadPackages(const TArray<UPackage*>& InPackagesToUnload) {
    FText UnloadErrorMessage;
    return UPackageTools::UnloadPackages(InPackagesToUnload, UnloadErrorMessage);
}

bool FDungeonEditorService::GetLevelViewportCameraInfo(UWorld* World, FVector& CameraLocation, FRotator& CameraRotation) {
    if (IDungeonEditorService::GetLevelViewportCameraInfo(World, CameraLocation, CameraRotation)) {
        return true;
    }
    
    bool RetVal = false;
    CameraLocation = FVector::ZeroVector;
    CameraRotation = FRotator::ZeroRotator;

    for (FLevelEditorViewportClient* LevelVC : GEditor->GetLevelViewportClients()) {
        if (LevelVC && LevelVC->IsPerspective() && LevelVC->GetWorld() == World) {
            CameraLocation = LevelVC->GetViewLocation();
            CameraRotation = LevelVC->GetViewRotation();
            RetVal = true;
            break;
        }
    }

    return RetVal;
}

ULevelStreaming* FDungeonEditorService::AddLevelToWorld(UWorld* InWorld, const TCHAR* LevelPackageName,
                                                        TSubclassOf<ULevelStreaming> LevelStreamingClass,
                                                        const FTransform& LevelTransform) {
    return EditorLevelUtils::AddLevelToWorld(InWorld, LevelPackageName, LevelStreamingClass, LevelTransform);
}

#undef LOCTEXT_NAMESPACE

