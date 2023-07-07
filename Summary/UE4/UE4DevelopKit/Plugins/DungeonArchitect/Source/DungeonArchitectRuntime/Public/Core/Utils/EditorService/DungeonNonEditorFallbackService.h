//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Utils/EditorService/IDungeonEditorService.h"

/** This is a fallback module that would be registered if the editor module is unavailable (e.g. standalone builds) */
class DUNGEONARCHITECTRUNTIME_API FDungeonNonEditorFallbackService : public IDungeonEditorService {
public:
    virtual AActor* CreateGroupActor(UWorld* World, const TArray<AActor*>& MergedActorList,
                                     const FTransform& Transform) override;
    virtual UObject* CloneAsset(UObject* SourceAsset, const FString& FolderPath) override;
    virtual void SetActorEnabled(AActor* Actor, bool bEnabled) override;

    virtual void RefreshEditorViews() override {
    }

    virtual ULevelStreaming* AddLevelToWorld(UWorld* InWorld, const TCHAR* LevelPackageName,
                                             TSubclassOf<ULevelStreaming> LevelStreamingClass,
                                             const FTransform& LevelTransform) override;
};

