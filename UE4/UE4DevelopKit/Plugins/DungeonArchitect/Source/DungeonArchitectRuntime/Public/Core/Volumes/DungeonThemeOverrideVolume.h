//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Volumes/DungeonVolume.h"
#include "Frameworks/ThemeEngine/DungeonThemeAsset.h"
#include "DungeonThemeOverrideVolume.generated.h"

/**
*
*/
UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API ADungeonThemeOverrideVolume : public ADungeonVolume {
    GENERATED_BODY()

public:
    ADungeonThemeOverrideVolume(const FObjectInitializer& ObjectInitializer);

public:
    /** Indicates if the bounds of this volume should be reversed, i.e. everything outside this volume is overridden */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    bool Reversed;

    /** If a marker has no meshes in the overridden theme, it will fallback to the base theme if this flag is checked */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    bool FallbackOnMissingMarkers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    UDungeonThemeAsset* ThemeOverride;

    /** If multiple override volumes overlap, the volume with the highest weight takes precedence */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    float OverrideWeight = 0;
};

