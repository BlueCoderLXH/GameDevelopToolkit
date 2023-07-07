//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

class ULevelStreaming;
class FConnectionDrawingPolicy;
class UEdGraph;
class UPackage;

struct DUNGEONARCHITECTRUNTIME_API FStreamingLevelInstanceInfo {
    FStreamingLevelInstanceInfo() : LevelStreaming(nullptr) {
    }

    ULevelStreaming* LevelStreaming;
    UWorld* MapPackage;
    FName MapPackageName;
};

/** This provides a way for the runtime module to communicate with the editor module (if it exists) without adding a reference to it */
class DUNGEONARCHITECTRUNTIME_API IDungeonEditorService {
public:
    virtual ~IDungeonEditorService() {
    }

    virtual AActor* CreateGroupActor(UWorld* World, const TArray<AActor*>& MergedActorList,
                                     const FTransform& Transform) = 0;
    virtual void RefreshEditorViews() = 0;
    virtual void SetActorEnabled(AActor* Actor, bool bEnabled) = 0;
    virtual UObject* CloneAsset(UObject* SourceAsset, const FString& FolderPath) = 0;

    virtual void DeleteAssets(const TArray<UObject*>& InObjectsToDelete, bool bShowConfirmation) {
    }

    virtual void ForceDeleteAssets(const TArray<UObject*>& InObjectsToDelete, bool bShowConfirmation) {
    }

    virtual void MoveActorsAcrossLevels(ULevel* SourceLevel, ULevel* TargetLevel, const FTransform& LevelTransform,
                                        TArray<FGuid>& OutFoliageList) {
    }

    virtual void DestroyFoliage(ULevel* Level, const TArray<FGuid>& InFoliageList) {
    }

    virtual void BuildMapGeometry(UWorld* World) {
    }

    virtual void SaveDirtyPackages() {
    }

    virtual bool UnloadPackages(const TArray<UPackage*>& InPackagesToUnload) { return false; }
    virtual bool GetLevelViewportCameraInfo(UWorld* World, FVector& CameraLocation, FRotator& CameraRotation);

    virtual ULevelStreaming* AddLevelToWorld(UWorld* InWorld, const TCHAR* LevelPackageName,
                                             TSubclassOf<ULevelStreaming> LevelStreamingClass,
                                             const FTransform& LevelTransform) = 0;

    static TSharedPtr<IDungeonEditorService> Get();
    static void Set(TSharedPtr<IDungeonEditorService> InInstance);
private:
    static TSharedPtr<IDungeonEditorService> Instance;
};

