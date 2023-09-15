//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/LevelStreaming/DungeonLevelStreamingNavigation.h"

#include "NavMesh/NavMeshBoundsVolume.h"
#include "NavigationSystem.h"

void UDungeonLevelStreamingNavigation::Initialize(UWorld* InWorld) {
    OwningWorld = InWorld;
}

void UDungeonLevelStreamingNavigation::Release() {
    OwningWorld = nullptr;
}

void UDungeonLevelStreamingNavigation::AddLevelNavigation(ULevel* InLevel, const FBox& ChunkBounds) {
    if (!bEnabled) return;

    UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(OwningWorld.Get());
    if (NavSystem) {
        ANavMeshBoundsVolume* NavVolume = nullptr;
        for (AActor* Actor : InLevel->Actors) {
            if (ANavMeshBoundsVolume* CastNavVolume = Cast<ANavMeshBoundsVolume>(Actor)) {
                NavVolume = CastNavVolume;
                break;
            }
        }

        if (NavVolume && bAutoResizeNavVolume) {
            NavVolume->GetRootComponent()->Mobility = EComponentMobility::Movable;
            NavVolume->SetActorLocation(ChunkBounds.GetCenter());
            FVector Scale = ChunkBounds.GetExtent() * 2.0f / 200.0f;
            NavVolume->SetActorScale3D(Scale);
            NavVolume->SetActorRotation(FQuat::Identity);
        }

        if (NavVolume) {
            NavSystem->OnNavigationBoundsUpdated(NavVolume);
        }

        NavSystem->Build();
    }
}

void UDungeonLevelStreamingNavigation::RemoveLevelNavigation(ULevel* InLevel) {
    if (!bEnabled) return;

    if (InLevel) {
        UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(OwningWorld.Get());
        if (NavSystem) {
            for (AActor* Actor : InLevel->Actors) {
                if (ANavMeshBoundsVolume* NavVolume = Cast<ANavMeshBoundsVolume>(Actor)) {
                    NavSystem->OnNavigationBoundsRemoved(NavVolume);
                }
            }

            NavSystem->Build();
        }
    }
}

