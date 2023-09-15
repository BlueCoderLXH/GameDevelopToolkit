//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "PreviewScene.h"

class UFlowAssetBase;

class DUNGEONARCHITECTEDITOR_API FFlowPreview3DViewportClient : public FEditorViewportClient,
                                                                    public TSharedFromThis<
                                                                        FFlowPreview3DViewportClient> {
public:
    FFlowPreview3DViewportClient(FPreviewScene& InPreviewScene, const TWeakPtr<SEditorViewport>& InEditorViewport);

    // FEditorViewportClient interface
    virtual void Tick(float DeltaSeconds) override;
    // End of FEditorViewportClient interface
};

