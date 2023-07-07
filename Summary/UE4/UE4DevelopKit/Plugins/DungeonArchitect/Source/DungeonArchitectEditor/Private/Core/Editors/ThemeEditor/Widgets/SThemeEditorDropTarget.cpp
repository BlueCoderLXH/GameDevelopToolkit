//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/ThemeEditor/Widgets/SThemeEditorDropTarget.h"


FReply SThemeEditorDropTarget::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) {
    PanelCoordDropPosition = MyGeometry.AbsoluteToLocal(DragDropEvent.GetScreenSpacePosition());
    return SAssetDropTarget::OnDrop(MyGeometry, DragDropEvent);
}

void SThemeEditorDropTarget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime,
                                  const float InDeltaTime) {
    SAssetDropTarget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

    // Enable mouse integration while dragging
    if (GetDragOverlayVisibility().IsVisible()) {
        SetVisibility(EVisibility::Visible);
    }
    else {
        SetVisibility(EVisibility::SelfHitTestInvisible);
    }
}

