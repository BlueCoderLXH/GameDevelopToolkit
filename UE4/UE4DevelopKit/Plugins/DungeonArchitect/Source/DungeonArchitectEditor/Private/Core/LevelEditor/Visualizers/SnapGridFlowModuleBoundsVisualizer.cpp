//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/LevelEditor/Visualizers/SnapGridFlowModuleBoundsVisualizer.h"

#include "Frameworks/Snap/SnapGridFlow/SnapGridFlowModuleBounds.h"

void FSnapGridFlowModuleBoundsVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) {
    const USnapGridFlowModuleBoundsComponent* BoundsComponent = Cast<USnapGridFlowModuleBoundsComponent>(Component);
    if (!BoundsComponent) return;

    const AActor* BoundsActor = BoundsComponent->GetOwner();
    if (!BoundsActor) return;
    
    const USnapGridFlowModuleBoundsAsset* BoundsAsset = BoundsComponent->ModuleBounds.LoadSynchronous();
    if (!BoundsAsset) return;

    FIntVector ChunkSize = BoundsComponent->NumChunks;

    ChunkSize.X = FMath::Clamp(ChunkSize.X, 1, 20);
    ChunkSize.Y = FMath::Clamp(ChunkSize.Y, 1, 20);
    ChunkSize.Z = FMath::Clamp(ChunkSize.Z, 1, 20);
    const FVector ModuleSize = BoundsAsset->ChunkSize;
    const FTransform BaseTransform(FRotator::ZeroRotator, BoundsActor->GetActorLocation());
    const FMatrix ActorTransform = BaseTransform.ToMatrixNoScale();

    if (ChunkSize != FIntVector(1, 1, 1)) {
        // Draw the chunk bounds
        for (int z = 0; z < ChunkSize.Z; z++) {
            for (int y = 0; y < ChunkSize.Y; y++) {
                for (int x = 0; x < ChunkSize.X; x++) {
                    FVector Min = FVector(x, y, z) * ModuleSize;
                    FVector Max = Min + ModuleSize;
                    FBox Bounds(Min, Max);
                    DrawWireBox(PDI, ActorTransform, Bounds, FLinearColor(0.2f, 0, 0), SDPG_World);
                }
            }
        }
    }
    
    // Draw the main combined bounds
    {
        const FVector TotalSize = FVector(ChunkSize.X, ChunkSize.Y, ChunkSize.Z) * ModuleSize;
        const FBox Bounds(FVector::ZeroVector, TotalSize);
        DrawWireBox(PDI, ActorTransform, Bounds, FLinearColor::Red, SDPG_Foreground);
    }
}

