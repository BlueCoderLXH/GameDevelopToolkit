//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/MiniMap/DungeonMiniMap.h"

#include "Core/DungeonLayoutData.h"
#include "Core/DungeonModel.h"
#include "Core/Utils/MathUtils.h"

#include "Engine/Canvas.h"
#include "Engine/TextureRenderTarget2D.h"
#include "EngineUtils.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "RenderUtils.h"

DEFINE_LOG_CATEGORY_STATIC(LogDungeonMiniMap, Log, All);

ADungeonMiniMap::ADungeonMiniMap() {
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>("SceneRoot");
    RootComponent = SceneRoot;

    if (!IsRunningCommandlet()) {
        //ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialTemplateRef(TEXT("/DungeonArchitect/Core/Materials/MiniMap/Material/M_MMTemplate_Inst"));
        //MaterialTemplate = MaterialTemplateRef.Object;
        MaterialTemplate = nullptr;
    }
}

UMaterialInterface* ADungeonMiniMap::CreateMaterialInstance() {
    if (!MaterialTemplate) return nullptr;
    return CreateMaterialInstanceFromTemplate(MaterialTemplate);
}

UMaterialInterface* ADungeonMiniMap::CreateMaterialInstanceFromTemplate(UMaterialInterface* InMaterialTemplate) {
    UMaterialInstanceDynamic* MaterialInstance = UMaterialInstanceDynamic::Create(InMaterialTemplate, this);
    UpdateMaterial(MaterialInstance);
    return MaterialInstance;

}

void ADungeonMiniMap::UpdateMaterial(UMaterialInterface* InMaterial) {
    UMaterialInstanceDynamic* MaterialInstance = Cast<UMaterialInstanceDynamic>(InMaterial);
    if (MaterialInstance) {
        MaterialInstance->SetTextureParameterValue("MaskTex", MaskTexture);
        MaterialInstance->SetTextureParameterValue("StaticOverlayTex", StaticOverlayTexture);
        MaterialInstance->SetTextureParameterValue("DynamicOverlayTex", DynamicOverlayTexture);
        MaterialInstance->SetTextureParameterValue("FogOfWarTex", FogOfWarTexture);
    }
}

namespace {
    enum ECopyRTTMaskChannel : uint8 {
        Red = 1,
        Green = 2,
        Blue = 4,
        Alpha = 8
    };

    void CopyRTTMask(UTextureRenderTarget2D* RenderTexture, TArray<FColor>& OutMask, uint8 Channel) {
        TArray<FColor> SurfaceData;
        FRenderTarget* RenderTarget = RenderTexture->GameThread_GetRenderTargetResource();
        RenderTarget->ReadPixels(SurfaceData);

        // Resize the mask array to fit the new data
        {
            int32 NumItemsToAdd = SurfaceData.Num() - OutMask.Num();
            if (NumItemsToAdd > 0) {
                OutMask.AddZeroed(NumItemsToAdd);
            }
        }

        int32 NumItems = OutMask.Num();
#define COPY_MASK(CHANNEL) for (int i = 0; i < NumItems; i++) { OutMask[i].CHANNEL = SurfaceData[i].R; }
        if (Channel & Red)
            COPY_MASK(R);
        if (Channel & Green)
            COPY_MASK(G);
        if (Channel & Blue)
            COPY_MASK(B);
        if (Channel & Alpha)
            COPY_MASK(A);
#undef COPY_MASK

    }
}

