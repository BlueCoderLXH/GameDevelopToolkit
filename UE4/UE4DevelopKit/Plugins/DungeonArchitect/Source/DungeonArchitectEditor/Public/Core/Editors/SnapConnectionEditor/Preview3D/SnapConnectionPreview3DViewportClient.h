//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Editors/SnapConnectionEditor/Preview3D/SSnapConnectionPreview3DViewport.h"

#include "PreviewScene.h"

/** Viewport Client for the preview viewport */
class DUNGEONARCHITECTEDITOR_API FSnapConnectionPreview3DViewportClient : public FEditorViewportClient,
                                                                             public TSharedFromThis<
                                                                                 FSnapConnectionPreview3DViewportClient
                                                                             > {
public:
    FSnapConnectionPreview3DViewportClient(FPreviewScene& InPreviewScene);

    // FEditorViewportClient interface
    virtual void Tick(float DeltaSeconds) override;
    // End of FEditorViewportClient interface
};

