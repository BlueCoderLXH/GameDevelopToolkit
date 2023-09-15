//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Editors/SnapMapEditor/Viewport/SSnapMapEditorViewport.h"

/** Viewport Client for the preview viewport */
class DUNGEONARCHITECTEDITOR_API FSnapMapEditorViewportClient : public FEditorViewportClient,
                                                                    public TSharedFromThis<
                                                                        FSnapMapEditorViewportClient> {
public:
    FSnapMapEditorViewportClient(FPreviewScene& InPreviewScene,
                                     TWeakPtr<class SEditorViewport> InEditorViewportWidget);

    // FEditorViewportClient interface
    virtual void Tick(float DeltaSeconds) override;
    virtual void ProcessClick(class FSceneView& View, class HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX,
                              uint32 HitY) override;
    // End of FEditorViewportClient interface

};

