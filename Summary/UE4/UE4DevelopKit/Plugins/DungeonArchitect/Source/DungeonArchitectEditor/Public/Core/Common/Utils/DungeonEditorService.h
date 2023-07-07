//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Utils/EditorService/IDungeonEditorService.h"

#include "IAssetTools.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEditorService, Log, All)

/** Provides editor services to the runtime module */
class DUNGEONARCHITECTEDITOR_API FDungeonEditorService : public IDungeonEditorService {
public:
    virtual AActor* CreateGroupActor(UWorld* World, const TArray<AActor*>& MergedActorList,
                             const FTransform& Transform) override;

    virtual UObject* CloneAsset(UObject* SourceAsset, const FString& FolderPath) override;

    virtual void MoveActorsAcrossLevels(ULevel* SourceLevel, ULevel* TargetLevel, const FTransform& LevelTransform,
                                TArray<FGuid>& OutFoliageList) override;
    virtual void RefreshEditorViews() override;
    virtual void SetActorEnabled(AActor* Actor, bool bEnabled) override;
    virtual void DeleteAssets(const TArray<UObject*>& InObjectsToDelete, bool bShowConfirmation) override;
    virtual void ForceDeleteAssets(const TArray<UObject*>& InObjectsToDelete, bool bShowConfirmation) override;
    virtual void DestroyFoliage(ULevel* Level, const TArray<FGuid>& InFoliageList) override;
    virtual void BuildMapGeometry(UWorld* World) override;
    virtual void SaveDirtyPackages() override;

    virtual bool UnloadPackages(const TArray<UPackage*>& InPackagesToUnload) override;
    virtual bool GetLevelViewportCameraInfo(UWorld* World, FVector& CameraLocation, FRotator& CameraRotation) override;
    
    virtual ULevelStreaming* AddLevelToWorld(UWorld* InWorld, const TCHAR* LevelPackageName,
                                     TSubclassOf<ULevelStreaming> LevelStreamingClass,
                                     const FTransform& LevelTransform) override;


public:
    static FString GetAssetPackagePath(UObject* AssetObject) {
        return AssetObject->GetOutermost()->GetName();
    }

    /*
    template<typename T>
    static T* CloneAsset(const FString& TemplatePath, const FString& TargetName, const FString& TargetDirectory) {
        T* Template = LoadObject<T>(NULL, *TemplatePath, NULL, LOAD_None, NULL);
        IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
        UObject* AssetObject = AssetTools.DuplicateAsset(TargetName, TargetDirectory, Template);
        T* Asset = Cast<T>(AssetObject);
        if (!Asset) {
            UE_LOG(LogEditorService, Warning, TEXT("Failed to clone asset at location %s/%s"), *TargetDirectory, *TargetName);
        }
        return Asset;
    }
    */
};