void ADungeonMiniMap::BuildLayout(UDungeonModel* DungeonModel, UDungeonConfig* DungeonConfig) {
    if (!DungeonModel) {
        UE_LOG(LogDungeonMiniMap, Error, TEXT("Dungeon model not specified"));
        return;
    }

    FDungeonLayoutData LayoutData;
    DungeonModel->GenerateLayoutData(LayoutData);
    WorldToScreen = LayoutData.WorldToScreen;

    BuildLayoutTexture(LayoutData);
    BuildStaticOverlayTexture(LayoutData);

    // Build the dynamic overlay texture
    if (DynamicOverlayTexture) {
        UKismetRenderingLibrary::ReleaseRenderTarget2D(DynamicOverlayTexture);
    }
    DynamicOverlayTexture = UKismetRenderingLibrary::CreateRenderTarget2D(
        GetWorld(), TextureSize, TextureSize, RTF_RGBA8);
    UpdateDynamicOverlayTexture();

    if (FogOfWarTexture) {
        UKismetRenderingLibrary::ReleaseRenderTarget2D(FogOfWarTexture);
    }
    int32 FogOfWarTextureSize = FMath::RoundToInt(TextureSize * FogOfWarTextureScale);
    FogOfWarTexture = UKismetRenderingLibrary::CreateRenderTarget2D(GetWorld(), FogOfWarTextureSize,
                                                                    FogOfWarTextureSize, RTF_RGBA8);
    UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), FogOfWarTexture,
                                                 bEnableFogOfWar ? FLinearColor::Black : FLinearColor::White);
}

void ADungeonMiniMap::BeginDestroy() {
    Super::BeginDestroy();

    if (DynamicOverlayTexture) {
        UKismetRenderingLibrary::ReleaseRenderTarget2D(DynamicOverlayTexture);
        DynamicOverlayTexture = nullptr;
    }
}

void ADungeonMiniMap::Tick(float DeltaSeconds) {
    Super::Tick(DeltaSeconds);

    UpdateDynamicOverlayTexture();
    UpdateFogOfWarTexture();
}

float ADungeonMiniMap::GetAttributeScaleMultiplier() const {
    const float ReferenceTextureSize = 512;
    return TextureSize / ReferenceTextureSize;
}

float ADungeonMiniMap::GetIconPixelSize(const FDungeonMiniMapOverlayIcon& OverlayData, const FVector2D& CanvasSize) {
    if (OverlayData.ScreenSizeType == EDungeonMiniMapIconCoordinateSystem::Pixels) {
        return OverlayData.ScreenSize * GetAttributeScaleMultiplier();
    }
    if (OverlayData.ScreenSizeType == EDungeonMiniMapIconCoordinateSystem::WorldCoordinates) {
        return OverlayData.ScreenSize * WorldToScreen.GetScale3D().X * CanvasSize.X;
    }
    return 0;
}

