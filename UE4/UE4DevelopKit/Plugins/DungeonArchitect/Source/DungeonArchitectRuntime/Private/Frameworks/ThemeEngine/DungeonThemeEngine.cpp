//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/ThemeEngine/DungeonThemeEngine.h"

#include "Core/Actors/DungeonActorTemplate.h"
#include "Core/DungeonProp.h"
#include "Core/Utils/DungeonModelHelper.h"
#include "Core/Utils/Rectangle.h"
#include "Core/Volumes/DungeonThemeOverrideVolume.h"
#include "Frameworks/ThemeEngine/DungeonThemeAsset.h"
#include "Frameworks/ThemeEngine/SceneProviders/DungeonSceneProvider.h"
#include "Frameworks/ThemeEngine/SceneProviders/DungeonSceneProviderContext.h"

#include "Particles/ParticleSystem.h"

typedef TMap<FString, TArray<FPropTypeData>> PropBySocketType_t;
typedef TMap<UDungeonThemeAsset*, PropBySocketType_t> PropBySocketTypeByTheme_t;

class FDAThemeEngineImpl {
public:
    static void CreatePropLookup(UDungeonThemeAsset* PropAsset, PropBySocketTypeByTheme_t& PropBySocketTypeByTheme) {
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
    static UDungeonThemeAsset* GetBestMatchedTheme(const FRandomStream& random, const TArray<UDungeonThemeAsset*>& Themes,
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

        const int32 Index = FMath::FloorToInt(random.FRand() * ValidThemes.Num()) % ValidThemes.Num();
        return ValidThemes[Index];
    }

    static void AddMarker(TArray<FPropSocket>& Markers, const FString& InMarkerName, const FTransform& InTransform,
            TSharedPtr<class IDungeonMarkerUserData> InUserData = nullptr) {
        FPropSocket socket;
        socket.Id = Markers.Num();
        socket.IsConsumed = false;
        socket.SocketType = InMarkerName;
        socket.Transform = InTransform;
        socket.UserData = InUserData;
        Markers.Add(socket);
    }
    
};

void FDungeonThemeEngine::Apply(TArray<FPropSocket>& InOutMarkers, const FRandomStream& InRandom,
        const FDungeonThemeEngineSettings& InSettings, const FDungeonThemeEngineEventHandlers& EventHandlers) {
    PropBySocketTypeByTheme_t PropBySocketTypeByTheme;
    for (UDungeonThemeAsset* Theme : InSettings.Themes) {
        FDAThemeEngineImpl::CreatePropLookup(Theme, PropBySocketTypeByTheme);
    }

    // Process the Theme Overrides
    TMap<ADungeonThemeOverrideVolume*, FRectangle> ThemeOverrideBounds;
    for (ADungeonThemeOverrideVolume* OverrideVolume : InSettings.ThemeOverrideVolumes) {
        FRectangle VolumeBounds;
        OverrideVolume->GetDungeonVolumeBounds(FVector(1, 1, 1), VolumeBounds);
        ThemeOverrideBounds.Add(OverrideVolume, VolumeBounds);

        // Build a lookup of the theme for faster access later on
        FDAThemeEngineImpl::CreatePropLookup(OverrideVolume->ThemeOverride, PropBySocketTypeByTheme);
    }

    TMap<FString, FClusterThemeInfo> ClusteredThemeByName;
    for (const FClusterThemeInfo& ClusteredThemeInfo : InSettings.ClusteredThemes) {
        if (!ClusteredThemeByName.Contains(ClusteredThemeInfo.ClusterThemeName)) {
            ClusteredThemeByName.Add(ClusteredThemeInfo.ClusterThemeName, ClusteredThemeInfo);
        }

        for (UDungeonThemeAsset* Theme : ClusteredThemeInfo.Themes) {
            if (!PropBySocketTypeByTheme.Contains(Theme)) {
                FDAThemeEngineImpl::CreatePropLookup(Theme, PropBySocketTypeByTheme);
            }
        }
    }

    TArray<FDungeonMarkerInfo> MarkersToEmit;

    // Fill up the prop sockets with the defined mesh data 
    for (int32 i = 0; i < InOutMarkers.Num(); i++) {
        const FPropSocket& ThemeItem = InOutMarkers[i];

        UDungeonThemeAsset* ThemeToUse = nullptr;

        // User the overridden theme if specified
        if (!ThemeItem.ClusterThemeOverride.IsEmpty() && ClusteredThemeByName.Contains(ThemeItem.ClusterThemeOverride)) {
            ThemeToUse = FDAThemeEngineImpl::GetBestMatchedTheme(InRandom, ClusteredThemeByName[ThemeItem.ClusterThemeOverride].Themes, ThemeItem,
                                             PropBySocketTypeByTheme); // PropAsset;
        }

        if (!ThemeToUse) {
            // use the default theme list
            ThemeToUse = FDAThemeEngineImpl::GetBestMatchedTheme(InRandom, InSettings.Themes, ThemeItem, PropBySocketTypeByTheme); // PropAsset;
        }

        UDungeonThemeAsset* FallbackTheme = ThemeToUse;

        ADungeonThemeOverrideVolume* BestOverrideVolume = nullptr;
        float BestOverrideWeight = 0;

        // Check if this socket resides within a override volume
        for (auto& Entry : ThemeOverrideBounds) {
            ADungeonThemeOverrideVolume* ThemeOverrideVolume = Entry.Key;
            FRectangle OverrideBounds = Entry.Value;
            
            FVector Location = ThemeItem.Transform.GetLocation();
            if (!ThemeOverrideVolume) continue;

            FIntVector ILocation(Location.X, Location.Y, Location.Z);
            bool bIntersects = OverrideBounds.Contains(ILocation);
            if (ThemeOverrideVolume->Reversed) {
                bIntersects = !bIntersects;
            }

            if (bIntersects) {
                if (!BestOverrideVolume || BestOverrideWeight < ThemeOverrideVolume->OverrideWeight) {
                    BestOverrideVolume = ThemeOverrideVolume;
                    BestOverrideWeight = ThemeOverrideVolume->OverrideWeight;
                }
            }
        }

        if (BestOverrideVolume) {
            ThemeToUse = BestOverrideVolume->ThemeOverride;
            if (!BestOverrideVolume->FallbackOnMissingMarkers) {
                // We do not want a fallback. So use this same theme as a fallback
                FallbackTheme = ThemeToUse;
            }
        }

        if (!ThemeToUse) continue;

        check(PropBySocketTypeByTheme.Contains(ThemeToUse));
        PropBySocketType_t* PropBySocketTypePtr = &PropBySocketTypeByTheme[ThemeToUse];
        if (FallbackTheme != ThemeToUse && FallbackTheme != nullptr && !PropBySocketTypePtr->Contains(ThemeItem.SocketType)
        ) {
            // The theme we are about to use doesn't have any nodes attached to this marker.
            // Check if we can use a fallback theme
            PropBySocketType_t* FallbackPropBySocketTypePtr = &PropBySocketTypeByTheme[FallbackTheme];
            if (FallbackPropBySocketTypePtr->Contains(ThemeItem.SocketType)) {
                PropBySocketTypePtr = FallbackPropBySocketTypePtr;
            }
        }

        PropBySocketType_t& PropBySocketType = *PropBySocketTypePtr;

        if (PropBySocketType.Contains(ThemeItem.SocketType)) {
            const TArray<FPropTypeData>& Props = PropBySocketType[ThemeItem.SocketType];
            for (const FPropTypeData& Prop : Props) {
                bool bInsertMesh = false;
                if (Prop.bUseSelectionLogic) {
                    bInsertMesh = EventHandlers.PerformSelectionLogic(Prop.SelectionLogics, ThemeItem);

                    if (bInsertMesh && !Prop.bLogicOverridesAffinity) {
                        // The logic has selected the mesh and it doesn't override the affinity.
                        // Respect the affinity variable and apply probability
                        float probability = InRandom.FRand();
                        bInsertMesh = (probability < Prop.Probability);
                    }
                }
                else {
                    // Perform probability based selection logic
                    float probability = InRandom.FRand();
                    bInsertMesh = (probability < Prop.Probability);
                }

                FQuat spatialRotationOffset = FQuat::Identity;

                // Check if we are using spatial constraints
                if (Prop.bUseSpatialConstraint) {

                    bool bPassesSpatialConstraint = EventHandlers.ProcessSpatialConstraint(Prop.SpatialConstraint, ThemeItem.Transform, spatialRotationOffset);
                    if (!bPassesSpatialConstraint) {
                        bInsertMesh = false;
                    }
                }

                if (bInsertMesh) {
                    // Attach this prop to the socket
                    FTransform Transform = ThemeItem.Transform;

                    // Apply the spatial rotation offset
                    if (Prop.bUseSpatialConstraint && Prop.SpatialConstraint) {
                        if (!Prop.SpatialConstraint->bApplyBaseRotation) {
                            Transform.SetRotation(FQuat::Identity);
                        }
                        FTransform spatialRotationTransform = FTransform::Identity;
                        spatialRotationTransform.SetRotation(spatialRotationOffset);
                        FTransform OutTempTransform;
                        FTransform::Multiply(&OutTempTransform, &spatialRotationTransform, &Transform);
                        Transform = OutTempTransform;
                    }

                    {
                        FTransform OutTempTransform;
                        FTransform::Multiply(&OutTempTransform, &Prop.Offset, &Transform);
                        Transform = OutTempTransform;
                    }


                    // Apply transform logic, if specified
                    if (Prop.bUseTransformLogic) {
                        FTransform LogicOffset = EventHandlers.PerformTransformLogic(Prop.TransformLogics, ThemeItem);
                        FTransform OutTempTransform;
                        FTransform::Multiply(&OutTempTransform, &LogicOffset, &Transform);
                        Transform = OutTempTransform;
                    }

                    TArray<UDungeonSpawnLogic*> SpawnLogics;
                    if (Prop.bUseSpawnLogic && Prop.SpawnLogics.Num() > 0) {
                        TArray<UDungeonSpawnLogic*> SpawnLogicsSource = Prop.bUseSpawnLogic ? Prop.SpawnLogics : TArray<UDungeonSpawnLogic*>();
                        UObject* SpawnLogicArrayOuter = GetTransientPackage();
                        UDungeonModelHelper::CloneUObjectArray(SpawnLogicArrayOuter, SpawnLogicsSource, SpawnLogics);
                    }

                    if (Prop.AssetObject) {
                        FDungeonMarkerInfo MarkerInfo;
                        MarkerInfo.transform = Transform;
                        MarkerInfo.NodeId = Prop.NodeId;
                        MarkerInfo.SpawnLogics = SpawnLogics;
                        MarkerInfo.TemplateObject = Prop.AssetObject;
                        MarkerInfo.UserData = ThemeItem.UserData;
                        MarkersToEmit.Add(MarkerInfo);
                    }

                    // Add child sockets if any
                    for (const FPropChildSocketData& ChildSocket : Prop.ChildSockets) {
                        FTransform childTransform;
                        FTransform::Multiply(&childTransform, &ChildSocket.Offset, &Transform);
                        FDAThemeEngineImpl::AddMarker(InOutMarkers, ChildSocket.SocketType, childTransform);

                        // Sync the user data
                        if (InOutMarkers.Num() > 0) {
                            FPropSocket& NewSocket = InOutMarkers[InOutMarkers.Num() - 1];
                            NewSocket.ClusterThemeOverride = ThemeItem.ClusterThemeOverride;
                            NewSocket.UserData = ThemeItem.UserData;
                        }
                    }

                    if (Prop.ConsumeOnAttach) {
                        // Attach no more on this socket
                        break;
                    }
                }
            }
        }
    }

    EventHandlers.HandlePostMarkersEmit(MarkersToEmit);

    // Create the scene build commands based on the markers emitted on the scene
    InSettings.SceneProvider->OnDungeonBuildStart();

    for (const FDungeonMarkerInfo& MarkerInfo : MarkersToEmit) {
        FDungeonSceneProviderContext Context;
        Context.transform = MarkerInfo.transform;
        Context.NodeId = MarkerInfo.NodeId;
        Context.SpawnLogics = MarkerInfo.SpawnLogics;
        Context.MarkerUserData = MarkerInfo.UserData;

        // Add a mesh instance, if specified
        AActor* SpawnedActor = nullptr;
        if (UDungeonMesh* Mesh = Cast<UDungeonMesh>(MarkerInfo.TemplateObject)) {
            InSettings.SceneProvider->AddStaticMesh(Mesh, Context);
        }
        else if (UPointLightComponent* Light = Cast<UPointLightComponent>(MarkerInfo.TemplateObject)) {
            InSettings.SceneProvider->AddLight(Light, Context);
        }
        else if (UParticleSystem* Particle = Cast<UParticleSystem>(MarkerInfo.TemplateObject)) {
            InSettings.SceneProvider->AddParticleSystem(Particle, Context);
        }
        else if (UClass* ClassTemplate = Cast<UClass>(MarkerInfo.TemplateObject)) {
            InSettings.SceneProvider->AddActorFromTemplate(ClassTemplate, Context);
        }
        else if (UDungeonActorTemplate* ActorTemplate = Cast<UDungeonActorTemplate>(MarkerInfo.TemplateObject)) {
            InSettings.SceneProvider->AddClonedActor(ActorTemplate, Context);
        }
        else {
            // Not supported.  Give the implementation an opportunity to handle it
            InSettings.SceneProvider->ProcessUnsupportedObject(MarkerInfo.TemplateObject, Context);
        }
    }

    InSettings.SceneProvider->OnDungeonBuildStop();
}

