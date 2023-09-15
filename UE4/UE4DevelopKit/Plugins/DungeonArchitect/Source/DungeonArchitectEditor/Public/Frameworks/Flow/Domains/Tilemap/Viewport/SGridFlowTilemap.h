//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class FGridFlowTilemapViewportClient;
class FGridFlowTilemapViewport;
class SViewport;
class UTexture2D;

class UGridFlowTilemap;

class SGridFlowTilemap : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SGridFlowTilemap) {}
    SLATE_END_ARGS()

public:

    /** Constructs the widget */
    void Construct(const FArguments& InArgs);

public:
    // SWidget overrides
    virtual void Tick(const FGeometry& AllottedGeometry, double InCurrentTime, float InDeltaTime) override;
    // End of SWidget interface

    // Level viewport client.
    TSharedPtr<FGridFlowTilemapViewportClient> GetViewportClient() const { return ViewportClient; }

    // Slate viewport for rendering and IO.
    TSharedPtr<FGridFlowTilemapViewport> GetViewport() const { return Viewport; }

    // Slate viewport for rendering and IO.
    TSharedPtr<SViewport> GetViewportWidget() const { return ViewportWidget; }

    //void SetPreviewTexture(UTexture2D* Texture);
    void GeneratePreviewTexture(UGridFlowTilemap* InTilemap);

private:
    // Level viewport client.
    TSharedPtr<FGridFlowTilemapViewportClient> ViewportClient;

    // Slate viewport for rendering and IO.
    TSharedPtr<FGridFlowTilemapViewport> Viewport;

    // Slate viewport for rendering and IO.
    TSharedPtr<SViewport> ViewportWidget;

    SOverlay::FOverlaySlot* TexturePanelSlot = nullptr;
};