void ADungeonMiniMap::BuildLayoutTexture(const FDungeonLayoutData& LayoutData) {
    FDrawToRenderTargetContext RenderContext;
    FVector2D CanvasSize;
    UCanvas* Canvas = nullptr;

    const float AttributeScale = GetAttributeScaleMultiplier();

    UTextureRenderTarget2D* RenderTexture = UKismetRenderingLibrary::CreateRenderTarget2D(
        GetWorld(), TextureSize, TextureSize, RTF_RGBA8);
    if (!RenderTexture) {
        UE_LOG(LogDungeonMiniMap, Error, TEXT("Cannot create render texture for mini map"));
        return;
    }

    TArray<FColor> MaskData;

    // Draw the layout
    {
        UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), RenderTexture, FLinearColor::Black);
        UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(GetWorld(), RenderTexture, Canvas, CanvasSize,
                                                               RenderContext);

        if (Canvas) {
            for (const FDungeonLayoutDataBlockItem& LayoutItem : LayoutData.LayoutItems) {
                int32 NumTriangles = LayoutItem.FillTriangles.Num() / 3;
                if (NumTriangles > 0) {
                    for (int i = 0; i < NumTriangles; i++) {
                        int i0 = i * 3 + 0;
                        int i1 = i * 3 + 1;
                        int i2 = i * 3 + 2;
                        FVector2D P0 = LayoutItem.FillTriangles[i0] * CanvasSize;
                        FVector2D P1 = LayoutItem.FillTriangles[i1] * CanvasSize;
                        FVector2D P2 = LayoutItem.FillTriangles[i2] * CanvasSize;

                        FCanvasTriangleItem Triangle(P0, P1, P2, GWhiteTexture);
                        Canvas->DrawItem(Triangle);
                    }

                }
            }
        }
        UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(GetWorld(), RenderContext);

        // Copy to the mask data
        CopyRTTMask(RenderTexture, MaskData, Red);
    }

    // Draw the outline
    {
        float Thickness = OutlineThickness * AttributeScale;
        UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), RenderTexture, FLinearColor::Black);
        UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(GetWorld(), RenderTexture, Canvas, CanvasSize,
                                                               RenderContext);

        if (Canvas) {
            for (const FDungeonLayoutDataBlockItem& LayoutItem : LayoutData.LayoutItems) {
                int32 NumPoints = LayoutItem.Outline.Num();
                if (NumPoints > 0) {
                    for (int i = 0; i < NumPoints; i++) {
                        int i0 = i;
                        int i1 = (i + 1) % NumPoints;
                        FVector2D P0 = LayoutItem.Outline[i0] * CanvasSize;
                        FVector2D P1 = LayoutItem.Outline[i1] * CanvasSize;

                        Canvas->K2_DrawLine(P0, P1, Thickness);
                    }
                }
            }
        }

        UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(GetWorld(), RenderContext);

        // Copy to the mask data
        CopyRTTMask(RenderTexture, MaskData, Green);
    }


    // Draw the doors texture
    {
        float Thickness = DoorThickness * AttributeScale;
        UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), RenderTexture, FLinearColor::Black);
        UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(GetWorld(), RenderTexture, Canvas, CanvasSize,
                                                               RenderContext);
        if (Canvas) {
            for (const FDungeonLayoutDataDoorItem& Door : LayoutData.Doors) {
                int32 NumPoints = Door.Outline.Num();
                if (NumPoints > 0) {
                    for (int i = 0; i + 1 < NumPoints; i++) {
                        int i0 = i;
                        int i1 = i + 1;
                        FVector2D P0 = Door.Outline[i0] * CanvasSize;
                        FVector2D P1 = Door.Outline[i1] * CanvasSize;

                        Canvas->K2_DrawLine(P0, P1, Thickness);
                    }
                }
            }
        }
        UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(GetWorld(), RenderContext);

        // Copy to the mask data
        CopyRTTMask(RenderTexture, MaskData, Blue);
    }

    // We are done with the render texture and all the data has been copied over to the mask data
    UKismetRenderingLibrary::ReleaseRenderTarget2D(RenderTexture);
    RenderTexture = nullptr;

    // Blur the layout data (R channel) and save it in the alpha channel
    {
        TArray<float> BlurData0;
        BlurData0.AddUninitialized(MaskData.Num());
        for (int i = 0; i < MaskData.Num(); i++) {
            BlurData0[i] = MaskData[i].R / 255.0f;
        }

        TArray<float> BlurData1;
        BlurData1.AddUninitialized(MaskData.Num());

        TArray<float> BlurWeights;
        BlurWeights.AddZeroed(MaskData.Num());

        float* BlurData0Ptr = BlurData0.GetData();
        float* BlurData1Ptr = BlurData1.GetData();
        float* BlurWeightsPtr = BlurWeights.GetData();
        const int32 SizeX = TextureSize;
        const int32 SizeY = TextureSize;

        float ScaledBlurRadius = BlurRadius * AttributeScale;

        TArray<int32> BlurKernels = BlurUtils::boxesForGauss(ScaledBlurRadius, BlurIterations);
        float* SourceData = BlurData0Ptr;
        float* TargetData = BlurData1Ptr;
        for (int i = 0; i < BlurKernels.Num(); i++) {
            BlurUtils::boxBlur_4(SourceData, TargetData, BlurWeightsPtr, SizeX, SizeY, (BlurKernels[i] - 1) / 2);
            // Swap source / target
            float* Temp = SourceData;
            SourceData = TargetData;
            TargetData = Temp;
        }

        for (int i = 0; i < MaskData.Num(); i++) {
            MaskData[i].A = FMath::RoundToInt(SourceData[i] * 255);
        }
    }

    // Create the mask texture
    MaskTexture = UTexture2D::CreateTransient(TextureSize, TextureSize, PF_B8G8R8A8);
#if WITH_EDITORONLY_DATA
    MaskTexture->MipGenSettings = TMGS_NoMipmaps;
