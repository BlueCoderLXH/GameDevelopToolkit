//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/DungeonBuilder.h"

#include "Builders/Grid/GridDungeonBuilder.h"
#include "Core/Actors/DungeonActorTemplate.h"
#include "Core/Dungeon.h"
#include "Core/DungeonEventListener.h"
#include "Core/DungeonQuery.h"
#include "Core/Utils/DungeonModelHelper.h"
#include "Core/Volumes/DungeonMarkerReplaceVolume.h"
#include "Core/Volumes/DungeonMirrorVolume.h"
#include "Core/Volumes/DungeonThemeOverrideVolume.h"
#include "Frameworks/ThemeEngine/DungeonThemeEngine.h"
#include "Frameworks/ThemeEngine/Markers/DungeonMarkerEmitter.h"
#include "Frameworks/ThemeEngine/SceneProviders/InstancedDungeonSceneProvider.h"
#include "Frameworks/ThemeEngine/SceneProviders/PooledDungeonSceneProvider.h"

#include "EngineUtils.h"
#include "Particles/ParticleSystem.h"
#include "PreviewScene.h"
#include "UObject/UObjectIterator.h"

DEFINE_LOG_CATEGORY(DungeonBuilderLog);


void CreatePropLookup(UDungeonThemeAsset* PropAsset, PropBySocketTypeByTheme_t& PropBySocketTypeByTheme) {
    if (!PropAsset || PropBySocketTypeByTheme.Contains(PropAsset)) {
        // Lookup for this theme has already been built
        return;
    }

    PropBySocketTypeByTheme.Add(PropAsset, PropBySocketType_t());
    PropBySocketType_t& PropBySocketType = PropBySocketTypeByTheme[PropAsset];

    for (const FPropTypeData& Prop : PropAsset->Props) {
        if (!PropBySocketType.Contains(Prop.AttachToSocket)) {
            PropBySocketType.Add(Prop.AttachToSocket, TArray<FPropTypeData>());
        }
        PropBySocketType[Prop.AttachToSocket].Add(Prop);
    }
}

// Picks a theme from the list that has a definition for the defined socket
UDungeonThemeAsset* GetBestMatchedTheme(const FRandomStream& random, const TArray<UDungeonThemeAsset*>& Themes,
                                        const FPropSocket& socket, PropBySocketTypeByTheme_t& PropBySocketTypeByTheme) {
    TArray<UDungeonThemeAsset*> ValidThemes;
    for (UDungeonThemeAsset* Theme : Themes) {
        if (PropBySocketTypeByTheme.Contains(Theme)) {
            PropBySocketType_t& PropBySocketType = PropBySocketTypeByTheme[Theme];
            if (PropBySocketType.Num() > 0) {
                if (PropBySocketType.Contains(socket.SocketType) && PropBySocketType[socket.SocketType].Num() > 0) {
                    ValidThemes.Add(Theme);
                }
            }
        }
    }
    if (ValidThemes.Num() == 0) {
        return nullptr;
    }

    int32 index = FMath::FloorToInt(random.FRand() * ValidThemes.Num()) % ValidThemes.Num();
    return ValidThemes[index];
}

struct ThemeOverrideInfo {
    FRectangle Bounds;
    ADungeonThemeOverrideVolume* Volume;
};

void GenerateThemeOverrideList(UWorld* World, ADungeon* Dungeon, PropBySocketTypeByTheme_t& PropBySocketTypeByTheme,
                               TArray<ThemeOverrideInfo>& OutOverrideList) {
    if (!World) return;

    OutOverrideList.Reset();
    if (World) {
        for (TActorIterator<ADungeonThemeOverrideVolume> VolumeIt(World); VolumeIt; ++VolumeIt) {
            ADungeonThemeOverrideVolume* Volume = *VolumeIt;
            if (Volume->IsPendingKill() || !Volume->IsValidLowLevel()) {
                continue;
            }
            bool valid;
            if (!Dungeon) {
                valid = true;
            }
            else {
                valid = (Volume->Dungeon == Dungeon);
            }
            if (valid && Volume->ThemeOverride) {
                FRectangle VolumeBounds;
                Volume->GetDungeonVolumeBounds(FVector(1, 1, 1), VolumeBounds);
                ThemeOverrideInfo Info;
                Info.Bounds = VolumeBounds;
                Info.Volume = Volume;
                OutOverrideList.Add(Info);

                // Build a lookup of the theme for faster access later on
                CreatePropLookup(Volume->ThemeOverride, PropBySocketTypeByTheme);
            }
        }
    }
}


void UDungeonBuilder::BuildDungeon(ADungeon* InDungeon, UWorld* World) {
    this->Dungeon = InDungeon;
    if (!Dungeon) {
        UE_LOG(DungeonBuilderLog, Log, TEXT("Cannot build dungeon due to invalid reference"));
        return;
    }

    BuildDungeon(Dungeon->GetModel(), Dungeon->GetConfig(), Dungeon->GetQuery(), World);
}

