//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/Tilemap/Viewport/GridFlowTilemapViewportClient.h"

#include "CanvasTypes.h"
#include "UnrealClient.h"

FGridFlowTilemapViewportClient::FGridFlowTilemapViewportClient(
    TWeakPtr<SGridFlowTilemap> InThemeEditorViewport)
    : ThemeEditorViewportPtr(InThemeEditorViewport)
      , Texture(nullptr)
      , ZoomAmount(1.0f)
      , ZoomPos(FVector2D::ZeroVector)
      , ZoomSensitivity(0.1f) {
    check(ThemeEditorViewportPtr.IsValid());

}

FGridFlowTilemapViewportClient::~FGridFlowTilemapViewportClient() {
}

void FGridFlowTilemapViewportClient::Draw(FViewport* Viewport, FCanvas* Canvas) {
    Canvas->Clear(FLinearColor(0.15f, 0.15f, 0.15f));

    if (!Texture) {
        // We don't have a texture to render
        return;
    }

    // Fully stream in the texture before drawing it.
    if (Texture) {
        // Fully stream in the texture before drawing it.
        Texture->SetForceMipLevelsToBeResident(30.0f);
        Texture->WaitForStreaming();

        FLinearColor TextureDrawColor = FLinearColor::White;

        {
            // Draw the tile sheet texture 
            const float XPos = -ZoomPos.X * ZoomAmount;
            const float YPos = -ZoomPos.Y * ZoomAmount;
            const float Width = Texture->GetSurfaceWidth() * ZoomAmount;
            const float Height = Texture->GetSurfaceHeight() * ZoomAmount;

            Canvas->DrawTile(XPos, YPos, Width, Height, 0.0f, 0.0f, 1.0f, 1.0f, TextureDrawColor, Texture->Resource,
                             bUseTranslucentBlend);
        }
    }
}

