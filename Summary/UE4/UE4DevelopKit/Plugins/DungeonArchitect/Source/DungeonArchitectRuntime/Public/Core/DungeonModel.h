//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "DungeonModel.generated.h"

class UDungeonSpawnLogic;

class DUNGEONARCHITECTRUNTIME_API IDungeonMarkerUserData {
};

USTRUCT(Blueprintable)
struct DUNGEONARCHITECTRUNTIME_API FDungeonMarkerInfo {
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Dungeon)
    FTransform transform;

    UPROPERTY()
    FName NodeId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    TArray<UDungeonSpawnLogic*> SpawnLogics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    UObject* TemplateObject;

    TSharedPtr<IDungeonMarkerUserData> UserData;

    FORCEINLINE bool operator==(const FDungeonMarkerInfo& other) const {
        return other.NodeId == NodeId;
    }
};

/**
* 
*/
UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API UDungeonModel : public UObject {
    GENERATED_BODY()

public:
    virtual void Cleanup() {}
    virtual void Reset() {}
    virtual bool ShouldAutoResetOnBuild() const { return true; }
    virtual void GenerateLayoutData(class FDungeonLayoutData& OutLayout) {}
};