#endif
    MaskTexture->SRGB = 0;

    if (UTexture2D* MaskTex2D = Cast<UTexture2D>(MaskTexture)) {
        FByteBulkData* BulkData = &MaskTex2D->PlatformData->Mips[0].BulkData;
        void* TextureData = BulkData->Lock(LOCK_READ_WRITE);
        int32 NumBytes = MaskData.Num() * sizeof(FColor);
        FMemory::Memcpy(TextureData, MaskData.GetData(), NumBytes);
        BulkData->Unlock();
        MaskTex2D->UpdateResource();
    }
}

namespace {
    void DrawMiniMapCanvasIcon(UCanvas* Canvas, UTexture2D* Icon, const FVector2D& InLocation, float ScreenSize,
                               FLinearColor InColor, float Rotation = 0.0f, EBlendMode BlendMode = BLEND_Masked) {
        if (!Canvas || !Icon) return;
        float SizeX = ScreenSize;
        float IconAspect = Icon->GetSizeX() / Icon->GetSizeY();
        float SizeY = SizeX / IconAspect;
        FVector2D Size = FVector2D(SizeX, SizeY);
        FVector2D CanvasLocation = InLocation - Size * 0.5f;
        Canvas->K2_DrawTexture(Icon, CanvasLocation, Size,
                               FVector2D::ZeroVector, FVector2D::UnitVector, InColor, BlendMode, Rotation);
    }

}

void ADungeonMiniMap::BuildStaticOverlayTexture(const FDungeonLayoutData& LayoutData) {
    const float AttributeScale = GetAttributeScaleMultiplier();

    TMap<FName, FDungeonMiniMapOverlayIcon> OverlayMap;
    for (const FDungeonMiniMapOverlayIcon& OverlayItem : OverlayIcons) {
        if (OverlayItem.Icon && !OverlayMap.Contains(OverlayItem.Name)) {
            OverlayMap.Add(OverlayItem.Name, OverlayItem);
        }
    }
    // Grab the items that have icons defined for it
    TArray<FDungeonLayoutDataPointOfInterest> PointsOfInterest;
    for (const FDungeonLayoutDataPointOfInterest& PointOfInterest : LayoutData.PointsOfInterest) {
        if (OverlayMap.Contains(PointOfInterest.Id)) {
            PointsOfInterest.Add(PointOfInterest);
        }
    }

    if (StaticOverlayTexture) {
        UKismetRenderingLibrary::ReleaseRenderTarget2D(StaticOverlayTexture);
        StaticOverlayTexture = nullptr;
    }
    StaticOverlayTexture = UKismetRenderingLibrary::CreateRenderTarget2D(
        GetWorld(), TextureSize, TextureSize, RTF_RGBA8);
    UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), StaticOverlayTexture, FLinearColor::Transparent);

    FDrawToRenderTargetContext RenderContext;
    FVector2D CanvasSize;
    UCanvas* Canvas = nullptr;
    UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(GetWorld(), StaticOverlayTexture, Canvas, CanvasSize,
                                                           RenderContext);
    if (Canvas) {
        for (const FDungeonLayoutDataPointOfInterest& PointOfInterest : PointsOfInterest) {
            FDungeonMiniMapOverlayIcon* SearchResult = OverlayMap.Find(PointOfInterest.Id);
            if (SearchResult) {
                FDungeonMiniMapOverlayIcon& OverlayData = *SearchResult;
                float Rotation = OverlayData.Rotation;
                FVector2D Location = PointOfInterest.Location * CanvasSize;
                float ScreenSize = GetIconPixelSize(OverlayData, CanvasSize);
                DrawMiniMapCanvasIcon(Canvas, OverlayData.Icon, Location, ScreenSize, OverlayData.Tint, Rotation);
            }
        }
    }
    UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(GetWorld(), RenderContext);
}