void UDungeonBuilder::BuildDungeon(UDungeonModel* InModel, UDungeonConfig* InConfig, UDungeonQuery* InQuery,
                                   UWorld* InWorld) {
    this->model = InModel;
    this->config = InConfig;
    this->query = InQuery;

    if (query && query->UserState) {
        query->UserState->ClearAllState();
    }

    _SocketIdCounter = 0;
    nrandom.Init(config->Seed);
    random.Initialize(config->Seed);

    BuildDungeonImpl(InWorld);
}

void UDungeonBuilder::BuildNonThemedDungeon(ADungeon* InDungeon, TSharedPtr<FDungeonSceneProvider> InSceneProvider,
                                            UWorld* World) {
    this->Dungeon = InDungeon;
    if (!Dungeon) {
        UE_LOG(DungeonBuilderLog, Log, TEXT("Cannot build dungeon due to invalid reference"));
        return;
    }

    BuildNonThemedDungeon(Dungeon->GetModel(), Dungeon->GetConfig(), Dungeon->GetQuery(), InSceneProvider, World);
}

void UDungeonBuilder::BuildNonThemedDungeon(UDungeonModel* InModel, UDungeonConfig* InConfig, UDungeonQuery* InQuery,
                                            TSharedPtr<FDungeonSceneProvider> InSceneProvider, UWorld* InWorld) {
    this->model = InModel;
    this->config = InConfig;
    this->query = InQuery;

    if (query && query->UserState) {
        query->UserState->ClearAllState();
    }

    _SocketIdCounter = 0;
    nrandom.Init(config->Seed);
    random.Initialize(config->Seed);

    BuildNonThemedDungeonImpl(InWorld, InSceneProvider);
}

void UDungeonBuilder::DestroyNonThemedDungeon(UDungeonModel* InModel, UDungeonConfig* InConfig, UDungeonQuery* InQuery,
                                              ADungeon* InDungeon, UWorld* InWorld) {
    this->model = InModel;
    this->config = InConfig;
    this->query = InQuery;
    this->Dungeon = InDungeon;

    DestroyNonThemedDungeonImpl(InWorld);
}

void UDungeonBuilder::ApplyDungeonTheme(const TArray<UDungeonThemeAsset*>& InThemes,
                                        const TArray<FClusterThemeInfo>& InClusteredThemes,
                                        TSharedPtr<FDungeonSceneProvider> InSceneProvider, UWorld* InWorld) {

    // Prepare the Theme Engine settings
    FDungeonThemeEngineSettings ThemeEngineSettings;
    ThemeEngineSettings.Themes = InThemes;
    ThemeEngineSettings.ClusteredThemes = InClusteredThemes;
    ThemeEngineSettings.SceneProvider = InSceneProvider;
    // Grab the theme override volumes
    if (InWorld) {
        for (TActorIterator<ADungeonThemeOverrideVolume> VolumeIt(InWorld); VolumeIt; ++VolumeIt) {
            ADungeonThemeOverrideVolume* ThemeOverrideVolume = *VolumeIt;
            if (ThemeOverrideVolume->IsPendingKill() || !ThemeOverrideVolume->IsValidLowLevel()) {
                continue;
            }
            const bool bValid = !Dungeon || (ThemeOverrideVolume->Dungeon == Dungeon);
            if (bValid && ThemeOverrideVolume->ThemeOverride) {
                ThemeEngineSettings.ThemeOverrideVolumes.Add(ThemeOverrideVolume);
            }
        }
    }

    // Prepare the Theme Engine callback handlers
    FDungeonThemeEngineEventHandlers EventHandlers;
    EventHandlers.PerformSelectionLogic = [this](const TArray<UDungeonSelectorLogic*>& SelectionLogics, const FPropSocket& socket) {
        return PerformSelectionLogic(SelectionLogics, socket);
    };
    
    EventHandlers.PerformTransformLogic = [this](const TArray<UDungeonTransformLogic*>& TransformLogics, const FPropSocket& socket) {
        return PerformTransformLogic(TransformLogics, socket);
    };

    EventHandlers.ProcessSpatialConstraint = [this](UDungeonSpatialConstraint* SpatialConstraint, const FTransform& Transform, FQuat& OutRotationOffset) {
        return ProcessSpatialConstraint(SpatialConstraint, Transform, OutRotationOffset);
    };

    EventHandlers.HandlePostMarkersEmit = [this](TArray<FDungeonMarkerInfo>& MarkersToEmit) {
        DungeonUtils::FDungeonEventListenerNotifier::NotifyMarkersEmitted(Dungeon, MarkersToEmit);
    };

    // Invoke the Theme Engine
    FDungeonThemeEngine::Apply(PropSockets, random, ThemeEngineSettings, EventHandlers);
}


