//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Snap/SnapGridFlow/SnapGridFlowModuleBounds.h"

#include "Components/BillboardComponent.h"
#include "DrawDebugHelpers.h"

/////////////////////////// ASnapGridFlowModuleBoundsActor ///////////////////////////

ASnapGridFlowModuleBoundsActor::ASnapGridFlowModuleBoundsActor(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    BoundsComponent = ObjectInitializer.CreateDefaultSubobject<USnapGridFlowModuleBoundsComponent>(this, "Bounds");
    RootComponent = BoundsComponent;

#if WITH_EDITORONLY_DATA
    Billboard = ObjectInitializer.CreateDefaultSubobject<UBillboardComponent>(this, "Billboard");
    Billboard->SetupAttachment(RootComponent);
#endif
}

/////////////////////////// FSnapGridFlowModuleBoundsUtils ///////////////////////////
class FSnapGridFlowModuleBoundsUtils {
public:
    
};

/////////////////////////// USnapGridFlowModuleBoundsComponent ///////////////////////////
USnapGridFlowModuleBoundsComponent::USnapGridFlowModuleBoundsComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bHiddenInGame = true;
    AlwaysLoadOnClient = false;
    AlwaysLoadOnServer = false;
    bUseAsOccluder = false;
    bUseEditorCompositing = true;
    bCanEverAffectNavigation = false;
}

FBoxSphereBounds USnapGridFlowModuleBoundsComponent::CalcBounds(const FTransform& LocalToWorld) const {
    if (!ModuleBounds) {
        return USceneComponent::CalcBounds(LocalToWorld);
    }
    
    const AActor* BoundsActor = GetOwner();
    check(BoundsActor);

    const FVector ModuleSize = ModuleBounds->ChunkSize;
    const FTransform BaseTransform(FRotator::ZeroRotator, BoundsActor->GetActorLocation());
    const FMatrix ActorTransform = BaseTransform.ToMatrixNoScale();
    
    const FVector TotalSize = FVector(NumChunks.X, NumChunks.Y, NumChunks.Z) * ModuleSize;
    const FBox LocalBounds(FVector::ZeroVector, TotalSize);

    const FBox WorldBounds = LocalBounds.TransformBy(ActorTransform);
    return FBoxSphereBounds(WorldBounds);
}

class FSnapGridFlowModuleBoundsProxy : public FPrimitiveSceneProxy {
public:
    FSnapGridFlowModuleBoundsProxy(const USnapGridFlowModuleBoundsComponent* InBoundsComponent)
        : FPrimitiveSceneProxy(InBoundsComponent)
        , bVisible(InBoundsComponent->bRenderBounds)
        , NumChunks(InBoundsComponent->NumChunks)
    {
        const USnapGridFlowModuleBoundsAsset* BoundsAsset = InBoundsComponent->ModuleBounds.LoadSynchronous();
        if (BoundsAsset) {
            ChunkSize = BoundsAsset->ChunkSize;
            BoxColor = BoundsAsset->BoundsWireColor;
            DoorColor = BoundsAsset->DoorColor;
            DoorOffsetZ = BoundsAsset->DoorOffsetZ;
        }
        else {
            bVisible = false;
        }
    }

    virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap,
            FMeshElementCollector& Collector) const override {
        if (!bVisible) return;
        
        for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++) {
            if (VisibilityMap & (1 << ViewIndex)) {
                FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);

                // Draw the inter chunk frames
                {
                    FLinearColor FrameColor = BoxColor * 0.25f;
                    for (int x = 1; x < NumChunks.X; x++) {
                        FVector Min = FVector(x, 0, 0) * ChunkSize;
                        FVector Max = FVector(0, NumChunks.Y, NumChunks.Z) * ChunkSize;
                        FBox Box(Min, Max);
                        DrawWireBox(PDI, GetLocalToWorld(), Box, FrameColor, SDPG_World, 3);
                    }

                    for (int y = 1; y < NumChunks.Y; y++) {
                        FVector Min = FVector(0, y, 0) * ChunkSize;
                        FVector Max = FVector(NumChunks.X, 0, NumChunks.Z) * ChunkSize;
                        FBox Box(Min, Max);
                        DrawWireBox(PDI, GetLocalToWorld(), Box, FrameColor, SDPG_World, 3);
                    }

                    for (int z = 1; z < NumChunks.Z; z++) {
                        FVector Min = FVector(0, 0, z) * ChunkSize;
                        FVector Max = FVector(NumChunks.X, NumChunks.Y, 0) * ChunkSize;
                        FBox Box(Min, Max);
                        DrawWireBox(PDI, GetLocalToWorld(), Box, FrameColor, SDPG_World, 3);
                    }
                }
                
    
                // Draw the main combined bounds
                {
                    const FVector TotalSize = FVector(NumChunks.X, NumChunks.Y, NumChunks.Z) * ChunkSize;
                    const FBox ChunkBounds(FVector::ZeroVector, TotalSize);
                    DrawWireBox(PDI, GetLocalToWorld(), ChunkBounds, BoxColor, SDPG_Foreground, 2, 0, true);
                }

                // Draw the Doors
                {
                    for (int z = 0; z < NumChunks.Z; z++) {
                        for (int y = 0; y < NumChunks.Y; y++) {
                            const TArray<int> ox = {0, NumChunks.X};
                            for (int x : ox) {
                                FVector DoorPoint = FVector(x, y + 0.5, z) * ChunkSize + FVector(0, 0, DoorOffsetZ);
                                {
                                    FVector BoxExtent(0, 100, 0);
                                    FBox Box(DoorPoint - BoxExtent, DoorPoint + BoxExtent);
                                    DrawWireBox(PDI, GetLocalToWorld(), Box, FLinearColor::Blue, SDPG_Foreground, 1, 0, true);
                                }
                                {
                                    FVector BoxExtent(0, 0, 150);
                                    FVector BoxCenter = DoorPoint + BoxExtent;
                                    FBox Box(BoxCenter - BoxExtent, BoxCenter + BoxExtent);
                                    DrawWireBox(PDI, GetLocalToWorld(), Box, FLinearColor::Blue, SDPG_Foreground, 1, 0, true);
                                }
                            }
                        }
                    }

                    for (int z = 0; z < NumChunks.Z; z++) {
                        for (int x = 0; x < NumChunks.X; x++) {
                            const TArray<int> oy = {0, NumChunks.Y};
                            for (int y : oy) {
                                FVector DoorPoint = FVector(x + 0.5, y, z) * ChunkSize + FVector(0, 0, DoorOffsetZ);
                                {
                                    FVector BoxExtent(100, 0, 0);
                                    FBox Box(DoorPoint - BoxExtent, DoorPoint + BoxExtent);
                                    DrawWireBox(PDI, GetLocalToWorld(), Box, DoorColor, SDPG_Foreground, 1, 0, true);
                                }
                                {
                                    FVector BoxExtent(0, 0, 150);
                                    FVector BoxCenter = DoorPoint + BoxExtent;
                                    FBox Box(BoxCenter - BoxExtent, BoxCenter + BoxExtent);
                                    DrawWireBox(PDI, GetLocalToWorld(), Box, DoorColor, SDPG_Foreground, 1, 0, true);
                                }
                            }
                        }
                    }
                }

                // Draw the vertical doors
                {
                    const int32 NumSides = 16;
                    const float Radius = FMath::Min(ChunkSize.X, ChunkSize.Y) * 0.02f;
                    TArray<FVector> LocalCirclePoints;
                    for (int i = 0; i < NumSides; i++) {
                        FVector& Point = LocalCirclePoints.AddDefaulted_GetRef();
                        const float Angle = PI * 2.0f * i / NumSides;
                        FMath::SinCos(&Point.Y, &Point.X, Angle);
                        Point.Z = 0;
                        Point *= Radius;
                    }

                    for (int x = 0; x < NumChunks.X; x++) {
                        for (int y = 0; y < NumChunks.Y; y++) {
                            const TArray<int> oz = {0, NumChunks.Z};
                            for (int z : oz) {
                                FVector DoorPoint = FVector(x + 0.5, y + 0.5, z) * ChunkSize;
                                for (int i = 0; i < LocalCirclePoints.Num(); i++) {
                                    const FVector P0 = DoorPoint + LocalCirclePoints[i];
                                    const FVector P1 = DoorPoint + LocalCirclePoints[(i + 1) % LocalCirclePoints.Num()];

                                    const FVector Start = GetLocalToWorld().TransformPosition(P0);
                                    const FVector End = GetLocalToWorld().TransformPosition(P1);
                                    PDI->DrawLine(Start, End, DoorColor, SDPG_World, 0, 0, true);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
    {
        FPrimitiveViewRelevance Result;
        Result.bDrawRelevance = IsShown(View);
        Result.bShadowRelevance = IsShadowCast(View);
        Result.bDynamicRelevance = true;
        return Result;
    }

    virtual uint32 GetMemoryFootprint(void) const override { return sizeof(*this) + GetAllocatedSize(); }
    virtual SIZE_T GetTypeHash() const override {
        static size_t UniquePointer;
        return reinterpret_cast<size_t>(&UniquePointer);
    }

private:
    bool bVisible;
    FLinearColor BoxColor = FLinearColor::Red;
    FLinearColor DoorColor = FLinearColor::Blue;
    FIntVector NumChunks;
    FVector ChunkSize;
    float DoorOffsetZ = 0;
};

FPrimitiveSceneProxy* USnapGridFlowModuleBoundsComponent::CreateSceneProxy() {
    return new FSnapGridFlowModuleBoundsProxy(this);
}

bool USnapGridFlowModuleBoundsComponent::IsEditorOnly() const {
    return true;
}

