//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/GridFlow/GridFlowMiniMap.h"

#include "Builders/GridFlow/GridFlowConfig.h"
#include "Builders/GridFlow/GridFlowModel.h"
#include "Core/Dungeon.h"
#include "Core/DungeonLayoutData.h"
#include "Frameworks/Flow/Domains/Tilemap/GridFlowTilemap.h"
#include "Frameworks/Flow/Domains/Tilemap/GridFlowTilemapRenderer.h"

#include "CanvasItem.h"
#include "Engine/Canvas.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogGridFlowMiniMap, Log, All);

void AGridFlowMiniMap::BuildLayout(UDungeonModel* DungeonModel, UDungeonConfig* DungeonConfig) {
    UGridFlowModel* GridFlowModel = Cast<UGridFlowModel>(DungeonModel);

    if (!GridFlowModel) {
        UE_LOG(LogGridFlowMiniMap, Error, TEXT("Invalid Grid Flow Model"));
        return;
    }

    UGridFlowConfig* GridFlowConfig = Cast<UGridFlowConfig>(DungeonConfig);
    if (!GridFlowConfig) {
        UE_LOG(LogGridFlowMiniMap, Error, TEXT("Invalid Grid Flow Config"));
        return;
    }

    FDungeonLayoutData LayoutData;
    DungeonModel->GenerateLayoutData(LayoutData);
    {
        FQuat TargetRotation = FQuat::Identity;
        FVector TargetTranslation = GridFlowConfig->GridSize * FVector(GridFlowModel->BuildTileOffset.X, GridFlowModel->BuildTileOffset.Y, 0);
        if (ADungeon* Dungeon = Cast<ADungeon>(DungeonModel->GetOuter())) {
            FTransform BaseTransform = Dungeon->GetTransform();
            TargetTranslation = BaseTransform.TransformPosition(TargetTranslation);
            TargetRotation = BaseTransform.TransformRotation(TargetRotation);
        }

        UGridFlowTilemap* Tilemap = GridFlowModel->Tilemap;
        float TargetScaleF = 1.0f /
        (FMath::Max(Tilemap->GetWidth(), Tilemap->GetHeight()) * FMath::Max(
            GridFlowConfig->GridSize.X, GridFlowConfig->GridSize.Y));

        FVector TargetScale(TargetScaleF);
        TargetTranslation *= TargetScaleF;
        WorldToScreen = FTransform(TargetRotation, TargetTranslation, TargetScale);
    }

    UTextureRenderTarget2D* MaskTextureRTT = Cast<UTextureRenderTarget2D>(MaskTexture);
    if (MaskTextureRTT) {
        UKismetRenderingLibrary::ReleaseRenderTarget2D(MaskTextureRTT);
    }
    MaskTextureRTT = UKismetRenderingLibrary::CreateRenderTarget2D(GetWorld(), TextureSize, TextureSize, RTF_RGBA8,
                                                                   FLinearColor::Transparent, false);

    {
        FGridFlowTilemapRendererSettings Settings;
        Settings.bUseTextureTileSize = false;
        Settings.TileSize = 10;
        Settings.BackgroundColor = FLinearColor::Transparent;

        UTextureRenderTarget2D* RenderedRTT = FGridFlowTilemapRenderer::Create(GridFlowModel->Tilemap, Settings);
        RenderedRTT->SRGB = 0;
        RenderedRTT->Filter = TF_Nearest;

        FDrawToRenderTargetContext RenderContext;
        FVector2D CanvasSize;
        UCanvas* Canvas = nullptr;

        UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(GetWorld(), MaskTextureRTT, Canvas, CanvasSize,
                                                               RenderContext);
        float Aspect = RenderedRTT->GetSurfaceWidth() / RenderedRTT->GetSurfaceHeight();
        float Width = CanvasSize.X;
        int32 Height = CanvasSize.Y;
        if (Aspect > 1.0f) {
            Height /= Aspect;
        }
        else {
            Width *= Aspect;
        }

        FCanvasTileItem Item(FVector2D::ZeroVector, RenderedRTT->Resource, FVector2D(Width, Height),
                             FLinearColor::White);
        Canvas->DrawItem(Item);
        UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(GetWorld(), RenderContext);
        UKismetRenderingLibrary::ReleaseRenderTarget2D(RenderedRTT);

        MaskTexture = MaskTextureRTT;
        MaskTexture->SRGB = 0;
        MaskTexture->Filter = TF_Nearest;
    }

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

