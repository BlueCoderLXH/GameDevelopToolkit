//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/SnapConnectionEditor/Preview3D/SnapConnectionPreview3DViewportClient.h"


namespace SnapConnectionPreview3DConstants {
    static const float GridSize = 2048.0f;
    static const int32 CellSize = 16;
}

FSnapConnectionPreview3DViewportClient::FSnapConnectionPreview3DViewportClient(FPreviewScene& InPreviewScene)
    : FEditorViewportClient(nullptr, &InPreviewScene)
{
    // Setup defaults for the common draw helper.
    DrawHelper.bDrawPivot = false;
    DrawHelper.bDrawWorldBox = false;
    DrawHelper.bDrawKillZ = false;
    DrawHelper.bDrawGrid = true;
    DrawHelper.GridColorAxis = FColor(160, 160, 160);
    DrawHelper.GridColorMajor = FColor(144, 144, 144);
    DrawHelper.GridColorMinor = FColor(128, 128, 128);
    DrawHelper.PerspectiveGridSize = SnapConnectionPreview3DConstants::GridSize;
    DrawHelper.NumCells = DrawHelper.PerspectiveGridSize / (SnapConnectionPreview3DConstants::CellSize * 2);

    SetViewMode(VMI_Lit);

    //EngineShowFlags.DisableAdvancedFeatures();
    EngineShowFlags.SetSnap(false);
    EngineShowFlags.CompositeEditorPrimitives = true;
    OverrideNearClipPlane(1.0f);
    bUsingOrbitCamera = true;

    // Set the initial camera position
    FRotator OrbitRotation(-40, 10, 0);
    SetCameraSetup(
        FVector::ZeroVector,
        OrbitRotation,
        FVector(0.0f, 100, 0.0f),
        FVector::ZeroVector,
        FVector(300, 400, 500),
        FRotator(-10, 0, 0)
    );
    SetViewLocation(FVector(500, 300, 500));
    //SetLookAtLocation(FVector(0, 0, 0));
}

void FSnapConnectionPreview3DViewportClient::Tick(float DeltaSeconds) {
    FEditorViewportClient::Tick(DeltaSeconds);

    // Tick the preview scene world.
    if (!GIntraFrameDebuggingGameThread) {
        PreviewScene->GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
    }
}

