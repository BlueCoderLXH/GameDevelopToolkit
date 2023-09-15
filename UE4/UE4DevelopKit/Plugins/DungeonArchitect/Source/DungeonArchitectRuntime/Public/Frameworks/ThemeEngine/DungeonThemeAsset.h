//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/DungeonProp.h"
#include "Core/Utils/DungeonEditorViewportProperties.h"
#include "DungeonThemeAsset.generated.h"

/** A Dungeon Theme asset lets you design the look and feel of you dungeon with an intuitive graph based approach */
UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API UDungeonThemeAsset : public UObject {
    GENERATED_UCLASS_BODY()

public:

    UPROPERTY()
    TArray<FPropTypeData> Props;

#if WITH_EDITORONLY_DATA
    /** EdGraph based representation */
    UPROPERTY()
    class UEdGraph* UpdateGraph;
#endif // WITH_EDITORONLY_DATA

    UPROPERTY()
    UDungeonEditorViewportProperties* PreviewViewportProperties;

    static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);
};


/**
A dungeon builder can cluster different parts of the dungeon into groups where different themes can be applied to each group
This registry contains the theme registration for each group
*/
USTRUCT(Blueprintable, BlueprintType)
struct DUNGEONARCHITECTRUNTIME_API FClusterThemeInfo {
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    FString ClusterThemeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
    TArray<class UDungeonThemeAsset*> Themes;
};

