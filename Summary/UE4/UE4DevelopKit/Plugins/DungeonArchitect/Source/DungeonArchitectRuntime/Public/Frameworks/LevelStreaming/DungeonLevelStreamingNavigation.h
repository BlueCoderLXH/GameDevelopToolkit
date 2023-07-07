//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "DungeonLevelStreamingNavigation.generated.h"

UCLASS()
class DUNGEONARCHITECTRUNTIME_API UDungeonLevelStreamingNavigation : public UObject {
    GENERATED_BODY()
public:
    UPROPERTY()
    bool bEnabled = false;

    UPROPERTY()
    bool bAutoResizeNavVolume = false;

public:
    void Initialize(UWorld* InWorld);
    void Release();

    void AddLevelNavigation(ULevel* InLevel, const FBox& ChunkBounds);
    void RemoveLevelNavigation(ULevel* InLevel);

private:
    TWeakObjectPtr<UWorld> OwningWorld;
};

