//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

class ULADataF;
class SGridFlowTilemap;
class FSketchImage;
class UTexture;

/** Viewport Client 2D for the preview viewport */
class DUNGEONARCHITECTEDITOR_API FGridFlowTilemapViewportClient
    : public FViewportClient {
public:
    FGridFlowTilemapViewportClient(
        TWeakPtr<SGridFlowTilemap> InLAThemeEditorViewport2D);
    ~FGridFlowTilemapViewportClient();

    /** FViewportClient interface */
    virtual void Draw(FViewport* Viewport, FCanvas* Canvas) override;
    virtual UWorld* GetWorld() const override { return nullptr; }

    void SetZoomPos(FVector2D InNewPos, float InNewZoom) {
        ZoomPos = InNewPos;
        ZoomAmount = InNewZoom;
    }

    void SetTexture(UTexture* InTexture) {
        this->Texture = InTexture;
    }

    void SetTranslucent(bool bInUseTranslucentBlend) {
        bUseTranslucentBlend = bInUseTranslucentBlend;
    }

private:
    TWeakPtr<SGridFlowTilemap> ThemeEditorViewportPtr;
    UTexture* Texture;

    float ZoomAmount;
    FVector2D ZoomPos;
    float ZoomSensitivity;
    bool bUseTranslucentBlend = true;
};

