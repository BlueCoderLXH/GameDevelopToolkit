//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "ComponentVisualizer.h"

class FPlaceableMarkerVisualizer : public FComponentVisualizer  {
public:
    /** Draw HUD on viewport for the supplied component */
    virtual void DrawVisualizationHUD(const UActorComponent* Component, const FViewport* Viewport, const FSceneView* View, FCanvas* Canvas) override;
};

