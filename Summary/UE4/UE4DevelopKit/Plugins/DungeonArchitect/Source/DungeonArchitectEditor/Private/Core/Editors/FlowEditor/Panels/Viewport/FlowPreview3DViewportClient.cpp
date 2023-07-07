//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/FlowEditor/Panels/Viewport/FlowPreview3DViewportClient.h"


namespace GridFlowPreview3DConstants {
    static const float GridSize = 2048.0f;
    static const int32 CellSize = 16;
}

FFlowPreview3DViewportClient::FFlowPreview3DViewportClient(FPreviewScene& InPreviewScene,
                                                                   const TWeakPtr<SEditorViewport>& InEditorViewport)
    : FEditorViewportClient(nullptr, &InPreviewScene, InEditorViewport)
{

    // Setup defaults for the common draw helper.
    DrawHelper.bDrawPivot = false;
    DrawHelper.bDrawWorldBox = false;
    DrawHelper.bDrawKillZ = false;
    DrawHelper.bDrawGrid = true;
    DrawHelper.GridColorAxis = FColor(160, 160, 160);
    DrawHelper.GridColorMajor = FColor(144, 144, 144);
    DrawHelper.GridColorMinor = FColor(128, 128, 128);
    DrawHelper.PerspectiveGridSize = GridFlowPreview3DConstants::GridSize;
    DrawHelper.NumCells = DrawHelper.PerspectiveGridSize / (GridFlowPreview3DConstants::CellSize * 2);

    FEditorViewportClient::SetViewMode(VMI_Lit);

    EngineShowFlags.SetSnap(false);
    EngineShowFlags.CompositeEditorPrimitives = true;
    OverrideNearClipPlane(1.0f);
    bUsingOrbitCamera = true;


    FVector CamLocation(0, 16000, 0);

    // Set the initial camera position
    FRotator OrbitRotation(-45, 180, 0);
    SetCameraSetup(
        FVector::ZeroVector,
        OrbitRotation,
        FVector(0.0f, 100, 0.0f),
        FVector::ZeroVector,
        CamLocation,
        FRotator(0, 0, 0)
    );


}

void FFlowPreview3DViewportClient::Tick(float DeltaSeconds) {
    FEditorViewportClient::Tick(DeltaSeconds);

    // Tick the preview scene world.
    if (!GIntraFrameDebuggingGameThread) {
        PreviewScene->GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
    }
}

