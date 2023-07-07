//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/SnapMap/EditorMode/SnapMapEdModeRenderer.h"

#include "Builders/SnapMap/EditorMode/SnapMapEdModeHandler.h"

FSnapMapEdModeRenderer::FSnapMapEdModeRenderer(USnapMapEdModeHandler* pModeHandler)
    : ModeHandler(pModeHandler) {
}

void FSnapMapEdModeRenderer::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI,
                                    TSharedPtr<IDungeonEdTool> ActiveTool) {

}

void FSnapMapEdModeRenderer::Destroy() {
}

