//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/ThemeEngine/DungeonThemeAsset.h"

class FDungeonSceneProvider;
struct FDungeonMarkerInfo;
struct FPropSocket;
class ADungeonThemeOverrideVolume;
class UDungeonSelectorLogic;
class UDungeonTransformLogic;
class UDungeonSpatialConstraint;

struct FDungeonThemeEngineSettings {
    TArray<UDungeonThemeAsset*> Themes;
    TArray<FClusterThemeInfo> ClusteredThemes;
    TArray<ADungeonThemeOverrideVolume*> ThemeOverrideVolumes;
    TSharedPtr<FDungeonSceneProvider> SceneProvider;
};

struct FDungeonThemeEngineEventHandlers {
    TFunction<bool(const TArray<UDungeonSelectorLogic*>&, const FPropSocket&)> PerformSelectionLogic
            = [](const TArray<UDungeonSelectorLogic*>&, const FPropSocket&){ return false; };

    TFunction<FTransform(const TArray<UDungeonTransformLogic*>&, const FPropSocket&)> PerformTransformLogic
            = [](const TArray<UDungeonTransformLogic*>&, const FPropSocket&) { return FTransform::Identity; };

    TFunction<bool(UDungeonSpatialConstraint*, const FTransform&, FQuat&)> ProcessSpatialConstraint
            = [](UDungeonSpatialConstraint*, const FTransform&, FQuat&) { return true; };

    TFunction<void(TArray<FDungeonMarkerInfo>&)> HandlePostMarkersEmit
            = [](TArray<FDungeonMarkerInfo>&) {};
};

class DUNGEONARCHITECTRUNTIME_API FDungeonThemeEngine {
public:
    static void Apply(TArray<FPropSocket>& InOutMarkers, const FRandomStream& InRandom,
                const FDungeonThemeEngineSettings& InSettings, const FDungeonThemeEngineEventHandlers& EventHandlers);

};