void UDungeonBuilder::MirrorDungeon() {
    if (Dungeon) {
        for (TObjectIterator<ADungeonMirrorVolume> Volume; Volume; ++Volume) {
            if (!Volume || Volume->IsPendingKill() || !Volume->IsValidLowLevel()) {
                continue;
            }
            if (Volume->Dungeon == Dungeon) {
                // Build a lookup of the theme for faster access later on
                MirrorDungeonWithVolume(*Volume);
            }
        }
    }
}

TSharedPtr<FDungeonSceneProvider> UDungeonBuilder::CreateSceneProvider(UDungeonConfig* pConfig, ADungeon* pDungeon,
                                                                       UWorld* World) {
    if (!pConfig) {
        UE_LOG(DungeonBuilderLog, Error, TEXT("Invalid config reference"));
        return nullptr;
    }

    TSharedPtr<FDungeonSceneProvider> SceneProvider;
    if (pConfig->Instanced) {
        SceneProvider = MakeShareable(new FInstancedDungeonSceneProvider(pDungeon, World));
    }
    else {
        SceneProvider = MakeShareable(new FPooledDungeonSceneProvider(pDungeon, World));
    }

    return SceneProvider;
}

bool UDungeonBuilder::ProcessSpatialConstraint(UDungeonSpatialConstraint* SpatialConstraint,
                                               const FTransform& Transform, FQuat& OutRotationOffset) {
    return true;
}

void UDungeonBuilder::AddMarker(const FString& SocketType, const FTransform& _transform, int count,
                                const FVector& InterOffset, TSharedPtr<class IDungeonMarkerUserData> InUserData) {
    FTransform transform = _transform;
    FVector Location = transform.GetLocation();
    for (int i = 0; i < count; i++) {
        AddMarker(SocketType, transform, InUserData);
        Location += InterOffset;
        transform.SetLocation(Location);
    }
}

void UDungeonBuilder::AddMarker(const FString& SocketType, const FTransform& transform,
                                TSharedPtr<class IDungeonMarkerUserData> InUserData) {
    FPropSocket socket;
    socket.Id = ++_SocketIdCounter;
    socket.IsConsumed = false;
    socket.SocketType = SocketType;
    socket.Transform = transform;
    socket.UserData = InUserData;
    PropSockets.Add(socket);
}

void UDungeonBuilder::AddMarker(TArray<FPropSocket>& pPropSockets, const FString& SocketType,
                                const FTransform& transform, TSharedPtr<class IDungeonMarkerUserData> InUserData) {
    FPropSocket socket;
    socket.Id = ++_SocketIdCounter;
    socket.IsConsumed = false;
    socket.SocketType = SocketType;
    socket.Transform = transform;
    socket.UserData = InUserData;
    pPropSockets.Add(socket);
}

void UDungeonBuilder::EmitDungeonMarkers_Implementation() {
    random.Initialize(config->Seed);
}

void UDungeonBuilder::EmitMarker(const FString& SocketType, const FTransform& Transform) {
    AddMarker(SocketType, Transform);
}

void UDungeonBuilder::EmitCustomMarkers(TArray<UDungeonMarkerEmitter*> MarkerEmitters) {
    /*
    UDungeonQuery* Query = nullptr;
    if (Dungeon) {
        Query = Dungeon->GetQuery();
    }
    else {
        // The theme editor does not have a dungeon actor. Create a new query object in that case
        Query = NewObject<UDungeonQuery>(this, GetQueryClass());
        Query->Initialize(config, GetModel(), FTransform::Identity);
    }
    */

    for (UDungeonMarkerEmitter* MarkerEmitter : MarkerEmitters) {
        if (MarkerEmitter) {
            MarkerEmitter->EmitMarkers(this, GetModel(), config, query);
        }
    }
}

UClass* UDungeonBuilder::DefaultBuilderClass() {
    return UGridDungeonBuilder::StaticClass();
}

void UDungeonBuilder::ProcessMarkerReplacementVolumes() {
    UWorld* World = Dungeon ? Dungeon->GetWorld() : nullptr;
    if (World) {
        for (TActorIterator<ADungeonMarkerReplaceVolume> VolumeIt(World); VolumeIt; ++VolumeIt) {
            ADungeonMarkerReplaceVolume* Volume = *VolumeIt;
            if (Volume && Volume->Dungeon == Dungeon) {
                ProcessMarkerReplacementVolume(Volume);
            }
        }
    }
}

void UDungeonBuilder::ProcessMarkerReplacementVolume(class ADungeonMarkerReplaceVolume* MarkerReplaceVolume) {
    if (!MarkerReplaceVolume) return;
    for (FPropSocket& Socket : PropSockets) {
        if (MarkerReplaceVolume->EncompassesPoint(Socket.Transform.GetLocation())) {
            for (const FMarkerReplaceEntry& Entry : MarkerReplaceVolume->Replacements) {
                if (Socket.SocketType == Entry.MarkerName) {
                    Socket.SocketType = Entry.ReplacementName;
                }
            }
        }
    }
}

void UDungeonBuilder::ProcessThemeItemUserData(TSharedPtr<IDungeonMarkerUserData> UserData, AActor* SpawnedActor) {
}

