//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/SnapMapEditor/Viewport/SnapMapEditorViewportClient.h"

#include "Editor.h"
#include "EngineUtils.h"
#include "PreviewScene.h"

namespace DungeonFlowEditorViewportConstants {
    static const float GridSize = 2048.0f;
    static const int32 CellSize = 16;
}

FSnapMapEditorViewportClient::FSnapMapEditorViewportClient(FPreviewScene& InPreviewScene,
                                                                   TWeakPtr<SEditorViewport> InEditorViewportWidget)
    : FEditorViewportClient(nullptr, &InPreviewScene, InEditorViewportWidget) {

    // Setup defaults for the common draw helper.
    DrawHelper.bDrawPivot = true;
    DrawHelper.bDrawWorldBox = false;
    DrawHelper.bDrawKillZ = false;
    DrawHelper.bDrawGrid = true;
    DrawHelper.GridColorAxis = FColor(160, 160, 160);
    DrawHelper.GridColorMajor = FColor(144, 144, 144);
    DrawHelper.GridColorMinor = FColor(128, 128, 128);
    DrawHelper.PerspectiveGridSize = DungeonFlowEditorViewportConstants::GridSize;
    DrawHelper.NumCells = DrawHelper.PerspectiveGridSize / (DungeonFlowEditorViewportConstants::CellSize * 2);

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

void FSnapMapEditorViewportClient::Tick(float DeltaSeconds) {
    FEditorViewportClient::Tick(DeltaSeconds);

    // Tick the preview scene world.
    if (!GIntraFrameDebuggingGameThread) {
        PreviewScene->GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
    }
}

void FSnapMapEditorViewportClient::ProcessClick(class FSceneView& View, class HHitProxy* HitProxy, FKey Key,
                                                    EInputEvent Event, uint32 HitX, uint32 HitY) {

    if (HitProxy) {
        if (HitProxy->IsA(HActor::StaticGetType())) {
            HActor* ActorProxy = static_cast<HActor*>(HitProxy);
            if (ActorProxy && ActorProxy->Actor && ActorProxy->PrimComponent != nullptr) {
                AActor* SelectedActor = ActorProxy->Actor;
                if (GEditor) {
                    GEditor->SelectActor(SelectedActor, true, true);
                }
            }

            Invalidate();
        }

        // Pass to component vis manager
        //GUnrealEd->ComponentVisManager.HandleProxyForComponentVis(HitProxy);
    }
}

