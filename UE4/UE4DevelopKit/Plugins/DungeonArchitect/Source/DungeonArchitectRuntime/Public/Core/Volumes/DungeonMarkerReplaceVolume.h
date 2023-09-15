//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Dungeon.h"
#include "Core/Volumes/DungeonVolume.h"
#include "DungeonMarkerReplaceVolume.generated.h"

USTRUCT(BlueprintType)
struct DUNGEONARCHITECTRUNTIME_API FMarkerReplaceEntry {
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = Dungeon)
    FString MarkerName;

    UPROPERTY(EditAnywhere, Category = Dungeon)
    FString ReplacementName;
};

/**
*
*/
UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API ADungeonMarkerReplaceVolume : public ADungeonVolume {
    GENERATED_BODY()

public:
    ADungeonMarkerReplaceVolume(const FObjectInitializer& ObjectInitializer);

public:
    /** Indicates if the negation should be reversed, i.e. everything outside this volume is negated */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    TArray<FMarkerReplaceEntry> Replacements;

};

