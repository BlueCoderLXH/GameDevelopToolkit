//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/Domains/Tilemap/Viewport/SGridFlowTilemap.h"

#include "Slate/SceneViewport.h"

class SGridFlowTilemap;

/** Viewport Client 2D for the preview viewport */
class DUNGEONARCHITECTEDITOR_API FGridFlowTilemapViewport
    : public FSceneViewport {
public:
    FGridFlowTilemapViewport(FViewportClient* InViewportClient, TSharedPtr<SViewport> InViewportWidget,
                             TWeakPtr<SGridFlowTilemap> InPreview2D);

    virtual FReply OnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& MouseEvent) override;
    virtual FReply OnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& MouseEvent) override;
    virtual FReply OnMouseMove(const FGeometry& InGeometry, const FPointerEvent& MouseEvent) override;
    virtual FReply OnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& MouseEvent) override;

    float GetZoomAmount() const { return ZoomAmount; }
    FVector2D GetViewOffset() const { return ViewOffset; }
    FVector2D GraphCoordToPanelCoord(const FVector2D& GraphSpaceCoordinate) const;
    FVector2D PanelCoordToGraphCoord(const FVector2D& PanelSpaceCoordinate) const;

private:
    TWeakPtr<SGridFlowTilemap> Preview2D;

    /** The position within the graph at which the user is looking */
    FVector2D ViewOffset;

    /** How zoomed in/out we are. e.g. 0.25f results in quarter-sized nodes. */
    float ZoomAmount;

    float ZoomSensitivity = 0.1f;

    /** Are we panning the view at the moment? */
    bool bIsPanning;

    /**	The current position of the software cursor */
    FVector2D SoftwareCursorPosition;

    /**	Whether the software cursor should be drawn */
    bool bShowSoftwareCursor;

    bool bRequestRedraw;

    /** The total distance that the mouse has been dragged while down */
    float TotalMouseDelta;
};