void ADungeonMiniMap::UpdateDynamicOverlayTexture() {
    if (!DynamicOverlayTexture) {
        return;
    }

    UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), DynamicOverlayTexture, FLinearColor::Transparent);

    if (DynamicTracking.Num() == 0) {
        return;
    }

    const float AttributeScale = GetAttributeScaleMultiplier();
    TMap<FName, FDungeonMiniMapOverlayIcon> OverlayMap;
    for (const FDungeonMiniMapOverlayIcon& OverlayItem : OverlayIcons) {
        if (OverlayItem.Icon && !OverlayMap.Contains(OverlayItem.Name)) {
            OverlayMap.Add(OverlayItem.Name, OverlayItem);
        }
    }


    FDrawToRenderTargetContext RenderContext;
    FVector2D CanvasSize;
    UCanvas* Canvas = nullptr;
    UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(GetWorld(), DynamicOverlayTexture, Canvas, CanvasSize,
                                                           RenderContext);
    if (Canvas) {
        for (const FDungeonMiniMapOverlayTracking& Overlay : DynamicTracking) {
            if (Overlay.TrackedActor.IsValid()) {
                FVector WorldLocation = Overlay.TrackedActor->GetActorLocation();
                FVector ScreenLocation = WorldToScreen.TransformPosition(WorldLocation);
                FVector2D ScreenLocation2D(ScreenLocation.X, ScreenLocation.Y);

                FDungeonMiniMapOverlayIcon* SearchResult = OverlayMap.Find(Overlay.IconName);
                if (SearchResult) {
                    FDungeonMiniMapOverlayIcon& OverlayData = *SearchResult;
                    float Rotation = OverlayData.Rotation;
                    if (Overlay.bOrientToRotation) {
                        FVector Angles = Overlay.TrackedActor->GetActorRotation().Euler();
                        Rotation += Angles.Z;
                    }

                    FVector2D CanvasLocation = ScreenLocation2D * CanvasSize;
                    float ScreenSize = GetIconPixelSize(OverlayData, CanvasSize);
                    DrawMiniMapCanvasIcon(Canvas, OverlayData.Icon, CanvasLocation, ScreenSize, OverlayData.Tint,
                                          Rotation);
                }
            }
        }
    }
    UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(GetWorld(), RenderContext);
}

void ADungeonMiniMap::UpdateFogOfWarTexture() {
    if (!bEnableFogOfWar) {
        return;
    }

    TWeakObjectPtr<AActor> TrackedActor;
    for (const FDungeonMiniMapOverlayTracking& Overlay : DynamicTracking) {
        if (Overlay.Id == FogOfWarTrackingItem && Overlay.TrackedActor.IsValid()) {
            TrackedActor = Overlay.TrackedActor;
            break;
        }
    }

    if (TrackedActor.IsValid()) {
        const float AttributeScale = GetAttributeScaleMultiplier() * FogOfWarTextureScale;
        FDrawToRenderTargetContext RenderContext;
        FVector2D CanvasSize;
        UCanvas* Canvas = nullptr;
        UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(GetWorld(), FogOfWarTexture, Canvas, CanvasSize,
                                                               RenderContext);
        if (Canvas) {
            FVector WorldLocation = TrackedActor->GetActorLocation();
            FVector ScreenLocation = WorldToScreen.TransformPosition(WorldLocation);
            FVector2D ScreenLocation2D(ScreenLocation.X, ScreenLocation.Y);

            FVector2D CanvasLocation = ScreenLocation2D * CanvasSize;
            float ScreenSize = FogOfWarVisiblityDistance * WorldToScreen.GetScale3D().X * CanvasSize.X;
            DrawMiniMapCanvasIcon(Canvas, FogOfWarExploreTexture, CanvasLocation, ScreenSize, FLinearColor::White, 0,
                                  BLEND_Additive);
        }
        UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(GetWorld(), RenderContext);
    }
}

void UDungeonMiniMapTrackedObject::BeginPlay() {
    Super::BeginPlay();

    // Find the Minimap component
    for (TActorIterator<ADungeonMiniMap> It(GetWorld()); It; ++It) {
        ADungeonMiniMap* MiniMap = *It;
        if (MiniMap) {
            FDungeonMiniMapOverlayTracking TrackingInfo;
            TrackingInfo.TrackedActor = GetOwner();
            TrackingInfo.Id = Id;
            TrackingInfo.IconName = IconName;
            TrackingInfo.bOrientToRotation = bOrientToRotation;
            MiniMap->DynamicTracking.Add(TrackingInfo);
            break;
        }
    }
}

