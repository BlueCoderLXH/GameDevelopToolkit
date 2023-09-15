//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/Tilemap/Viewport/GridFlowTilemapViewport.h"

#include "Frameworks/Flow/Domains/Tilemap/Viewport/SGridFlowTilemap.h"

#include "Layout/Geometry.h"
#include "Widgets/SViewport.h"

FGridFlowTilemapViewport::FGridFlowTilemapViewport(
    FViewportClient* InViewportClient,
    TSharedPtr<SViewport> InViewportWidget,
    TWeakPtr<SGridFlowTilemap> InPreview2D)
    : FSceneViewport(InViewportClient, InViewportWidget)
      , Preview2D(InPreview2D)
      , ViewOffset(FVector2D::ZeroVector)
      , ZoomAmount(1)
      , ZoomSensitivity(0.1f)
      , bIsPanning(false)
      , SoftwareCursorPosition(FVector2D::ZeroVector)
      , bShowSoftwareCursor(false)
      , bRequestRedraw(true)
      , TotalMouseDelta(0) {
}

FReply FGridFlowTilemapViewport::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
    TotalMouseDelta = 0;

    if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton) {
        TSharedRef<SViewport> ViewportWidgetRef = GetViewportWidget().Pin().ToSharedRef();
        // RIGHT BUTTON is for dragging and Context Menu.
        FReply ReplyState = FReply::Handled();
        ReplyState.CaptureMouse(ViewportWidgetRef);
        ReplyState.UseHighPrecisionMouseMovement(ViewportWidgetRef);

        SoftwareCursorPosition =
            PanelCoordToGraphCoord(MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()));

        bRequestRedraw = true;

        return ReplyState;
    }
    return FReply::Unhandled();
}

FReply FGridFlowTilemapViewport::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
    // Did the user move the cursor sufficiently far, or is it in a dead zone?
    // In Dead zone     - implies actions like summoning context menus and general clicking.
    // Out of Dead Zone - implies dragging actions like moving nodes and marquee selection.
    const bool bCursorInDeadZone = TotalMouseDelta <= FSlateApplication::Get().GetDragTriggerDistance();


    if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton) {
        FReply ReplyState = FReply::Handled();

        if (HasMouseCapture()) {
            FSlateRect ThisPanelScreenSpaceRect = MyGeometry.GetLayoutBoundingRect();
            const FVector2D ScreenSpaceCursorPos = MyGeometry.LocalToAbsolute(
                GraphCoordToPanelCoord(SoftwareCursorPosition));

            FIntPoint BestPositionInViewport(
                FMath::RoundToInt(FMath::Clamp(ScreenSpaceCursorPos.X, ThisPanelScreenSpaceRect.Left,
                                               ThisPanelScreenSpaceRect.Right)),
                FMath::RoundToInt(FMath::Clamp(ScreenSpaceCursorPos.Y, ThisPanelScreenSpaceRect.Top,
                                               ThisPanelScreenSpaceRect.Bottom))
            );

            if (!bCursorInDeadZone) {
                //ReplyState.SetMousePos(BestPositionInViewport);
            }
        }

        TSharedPtr<SWidget> WidgetToFocus;
        if (bCursorInDeadZone) {
            //WidgetToFocus = OnSummonContextMenu(MyGeometry, MouseEvent);
        }

        bRequestRedraw = true;

        bIsPanning = false;
        return (WidgetToFocus.IsValid())
                   ? ReplyState.ReleaseMouseCapture().SetUserFocus(WidgetToFocus.ToSharedRef(),
                                                                   EFocusCause::SetDirectly)
                   : ReplyState.ReleaseMouseCapture();
    }

    return FReply::Unhandled();
}

FReply FGridFlowTilemapViewport::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
    const bool bIsRightMouseButtonDown = MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton);
    const bool bIsLeftMouseButtonDown = MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton);

    if (HasMouseCapture()) {
        // Track how much the mouse moved since the mouse down.
        const FVector2D CursorDelta = MouseEvent.GetCursorDelta();
        TotalMouseDelta += CursorDelta.Size();

        if (bIsRightMouseButtonDown) {
            FReply ReplyState = FReply::Handled();

            if (!CursorDelta.IsZero()) {
                bShowSoftwareCursor = true;
            }

            bIsPanning = true;
            ViewOffset -= CursorDelta / GetZoomAmount();

            bRequestRedraw = true;

            return ReplyState;
        }
        if (bIsLeftMouseButtonDown) {
            const bool bCursorInDeadZone = TotalMouseDelta <= FSlateApplication::Get().GetDragTriggerDistance();
        }

    }

    return FReply::Unhandled();
}

FReply FGridFlowTilemapViewport::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
    // We want to zoom into this point; i.e. keep it the same fraction offset into the panel
    const FVector2D WidgetSpaceCursorPos = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
    FVector2D PointToMaintainGraphSpace = PanelCoordToGraphCoord(WidgetSpaceCursorPos);


    const int32 ZoomLevelDelta = FMath::FloorToInt(MouseEvent.GetWheelDelta());

    const bool bAllowFullZoomRange = true;
    const float OldZoomLevel = ZoomAmount;

    ZoomAmount *= (1 + ZoomLevelDelta * ZoomSensitivity);

    // Re-center the screen so that it feels like zooming around the cursor.
    ViewOffset = PointToMaintainGraphSpace - WidgetSpaceCursorPos / GetZoomAmount();

    bRequestRedraw = true;

    return FReply::Handled();
}


FVector2D FGridFlowTilemapViewport::GraphCoordToPanelCoord(const FVector2D& GraphSpaceCoordinate) const {
    return (GraphSpaceCoordinate - GetViewOffset()) * GetZoomAmount();
}

FVector2D FGridFlowTilemapViewport::PanelCoordToGraphCoord(const FVector2D& PanelSpaceCoordinate) const {
    return PanelSpaceCoordinate / GetZoomAmount() + GetViewOffset();
}

